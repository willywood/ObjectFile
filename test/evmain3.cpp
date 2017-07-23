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
#include <iostream>
#include "ofstring.h"
#include "ofile.h"
#include "oiter.h"
#include "evper2.h"
#include "oufile.h"
#include "ox.h"
#include "oiter.h"

#ifdef OFILE_STD_IN_NAMESPACE
using std::cout;
using std::cin;
#endif


void OUFile::convert(void)
// Convert objects from older versions.
// Called by the constructor of OUFile.
{

	if(userSourceVersion() < userVersion())
		throw OFileIOErr(_fileName,"The application file has a newer version than this"
									"program.");
	if(userSourceVersion() > userVersion())
    // Only if the file is of an older version than the current sources.
	{
		//  Iterate over all objects to make sure they are read. This
		// ensures that they get converted.
		EvPerson::It it(this);

		EvPerson *per;
		while(per = it++);
	}

}

int main(){

	OFile::setUserSourceVersion(3);

	OUFile *f = new OUFile("evperson.of2",OFILE_OPEN_FOR_WRITING, "~evperson.of2");

	// Write the file. This could fail if the disk is full.
	try{
		f->commit();
	}catch(OFileErr x){
		cout << x.why() << '\n';
		return 0;
	}

	cout << "Saving file...\n";
	f->saveAs("evperson.of3");
	cout << "...File saved\n";
	delete f;

	return 0;
}
