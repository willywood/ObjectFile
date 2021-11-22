//
//  Simple Command line multi-thread test program for ObjectFile.
//
//  Settting an object threshold causes objects to be purged,
//  so it really exercises the object cache well under multi
//  threaded conditions.
//
//  First write a file. Then read it.
//
//  This uses Windows threads, so is not portable.
//
//

#include "odefs.h"
#include <iostream>
#include <fstream>
#include <stdio.h>
#ifndef WIN32
// Borland only
#include <classlib\thread.h>
#define THREAD_FUNC int Run()
#endif
#ifdef WIN32
#include "tthread.h"
#endif
#include "ofile.h"
#include "oiter.h"
#include "ox.h"
#include "mmyclass.h"

using namespace std;


static long threshold;
static long nObjects,avSize;

void my_new_handler()
// new_handler for OPersist objects. Activated when the object
// threshold is reached.
// Exceptions: OFileThresholdErr is thrown if after purging there is no
// space left.
{
	// Purge 10% of the object threshold. This is
	//  good when randomly accessing a large file.
	long objectsPurged = 0;
	OFile *f = OFile::getFirstOFile();

	// Try to recover memory just by purging.
	while(f)
	{
		objectsPurged += f->purge(cOPersist,true,20*OFile::getObjectThreshold()/100);
		if(objectsPurged >= 20*OFile::getObjectThreshold()/100)
			break;
		f = f->getNextOFile();
	}

	// If we did not succede then throw an exception.
	if(OFile::getObjectCacheCount() >= OFile::getObjectThreshold())
		throw OFileThresholdErr("Object threshold exceeded");

	return;
}


class MyReadThread : public TThread
{
public:
	MyReadThread(OFile &file):TThread("ofile_read", TRUE), f(file),count(0){}
	long count;
	static MyReadThread **threads;
	static int nThreads;
private:
	OFile &f;
	int Run();
};

MyReadThread **MyReadThread::threads = 0;
int MyReadThread::nThreads = 0;

int MyReadThread::Run()
{
static OFMutex sMutex;

	try
	{
		long nObjects = f.objectCount();
		// Iterate sequentially over all objects.
		for(count = 0;count < nObjects; count++)
		{
			OPersist *p = f.getObject(rand() % nObjects);
			if(p)
			{
				p->oSetPurgeable();
	            {
					// Make sure io does not overlap
		        	OFGuard l(sMutex);
					cout << "Objects read ";
					for(int j = 0 ; j < nThreads; j++)
						cout << " " << threads[j]->count;
					cout << '\r';
            	}
			}
		}

	}catch(OFileErr x){
		cout << '\n' << x.why() << '\n';
	}

      return 0;
}

class MyWriteThread : public TThread
{
public:
	MyWriteThread(OFile &file):TThread("ofile_write",TRUE),f(file), count(0) {}
	long count;
	static MyWriteThread **threads;
	static int nThreads;
private:
	OFile &f;
	int Run();
};

MyWriteThread **MyWriteThread::threads = 0;
int MyWriteThread::nThreads = 0;

int MyWriteThread::Run()
{
static OFMutex sMutex;

	// Attach
	for(count = 0; count < nObjects ; count++)
	{
		MyClass10 *p = new MyClass10(avSize);
		f.attach(p);
		p->oSetPurgeable();
        {
			// Make sure io does not overlap
        	OFGuard l(sMutex);
			cout << "Objects written ";
			for(int j = 0 ; j < nThreads; j++)
				cout << " " << threads[j]->count;
			cout << '\r';
       	}
	}

	// Commit objects that were not automatically committed.
	f.commit();
    return 0;
}


