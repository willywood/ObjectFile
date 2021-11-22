//
// Several ObjectFile abilities are demonstrated.
// (i)  Complex relationships between objects.
// (ii) The ability for objects to exist in and out of file, and to
//      maintain those relationships.
// (iii)OnDemand attributes. Persons _spouse and _watch are OnDemand attributes
//      of Person
// (iv) Muliply inherited class Hybrid, with a virtual base class.
//      (just like any other class)
//

#include "odefs.h"
#include "ofstring.h"
#include "ofile.h"
#include "oiter.h"
#include "evperson.h"
#include "oufile.h"
#include "ox.h"
#include "oiter.h"
#include <iostream>

#ifdef OFILE_STD_IN_NAMESPACE
using std::cout;
#endif


void OUFile::convert(void)
// Convert objects from older versions.
// Called by the constructor of OUFile.
{
	long us = userSourceVersion();
	long uv = userVersion();

	if (us < uv)
	//if (userSourceVersion() < userVersion())
	{
		throw OFileIOErr(_fileName, "The application file has a newer version than this"
			" program.");
	}
}

int main(){

	OFile::setUserSourceVersion(1);

	OUFile *f = new OUFile(0,OFILE_CREATE);

	for(int i = 0;i < 100; i++)
	{
		// create object and attach into file.
		f->attach(new EvPerson());
	}

	// Write the file. This could fail if the disk is full.
	try{
		f->commit();
	}catch(OFileErr x){
		cout << x.why() << '\n';
		return 0;
	}

	cout << "Saving file...\n";
	f->saveAs("evperson.of1");
	cout << "...File saved\n";
	delete f;

//====================== R E A D   F I L E =======================

	cout << "Reading file.\n";
	f = new OUFile("evperson.of1",OFILE_OPEN_READ_ONLY, "~evperson.of1");

	oFAssert(f->objectCount(cEvPerson) == 100);

	// Typesafe access to the objects... 

	//  iterator
	EvPerson::It hit(f);

	EvPerson *per = (*hit);

//	cout << hybrid->chargeTime() <<'\n';


	delete f;

	// Test case where file is a newer version than the code.Should fail.
	try{
		f = new OUFile("evperson.of2",OFILE_OPEN_READ_ONLY, "~evperson.of2");
	}catch(OFileIOErr x){
		cout << x.why() <<'\n';
      return 0;
	}
   delete f;

	return 0;
}
