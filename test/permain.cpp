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
#include <fstream>
#include <iostream>
#include "ofile.h"
#include "oiter.h"
#include "person.h"
#include "oufile.h"
#include "ox.h"
#include "oiter.h"
#include "trans.h"
#include "oosxml.h"
#include "oisxml.h"

#ifdef OFILE_STD_IN_NAMESPACE
using std::cout;
using std::cin;
using std::fstream;
using std::ifstream;
using std::ios;
#endif

int main(){

	OUFile *f = new OUFile(0,OFILE_CREATE);

	Person *husband = new Person("John","Doe","13 Farmers Lane","South Bronx","New York","USA",23456,70.0,1.80F,33,0,
	new WristWatch("Omega"));

	// attach into file
	f->attach(husband);

	Person *wife = new Person("Jane","Doe","13 Farmers Lane","South Bronx","New York","USA",23456,65.0,1.50,18,husband,
	new WristWatch("Rolex"));

	husband->setSpouse(wife);
	f->attach(wife);

	husband = new Person("William","Smith","13 Bogside Road","Wales"," ","United Kingdom",23456,70.0 ,1.80F,33,0,
	new WristWatch("Timex"));


	wife = new Person("Anne","Smith","13 Bogside Road","Wales","  ","United Kingdom",23456,65.0 ,1.50,18,husband,
	new WristWatch("Swatch"));

	husband->setSpouse(wife);

	// attach into file
	f->attach(husband);
	f->attach(wife);


	oFAssert(f->objectCount(cWristWatch) == 4);
	oFAssert(f->objectCount(cPerson) == 4);

	// Now detach and reattach to demonstrate that object relationships can be maintained
	// in and out of the file.
	f->detach(husband);
	f->detach(wife);

	oFAssert(f->objectCount(cWristWatch) == 2);
	oFAssert(f->objectCount(cPerson) == 2);

	f->attach(husband);
	f->attach(wife);

	// Test multiple inheritence and virtual base class.
	Hybrid *hybrid = new Hybrid("ElectroFerrari",200,4,10000,12.0,10.0,50.0);
	cout << hybrid->chargeTime() <<'\n';
	f->attach(hybrid);

	oFAssert(f->objectCount(cHybrid) == 1);

	// Erase and attach
	f->detach(hybrid);
	oFAssert(f->objectCount(cHybrid) == 0);
	f->attach(hybrid);

	try{
	// Write the file. This could fail if the disk is full.
		f->commit();

	cout << "Saving file...\n";
	f->saveAs("person1.ofl");
	cout << "...File saved\n";

    // Write in XML it, using OOStreamXML
    {
		// Open a file stream
	    fstream out("person1.xml",ios::out);

		// Open a print stream on the file stream
		OOStreamXML print(f,out);
        print.writeObjects("person1");
    } 
     // Write in XML it, using OOStreamXML
    {
		// Open a file stream
	    fstream out("person1_sax.xml",ios::out);

		// Open a print stream on the file stream
		OOStreamXML print(f,out);
        print.setXMLStyle(OOStreamXML::cSAX);
        print.writeObjects("person1_sax");
    }
    {
		// Open a file stream
	    fstream out("person1_flat.xml",ios::out);

		// Open a print stream on the file stream
		OOStreamXML print(f,out);
        print.setXMLStyle(OOStreamXML::cFlat);
        print.writeObjects("person1_flat");
    } 
	{
		// Open a file stream
		ifstream in("person1_sax.xml",ios::in|ios::binary);

		// Open a print stream on the file stream
		OIStreamXML read(f,in);
        read.readObjects();
    }
     // Print it again, using OOStreamPrint
    {
		// Open a file stream
	    fstream out("person1_sax2.xml",ios::out);

		// Open a print stream on the file stream
		OOStreamXML print(f,out);
        print.setXMLStyle(OOStreamXML::cSAX);
        print.writeObjects("person");
    }

	}catch(OFileErr x){
		cout << x.why() << '\n';
		return 0;
	}


	delete f;

//====================== R E A D   F I L E =======================

	cout << "Reading file.\n";
	f = new OUFile("person1.ofl",OFILE_OPEN_READ_ONLY);

	oFAssert(f->objectCount(cWristWatch) == 4);
	oFAssert(f->objectCount(cPerson) == 4);
	oFAssert(f->objectCount(cHybrid) == 1);

	// Typesafe access to the objects... 

	// Person iterator
	Person::It it(f);

	husband = (*it);
	wife = husband->spouse();

	cout << husband->watch() << " " << wife->watch() <<'\n';


	// Hybrid iterator
	Hybrid::It hit(f);

	hybrid = (*hit);

	cout << hybrid->chargeTime() <<'\n';


	delete f;

	return 0;
}