int main()
{

	cout << "ObjectFile Benchmark.\n";

	int menu = 0;
	while(menu != 5)
	{

	cout << "\n1. Set object threshold\n2. Write\n3. Sequential read\n4"
			". Random read\n5. Exit\n?";
	cin >> menu;

	switch(menu)
	{
case 1:
	threshold = 1000;
	cout << "Object threshold (1000)?",cin >> threshold;
	OFile::setObjectThreshold(threshold);
	break;

case 2:
	cout << "Number of objects ?",cin >> nObjects;
	cout << "Average object size in bytes ?",cin >> avSize;

	try
	{
		int i;
		// Create a file
		OFile f("ofile.tst",OFILE_CREATE);

		// Set the default handler that is good for sequentially
		// writing a large file.
		OFile::set_new_handler(OFile::new_handler);

		// Automaticatally save if needed
		f.setAutoCommit();

		cout << "Number of threads ?",cin >> MyWriteThread::nThreads;

		MyWriteThread::threads = new MyWriteThread*[MyWriteThread::nThreads];

		cout << "Thread number ";
		// Create threads.
		for(i = 0; i < MyWriteThread::nThreads; i++)
		{
			MyWriteThread::threads[i] = new MyWriteThread(f);
			cout << " " << (i + 1);
		}
		cout << '\n';

		// Set threads going.
		for(i = 0; i < MyWriteThread::nThreads; i++)
		{
			MyWriteThread::threads[i]->Start();
		}
		MyWriteThread::threads[0]->StartAllThreads();

		// Delete the threads
		BOOL running;
		do
		{
			running = FALSE;
			for (i = 0; i < MyWriteThread::nThreads; i++)
			{
				if (MyWriteThread::threads[i] != 0 && MyWriteThread::threads[i]->isRunning())
				{
					running = TRUE;
				}
			}
			Sleep(1000);
		} while (running);

		for (i = 0; i < MyWriteThread::nThreads; i++)
		{
			delete MyWriteThread::threads[i];
			MyWriteThread::threads[i] = 0;
		}


	}catch(OFileErr x){
		cout << '\n' << x.why() << '\n';
	}

	cout << '\n';
	break;

case 3:
	try
	{
		// Open the file
		OFile f("ofile.tst",OFILE_OPEN_READ_ONLY);

		OIterator it(&f);

		OPersist *p;
		long count = 1;

		// Iterate sequentially over all objects.
		while(p = it++)
		{
			p->oSetPurgeable();
			cout << count++ << " objects read.\r";
		}


	}catch(OFileErr x){
		cout << '\n' << x.why() << '\n';
	}


	cout << '\n';
	break;

case 4:
	try
	{
		int i;
		// Open the file
		OFile f("ofile.tst",OFILE_OPEN_READ_ONLY);

		// Set a handler that is good for randomly accessing a large file.
		OFile::set_new_handler(my_new_handler);

		cout << "Number of threads ?",cin >> MyReadThread::nThreads;

		MyReadThread::threads = new MyReadThread*[MyReadThread::nThreads];

		cout << "Thread number ";
		// Create threads.
		for(i = 0; i < MyReadThread::nThreads; i++)
		{
			MyReadThread::threads[i] = new MyReadThread(f);
			cout << " " << (i + 1);
		}
		cout << '\n';

		// Set threads going.
		for(i = 0; i < MyReadThread::nThreads; i++)
		{
			MyReadThread::threads[i]->Start();
		}
		MyWriteThread::threads[0]->StartAllThreads();

		// Delete the threads
		BOOL running;
		do
		{
			running = FALSE;
			for (i = 0; i < MyReadThread::nThreads; i++)
			{
				if (MyReadThread::threads[i] != 0 && MyReadThread::threads[i]->isRunning())
				{
					running = TRUE;
				}
			}
			Sleep(1000);
		} while (running);

		for (i = 0; i < MyReadThread::nThreads; i++)
		{
			delete MyReadThread::threads[i];
			MyReadThread::threads[i] = 0;
		}

	}catch(OFileErr x){
		cout << '\n' << x.why() << '\n';
	}
	default: ;
	} // switch


	} // while
}



