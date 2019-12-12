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
#include <time.h>

#include "ofile.h"
#include "oiter.h"
#include "ox.h"
#include "mmyclass.h"

#ifdef OFILE_STD_IN_NAMESPACE
using std::min;
#endif

using namespace std;

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
		char str2[20];
		sprintf(str1,"%.3f",time);
		sprintf(str2,"%.3f",time*1000/obCount);

		cout << label << str1 << "          "<< str2 <<'\n';
	}
private:
	clock_t _start;
};



#define CREATE_OBJECT(n)   \
	p[obCount++] = new MyClass##n(rand() % avSize*2); \
	if(++classCount == nClasses) continue;

int main()
{
long nClasses,nObjects,avSize;

	cout << "ObjectFile Benchmark.\n\n";

	while(cout << "Number of classes(maximum 55) ?",cin >> nClasses,nClasses)
	{
	// The 55 limit is only because of Borland 16-bit compiler and
	// has nothing to do with the speed limit.
	//nClasses = min(nClasses,55L); // prevent crash

	cout << "Number of objects ?",cin >> nObjects;
	//nClasses = min(nObjects,nClasses);
	cout << "Average object size in bytes ?",cin >> avSize;

	cout << "\n                 Total(secs)    Per Object(ms)\n";

	OFile *file = 0;
	long obCount = 0;
	OPersist **p = new OPersist*[nObjects];
	long i;

	long loops = (nClasses != 0) ? nObjects/nClasses : 0;
	for(i = 0; i < loops ; i++){
		long classCount = 0;

		// Create objects of different classes.
		CREATE_OBJECT(10);
		CREATE_OBJECT(11);
		CREATE_OBJECT(12);
		CREATE_OBJECT(13);
		CREATE_OBJECT(14);
		CREATE_OBJECT(15);
		CREATE_OBJECT(16);
		CREATE_OBJECT(17);
		CREATE_OBJECT(18);
		CREATE_OBJECT(19);
		CREATE_OBJECT(20);
		CREATE_OBJECT(21);
		CREATE_OBJECT(22);
		CREATE_OBJECT(23);
		CREATE_OBJECT(24);
		CREATE_OBJECT(25);
		CREATE_OBJECT(26);
		CREATE_OBJECT(27);
		CREATE_OBJECT(28);
		CREATE_OBJECT(29);
		CREATE_OBJECT(30);
		CREATE_OBJECT(31);
		CREATE_OBJECT(32);
		CREATE_OBJECT(33);
		CREATE_OBJECT(34);
		CREATE_OBJECT(35);
		CREATE_OBJECT(36);
		CREATE_OBJECT(37);
		CREATE_OBJECT(38);
		CREATE_OBJECT(39);
		CREATE_OBJECT(40);
		CREATE_OBJECT(41);
		CREATE_OBJECT(42);
		CREATE_OBJECT(43);
		CREATE_OBJECT(44);
		CREATE_OBJECT(45);
		CREATE_OBJECT(46);
		CREATE_OBJECT(47);
		CREATE_OBJECT(48);
		CREATE_OBJECT(49);
		CREATE_OBJECT(50);
		CREATE_OBJECT(51);
		CREATE_OBJECT(52);
		CREATE_OBJECT(53);
		CREATE_OBJECT(54);
		CREATE_OBJECT(55);
		CREATE_OBJECT(56);
		CREATE_OBJECT(57);
		CREATE_OBJECT(58);
		CREATE_OBJECT(59);
		CREATE_OBJECT(60);
		CREATE_OBJECT(61);
		CREATE_OBJECT(62);
		CREATE_OBJECT(63);
		CREATE_OBJECT(64);
		CREATE_OBJECT(65);
/*		CREATE_OBJECT(66);   // My 16-bit compiler breaks down here
		CREATE_OBJECT(67);
		CREATE_OBJECT(68);
		CREATE_OBJECT(69);
		CREATE_OBJECT(70);
		CREATE_OBJECT(71);
		CREATE_OBJECT(72);
		CREATE_OBJECT(73);
		CREATE_OBJECT(74);
		CREATE_OBJECT(75);
		CREATE_OBJECT(76);
		CREATE_OBJECT(77);
		CREATE_OBJECT(78);
		CREATE_OBJECT(79);
		CREATE_OBJECT(80);
		CREATE_OBJECT(81);
		CREATE_OBJECT(82); */
	}

Timer timer;
float totTime  = 0.0;

	// Create a file
	file = new OFile("ofile.tst",OFILE_CREATE,"ofbm");

   // Using fast find should speed things up(unless you have a lousy memory manager,
   // as in MSVC 4.1)
//	OFile *f = new OFile("ofile.tst",OFILE_CREATE|OFILE_FAST_FIND);

	// Attach 
	timer.start();
	for(i = 0; i < obCount ; i++){
//*******************************************************************
		file->attach(p[i]);
//*******************************************************************
	}
	timer.printResult("Attach           ",obCount,timer.read());
	totTime += timer.read();

	// Access
	timer.start();
	for(i = 0; i < obCount ; i++)
	{
	// Worst case scenario, by specifying lowest possible resolution.
//*******************************************************************
		OPersist *ob = file->getObject(p[i]->oId(),cOPersist);
//*******************************************************************
	}
	timer.printResult("Access           ",obCount,timer.read());
	totTime += timer.read();

	// Save
	timer.start();
//*******************************************************************
	try{
		file->commit();
	}catch(OFileErr &x){
		cout << x.why();
	}
//*******************************************************************
	timer.printResult("Save             ",obCount,timer.read());
	totTime += timer.read();

	// Incremental Save
	timer.start();
//*******************************************************************
	// Set a single object dirty
	p[obCount/2]->oSetDirty();
	try{
		file->commit();
	}catch(OFileErr &x){
		cout << x.why();
	}
//*******************************************************************
	timer.printResult("Incremental save ",obCount,timer.read());
	totTime += timer.read();

	delete file;
	delete []p;

	// Load
	p = new OPersist*[nObjects + 1];
	timer.start();
//*******************************************************************
	//  Reopen the file
	file = new OFile("ofile.tst",OFILE_OPEN_READ_ONLY,"ofbm");
	// Declare iterator for all objects in the file
	OIterator it(file);
	i = 0;
	// Iterate over objects to make sure each one is brought into memory.
	while((p[i++] = it++)){}
//*******************************************************************
	if(i-1 != obCount)
		oFAssert(i-1 == obCount);

	timer.printResult("Load             ",obCount,timer.read());
	totTime += timer.read();

	// Erase
	timer.start();
	for(i = 0; i < obCount ; i++)
	{
//*******************************************************************
		// Detach the object from the file
		file->detach(p[i]);
//*******************************************************************
	}
	timer.printResult("Detach           ",obCount,timer.read());
	totTime += timer.read();


	cout << "================================\n";
	timer.printResult("Total            ",obCount,totTime);

	delete file;

	// Delete the now non-persistent objects.
	for(i = 0; i < obCount ; i++)
		delete p[i];
	delete []p;

	} // end main while
}

