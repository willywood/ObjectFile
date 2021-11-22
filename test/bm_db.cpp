//
//  Simple Command line benchmark program for ObjectFile.
//
// Note that this benchmark is very simple and will only give an indication
// of the maximum potential speed.
//
// Benchmarking on a i7 laptop with SATA III ssd shows that we are faster
// than the CrystalDiskMark Random disk benchmark!
//
//

#include "odefs.h"
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "ofile.h"
#include "oiter.h"
#include "ox.h"
#include "mmyclass.h"
#include "oblobp.h"

using namespace std;

static int sThreshold;

const OClassId_t cMyBlobP = 90;


class MyBlobP : public OPersist
{
	typedef OPersist inherited;
public:
	// Read constructor
	MyBlobP(OIStream* in): OPersist(in), _blob(in) 
	{
		// Force blob data into memory.
		(void)_blob.getBlob();
	}
	MyBlobP(size_t size): _blob(size)
	{
		// get pointer tp blob data.
		char *blobData = _blob.getBlob();
		// Set it all to x
		memset(blobData, '/x', size);
	}

	~MyBlobP() {}
	void MyBlobP::oWrite(OOStream* out)const;
	void MyBlobP::oAttach(OFile* file, bool deep);
	void MyBlobP::oDetach(OFile* file, bool deep);

	OMeta* meta(void)const { return &_metaClass; }
	// Define iterator for this class.
	typedef OIteratorT<MyBlobP, cMyBlobP> It;

private:
	// ObjectFile Instantiation function
	static OPersist* New(OIStream* s) { return new MyBlobP(s); }

	static OMeta _metaClass;
	OBlobP _blob;
};
// Declare BlobFile meta class.
OMeta MyBlobP::_metaClass(cMyBlobP, (Func)MyBlobP::New, cOPersist, 0);

void MyBlobP::oWrite(OOStream* out)const
// Write persistant data to the stream. This should always be overidden
// when there is persistant data. The first method called should be the inherited
// oWrite().
{
	inherited::oWrite(out);
	_blob.oWrite(out);
}

void MyBlobP::oAttach(OFile* file, bool deep)
// Attach this object to the file. That is, make the object persistant.
// This should be overridden if the object manages the persistance of
// other objects derived from OPersist.
{
	inherited::oAttach(file, deep);
	_blob.oAttach(file);
}

void MyBlobP::oDetach(OFile* file, bool deep)
// Detach this object from the file. That is, make the object non-persistant.
// This should be overridden if the object manages the persistance of
// other objects derived from OPersist.
{
	inherited::oDetach(file, deep);
	_blob.oDetach(file);
}

// This test works with MyClass10 normally, however, if you uncomment the following line it will
// work with MyBlobP instead. On a 64 bit configuration you can test raelly large files with MyBlobB.
//#define MyClass10 MyBlobP

class Timer {
public:
	void start(void) {
		_start = clock();
	}
	float read(void) {
		return((float)(clock() - _start) / CLOCKS_PER_SEC);
	}
	void printResult(const char* label, long obCount, float time)
	{
		char str1[20];
		sprintf(str1, "%.5f", time);

		cout << label << obCount << " Time " << str1 << '\n';
	}
private:
	clock_t _start;
};


static void my_new_handler()
// new_handler for OPersist objects. Activated when the object
// sThreshold is reached.
// Exceptions: OFileThresholdErr is thrown if after purging there is no
// space left.
{
	// Purge 10% of the object sThreshold. This is
	//  good when randomly accessing a large file.
	long objectsPurged = 0;
	OFile* f = OFile::getFirstOFile();

	// Try to recover memory just by purging.
	while (f)
	{
		objectsPurged += f->purge(cOPersist, true, max(1, 10 * sThreshold / 100));
		if (objectsPurged >= 10 * sThreshold / 100)
			break;
		f = f->getNextOFile();
	}

	// If we did not succede then throw an exception.
	if (OFile::getObjectCacheCount() >= OFile::getObjectThreshold())
		throw OFileThresholdErr("Object sThreshold exceeded");

	return;
}


int main()
{
	long nObjects, avSize;
	long count;

	cout << "ObjectFile Benchmark.\n\n";

	int menu = 0;
	while (menu != 5)
	{

		cout << "1. Set object sThreshold\n2. Write\n3. Sequential read\n4"
			". Random read\n5. Exit\n?";
		cin >> menu;

		Timer timerAll;
		timerAll.start();

		switch (menu)
		{
		case 1:
			cout << "Object sThreshold ?", cin >> sThreshold;
			OFile::setObjectThreshold(sThreshold);
			break;

		case 2:
			cout << "Number of objects ?", cin >> nObjects;
			cout << "Average object size in bytes ?", cin >> avSize;


			try
			{
				// Create a file
				OFile f("ofile.tst", OFILE_CREATE);

				// Set the default handler that is good for sequentially
				// writing a large file.
				OFile::set_new_handler(OFile::new_handler);

				// Automaticatally save if needed
				f.setAutoCommit();

				Timer timer;
				timer.start();
				// Attach
				for (long i = 1; i <= nObjects; i++)
				{
					MyClass10* p = new MyClass10(avSize);
					f.attach(p);
					p->oSetPurgeable();
					//cout << i << " objects written.\r";

					if (i % 1000 == 0)
					{
						timer.printResult("Objects ", i, timer.read());
						timer.start();
					}

				}

				// Commit objects that were not automatically committed.
				f.commit();

			}
			catch (OFileErr x) {
				cout << '\n' << x.why() << '\n';
			}
			timerAll.printResult("Objects ", nObjects, timerAll.read());

			cout << '\n';
			break;

		case 3:
			try
			{
				// Open the file
				OFile f("ofile.tst", OFILE_OPEN_READ_ONLY);

				OIterator it(&f);

				OPersist* p;
				count = 1;
				Timer timer;
				timer.start();

				// Iterate sequentially over all objects.
				while ((p = it++))
				{
					p->oSetPurgeable();
					if (count % 1000 == 0)
					{
						timer.printResult("Objects ", count, timer.read());
						timer.start();
					}
				}


			}
			catch (OFileErr x) {
				cout << '\n' << x.why() << '\n';
			}
			timerAll.printResult("Objects ", count, timerAll.read());


			cout << '\n';
			break;

		case 4:
			try
			{
				// Open the file
				OFile f("ofile.tst", OFILE_OPEN_READ_ONLY);

				// Set a handler that is good for randomly accessing a large file.
				OFile::set_new_handler(my_new_handler);
				Timer timer;
				timer.start();

				nObjects = f.objectCount();
				// Iterate sequentially over all objects.
				for (long i = 1; i < nObjects; i++)
				{
					OPersist* p = f.getObject(rand() % nObjects);
					if (p)
					{
						p->oSetPurgeable();
						if (i % 1000 == 0)
						{
							timer.printResult("Objects ", i, timer.read());
							timer.start();
						}
					}
				}


			}
			catch (OFileErr x) {
				cout << '\n' << x.why() << '\n';
			}
			timerAll.printResult("Objects ", nObjects, timerAll.read());

		default:;
		} // switch


	} // while
	return 0;
}


