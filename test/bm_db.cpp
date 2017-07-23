//
//  Simple Command line benchmark program for ObjectFile.
//
// Note that this benchmark is far too simple to be representative
// of any real application. The results therefore have no real
// meaning.
//
//

#include "odefs.h"
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include "ofile.h"
#include "oiter.h"
#include "ox.h"
#include "mmyclass.h"

using namespace std;

static long threshold;


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
		objectsPurged += f->purge(cOPersist,true,10*threshold/100);
		if(objectsPurged >= 10*threshold/100)
			break;
		f = f->getNextOFile();
	}

	// If we did not succede then throw an exception.
	if(OFile::getObjectCacheCount() >= OFile::getObjectThreshold())
		throw OFileThresholdErr("Object threshold exceeded");

	return;
}


int main()
{
long nObjects,avSize;


	cout << "ObjectFile Benchmark.\n\n";

	int menu;
	while(menu != 5)
	{

	cout << "1. Set object threshold\n2. Write\n3. Sequential read\n4"
			". Random read\n5. Exit\n?";
	cin >> menu;

	switch(menu)
	{
case 1:
	cout << "Object threshold ?",cin >> threshold;
	OFile::setObjectThreshold(threshold);
	break;

case 2:
	cout << "Number of objects ?",cin >> nObjects;
	cout << "Average object size in bytes ?",cin >> avSize;

	try
	{
		// Create a file
		OFile f("ofile.tst",OFILE_CREATE);

		// Set the default handler that is good for sequentially
		// writing a large file.
		OFile::set_new_handler(OFile::new_handler);

		// Automaticatally save if needed
		f.setAutoCommit();

		// Attach
		for(long i = 1; i <= nObjects ; i++)
		{
			MyClass10 *p = new MyClass10(avSize);
			f.attach(p);
			p->oSetPurgeable();
			cout << i << " objects written.\r";
		}

		// Commit objects that were not automatically committed.
		f.commit();

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
		while((p = it++))
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
		// Open the file
		OFile f("ofile.tst",OFILE_OPEN_READ_ONLY);

		// Set a handler that is good for randomly accessing a large file.
		OFile::set_new_handler(my_new_handler);

		nObjects = f.objectCount();
		// Iterate sequentially over all objects.
		for(long i = 1;i < nObjects; i++)
		{
			OPersist *p = f.getObject(rand() % nObjects);
			if(p)
			{
				p->oSetPurgeable();
				cout << i << " objects read.\r";
			}
		}


	}catch(OFileErr x){
		cout << '\n' << x.why() << '\n';
	}
	default: ;
	} // switch


	} // while
	return 0;
}


