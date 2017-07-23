#ifndef OFTHREAD_H
#define OFTHREAD_H
//======================================================================
// 	This software may only be compiled in conjunction with a licence
// agreement with ObjectFile Limited.
// Copyright(c) 1996,97 ObjectFile Ltd. 
//======================================================================

// Purpose: System dependant thread implementation.
// This file is included within the definition of OFile. All definitions
// made here will therefore be local to the OFile class.

//////////////////////////////////////////////////////////////////////
// System dependant stuff
#ifdef OF_MULTI_THREAD
#ifdef  RWSTD_MULTI_THREAD
// Multi-threaded. Each platform requires an OFMutex type and an OFGuard
// type. OFGuard should lock the mutex in its constructor and release
// it in its destructor.
//
// RogueWave STL including Borland 5.0
	typedef RWSTDMutex OFMutex;
	typedef RWSTDGuard OFGuard;

#elif defined(__WIN32__) || defined(_WIN32)

// WIN32 implementation of threads
#include <windows.h>

// Define this if you are using multiple processes.
// Otherwise critical sections are much faster.
#ifdef OF_THREADS_MUTEX

class OFMutex
{
  private:

    HANDLE mutex;

    void init ()
	{
	    mutex = CreateMutex(NULL, FALSE, NULL);
	}
    //
    // Disallow copying and assignment.
    //
    OFMutex (const OFMutex&){}
    OFMutex& operator= (const OFMutex&);

public:

  OFMutex ()
  // Construct the mutex.
  {
		init();
  }
  ~OFMutex ()
  // Destroy the mutex.
  {
    CloseHandle(mutex);
  }

  void acquire ()
  // Acquire the mutex.
  {
    WaitForSingleObject(mutex,INFINITE);
  }

  void release ()
  // Release the mutex.
  {
    ReleaseMutex(mutex);
  }
};

class OFGuard
{
public:

    OFGuard  (OFMutex& m): ofmutex(m)
    // Acquire the mutex.
	{
    	ofmutex.acquire();
	}


    ~OFGuard ()
    // Release the mutex.
	{
		 ofmutex.release(); 
	}

private:
    OFMutex& ofmutex;
};

#else

class OFMutex
{
  private:

    CRITICAL_SECTION mutex;

    void init ()
	{
	    InitializeCriticalSection(&mutex);
	}
    //
    // Disallow copying and assignment.
    //
    OFMutex (const OFMutex&){}
    OFMutex& operator= (const OFMutex&);

public:

  OFMutex ()
  // Construct the mutex.
  {
		init();
  }
  ~OFMutex ()
  // Destroy the mutex.
  {
    DeleteCriticalSection(&mutex);
  }

  void acquire ()
  // Acquire the mutex.
  {
    EnterCriticalSection(&mutex);
  }

  void release ()
  // Release the mutex.
  {
    LeaveCriticalSection(&mutex);
  }
};

class OFGuard
{
public:

    OFGuard  (OFMutex& m): ofmutex(m)
    // Acquire the mutex.
	{
    	ofmutex.acquire();
	}


    ~OFGuard ()
    // Release the mutex.
	{
		 ofmutex.release(); 
	}

private:
    OFMutex& ofmutex;
};

#endif // OF_THREADS_MUTEX
// End of WIN32
// #elif <other system>

#endif  
// End of OF_MULTI_THREAD
// End of system dependant stuff
//////////////////////////////////////////////////////////////////////
#else

// default non multi-threaded. This should compile to nothing.
typedef int OFMutex;
class OFGuard{
public:
	OFGuard(int){}  // does nothing
//	~OFGuard(){}    // does nothing (declaring causes Borland at least to generate code)
};

#endif


#endif // OFTHREAD_H
