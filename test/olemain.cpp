//
// Demonstrate ObjectFile saving and reading from an OLE compound document.
//
//
//#include "cstring.h"
#include "odefs.h"
#include "ofile.h"
#include "oiter.h"
#include "person.h"
#include "ox.h"
#include "oiter.h"
#include "trans.h"

using namespace std;

int main(){


	HRESULT hr = OleInitialize(NULL);
	oFAssert(hr == S_OK);

	// If an Compound document 'test.doc' exists, the data will be saved to
	// it. If not, a new regular file will be created.

	const wchar_t *wfname = L"test.doc";
/*
	// OLE, bless it, requires a UNICODE string.
	wchar_t wfname[256];
    int slen = MultiByteToWideChar(CP_ACP,0,fname,strlen(fname),wfname,256);
	// Null terminate returned string
    wfname[slen] = 0;
*/
	IStorage *istorage;
    // Try opening an existing document
	hr = StgOpenStorage(wfname,
	                            NULL,
		                        STGM_DIRECT|STGM_READWRITE|STGM_SHARE_EXCLUSIVE,
				                NULL,
				                0,
				                &istorage);
	if(FAILED(hr))
		// Try creating the document.
			hr = StgCreateDocfile(wfname,
			                      STGM_DIRECT|STGM_READWRITE|STGM_SHARE_EXCLUSIVE,
				                  0,
				                  &istorage);


	OFile *f = new OFile(istorage,(char *)L"OFILE",OFILE_CREATE|OFILE_OPEN_FOR_WRITING,
						 STGM_DIRECT|STGM_READWRITE|STGM_CREATE|STGM_SHARE_EXCLUSIVE);

	Person *husband = new Person("John","Doe","13 Farmers Lane","South Bronx","New York","USA",23456,70,1.80,33,0,
	new WristWatch("Omega"));

	// attach into file
	f->attach(husband);

	Person *wife = new Person("Jane","Doe","13 Farmers Lane","South Bronx","New York","USA",23456,65,1.50,18,husband,
	new WristWatch("Rolex"));

	husband->setSpouse(wife);
	f->attach(wife);

	husband = new Person("Willy","Wood","13 Perry Road","Toronto"," ","Canada",23456,70,1.80,33,0,
	new WristWatch("Timex"));


	wife = new Person("Janet","Wood","13 Perry Road","Toronto","  ","Canada",23456,65,1.50,18,husband,
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

	// Write the file. This could fail if the disk is full.
	try{
		f->commit();
	}catch(OFileErr x){
		cout << x.why() << '\n';
		return 0;
	}

//	cout << "Saving file...\n";
//	f = f->saveAs("person1.ofl");
//	cout << "...File saved\n";
	delete f;

	istorage->Release();
//====================== R E A D   F I L E =======================

	cout << "Reading file.\n";
    // Open an existing document
	hr = StgOpenStorage(wfname,
	                            NULL,
	                            STGM_DIRECT|STGM_READ|STGM_SHARE_EXCLUSIVE,
				                NULL,
				                0,
				                &istorage);

	f = new OFile(istorage,(char *)L"OFILE",OFILE_OPEN_READ_ONLY,
						 STGM_DIRECT|STGM_READ|STGM_SHARE_EXCLUSIVE);

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
	istorage->Release();

	OleUninitialize();
	return 0;
}