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

using namespace std;

static long threshold;

class Timer{
public:
	void start(void){
		_start = clock();
	}
	float read(void){
		return((float)(clock() - _start)/CLOCKS_PER_SEC);
	}
	void printResult(const char *label,long obCount,float time)
	{
		char str1[20];
		sprintf(str1,"%.5f",time);

		cout << label << obCount << " Time " << str1  <<'\n';
	}
private:
	clock_t _start;
};


static void my_new_handler()
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
		objectsPurged += f->purge(cOPersist,true,max(1,10*threshold/100));
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
long count;
float totTime  = 0.0;

	cout << "ObjectFile Benchmark.\n\n";

	int menu;
	while(menu != 5)
	{

	   cout << "1. Set object threshold\n2. Write\n3. Sequential read\n4"
			   ". Random read\n5. Exit\n?";
	   cin >> menu;

     	Timer timerAll;
      timerAll.start();

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

     	   Timer timer;
         timer.start();
 		   // Attach
		   for(long i = 1; i <= nObjects ; i++)
		   {
			   MyClass10 *p = new MyClass10(avSize);
			   f.attach(p);
			   p->oSetPurgeable();
			   //cout << i << " objects written.\r";

            if(i % 1000 == 0)
            {
        	         timer.printResult("Objects ",i,timer.read());
                  timer.start();
            }

		   }

		   // Commit objects that were not automatically committed.
		   f.commit();

	   }catch(OFileErr x){
		   cout << '\n' << x.why() << '\n';
	   }
      timerAll.printResult("Objects ",nObjects,timerAll.read());

	   cout << '\n';
	   break;

   case 3:
	   try
	   {
		   // Open the file
		   OFile f("ofile.tst",OFILE_OPEN_READ_ONLY);

		   OIterator it(&f);

		   OPersist *p;
		   count = 1;
     	   Timer timer;
         timer.start();
 
		   // Iterate sequentially over all objects.
		   while((p = it++))
		   {
			   p->oSetPurgeable();
            if(count % 1000 == 0)
            {
        	         timer.printResult("Objects ",count,timer.read());
                  timer.start();
            }
		   }


	   }catch(OFileErr x){
		   cout << '\n' << x.why() << '\n';
	   }
      timerAll.printResult("Objects ",count,timerAll.read());


	   cout << '\n';
	   break;

   case 4:
	   try
	   {
		   // Open the file
		   OFile f("ofile.tst",OFILE_OPEN_READ_ONLY);

		   // Set a handler that is good for randomly accessing a large file.
		   OFile::set_new_handler(my_new_handler);
     	   Timer timer;
         timer.start();

		   nObjects = f.objectCount();
		   // Iterate sequentially over all objects.
		   for(long i = 1;i < nObjects; i++)
		   {
			   OPersist *p = f.getObject(rand() % nObjects);
			   if(p)
			   {
				   p->oSetPurgeable();
               if(i % 1000 == 0)
               {
        	            timer.printResult("Objects ",i,timer.read());
                     timer.start();
               }
			   }
		   }


	   }catch(OFileErr x){
		   cout << '\n' << x.why() << '\n';
	   }
      timerAll.printResult("Objects ",nObjects,timerAll.read());

	   default: ;
	   } // switch


	   } // while
	return 0;
}


