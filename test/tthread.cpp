#include "tthread.h"
#include "odefs.h"
#include "ox.h"


HANDLE TThread::m_hStartAllThreads = INVALID_HANDLE_VALUE;
TThread* TThread::m_allocatedStartAllThreads = NULL;

struct ThreadParam
{
	TThread* pThread;
};

//----------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------
TThread::TThread(const CHAR* threadName, BOOL syncStart) :
	THREAD_NAME(threadName),
	SYNC_START(syncStart),
	m_hThreadStarted(INVALID_HANDLE_VALUE),
	m_hThreadExited(INVALID_HANDLE_VALUE),
	m_hThread(INVALID_HANDLE_VALUE),
	m_threadId(0),
	m_running(TRUE)
{
	if (m_hStartAllThreads == INVALID_HANDLE_VALUE)
	{
		m_hStartAllThreads = CreateEvent(NULL, TRUE, FALSE, TEXT("StartAllThreadsEvent"));
		m_allocatedStartAllThreads = this;
	}
}

//----------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------
TThread::~TThread()
{
	if (m_allocatedStartAllThreads == this && m_hStartAllThreads != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hStartAllThreads);
		m_hStartAllThreads = INVALID_HANDLE_VALUE;
	}

	/*if (m_hThread != INVALID_HANDLE_VALUE)
		WaitForExit();*/
}

//----------------------------------------------------------------------------
// PostThreadMessage
//----------------------------------------------------------------------------
void TThread::PostThreadMessage(UINT msg, void* data)
{
	BOOL success = ::PostThreadMessage(GetThreadId(), msg, (WPARAM)data, 0);
	oFAssert(success != 0);
}

//----------------------------------------------------------------------------
// CreateThread
//----------------------------------------------------------------------------
BOOL TThread::Start()
{
	// Is the thread already created?
	if (!IsCreated())
	{
		m_hThreadStarted = CreateEvent(NULL, TRUE, FALSE, TEXT("ThreadCreatedEvent"));

		// Create the worker thread
		ThreadParam threadParam;
		threadParam.pThread = this;
		m_hThread = ::CreateThread(NULL, 0, (unsigned long(__stdcall*)(void*))RunProcess, (void*)(&threadParam), 0, &m_threadId);
		oFAssert(m_hThread != NULL);

		// Block the thread until thread is fully initialized including message queue
		DWORD err = WaitForSingleObject(m_hThreadStarted, MAX_WAIT_TIME);
		oFAssert(err == WAIT_OBJECT_0);

		CloseHandle(m_hThreadStarted);
		m_hThreadStarted = INVALID_HANDLE_VALUE;

		return m_hThread ? TRUE : FALSE;
	}
	return FALSE;
}

//----------------------------------------------------------------------------
// ExitThread
//----------------------------------------------------------------------------
void TThread::WaitForExit()
{
	if (m_hThread != INVALID_HANDLE_VALUE)
	{
		m_hThreadExited = CreateEvent(NULL, TRUE, FALSE, TEXT("ThreadExitedEvent"));
		PostThreadMessage(WM_EXIT_THREAD);

		// Wait here for the thread to exit
		if (::WaitForSingleObject(m_hThreadExited, MAX_WAIT_TIME) == WAIT_TIMEOUT)
			::TerminateThread(m_hThread, 1);

		::CloseHandle(m_hThread);
		m_hThread = INVALID_HANDLE_VALUE;

		::CloseHandle(m_hThreadExited);
		m_hThreadExited = INVALID_HANDLE_VALUE;
	}
}

//----------------------------------------------------------------------------
// RunProcess
//----------------------------------------------------------------------------
int TThread::RunProcess(void* threadParam)
{
	// Extract the TThread pointer from ThreadParam.
	TThread* thread;
	thread = (TThread*)(static_cast<ThreadParam*>(threadParam))->pThread;

	// Force the system to create the message queue before setting the event below.
	// This prevents a situation where another thread calls PostThreadMessage to post
	// a message before this thread message queue is created.
	MSG msg;
	PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);

	// Thread now fully initialized. Set the thread started event.
	BOOL err = SetEvent(thread->m_hThreadStarted);
	oFAssert(err != 0);

	// Using a synchronized start?
	if (thread->SYNC_START == TRUE)
	{
		// Block the thread here until all other threads are ready. A call to 
		// StartAllThreads() releases all the threads at the same time.
		DWORD err = WaitForSingleObject(m_hStartAllThreads, MAX_WAIT_TIME);
		oFAssert(err == WAIT_OBJECT_0);
	}
	thread->m_running = TRUE;

	// Call the derived class Process() function to implement the thread loop.
	int retVal = thread->Run();

	/*// Thread loop exited. Set exit event. 
	err = SetEvent(thread->m_hThreadExited);
	if (!err)
	{
		throw OFileIOErr("SetEvent Failed.");
	}*/

	thread->m_running = FALSE;

	return retVal;
}

//----------------------------------------------------------------------------
// StartAllThreads
//----------------------------------------------------------------------------
void TThread::StartAllThreads()
{
	BOOL err = SetEvent(m_hStartAllThreads);
	oFAssert(err != 0);
}
