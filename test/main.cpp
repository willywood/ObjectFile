//
// ObjectFile test program. This simply exercises ObjectFile.
//

#include "odefs.h"
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include "ofile.h"
#include "oiter.h"
#include "myclass.h"
#include "oufile.h"
#include "ox.h"
#include "oosxml.h"
#include "oisxml.h"
#include "oosjson.h"
#include "mycolct.h"

using namespace std;

int main(){


// Test multiple files with oFileOf method.

	OUFile *files[10];
	MyClass *objects[10];
   int i;
	for(i = 0; i < 10; i++)
	{
		char fname[20];
		char fname_backup[20];
		sprintf(fname,"f%d.db",i);
		sprintf(fname_backup,"~f%d.db",i);

		files[i] = new OUFile(fname,OFILE_CREATE, fname_backup);

		objects[i] = new MyClass(100);

		files[i]->attach(objects[i],true);

	}

	for(i = 0 ; i < 10 ; i++)
	{
		oFAssert(OFile::oFileOf(objects[i]) == files[i]);
	}

	for(i = 0 ; i < 10 ; i++)
		delete files[i];

//

OUFile *file = 0 ;

	long  cIter;

	while(cout << "Iterations ?",cin >> cIter,cIter){
	delete file;
	file = new OUFile("ofile.tst",OFILE_CREATE,"~ofile.tst");

	oFAssert(file->isDirty());

	MyCollect *col = new MyCollect();
	file->attach(col);

	cout << "Creating Set "<< cIter*5 <<"\n";
	for(long l = 0;l < cIter; l++){
		file->attach(new MyClass(i),true);
		file->attach(new MyClass1(201));
		file->attach(new MyClass2);
		file->attach(new MyClass3);
		file->attach(new MyClass4);
	}
	cout << "finished\n";

	cout << "Starting search\n";
	for(long j = 0;j < cIter*5; j++){
		OPersist *ob3 = file->getObject(rand() % cIter*5);
	}
	cout << "end search\n";

//	f->print();

	cout << "\n Testing the iterator ++a\n" ;
	OIterator it(file,cMyClass1);
	OPersist *p = *it;
	col->push_back(p);
	while((p = ++it)){
		cout << p->oId() << " ";
		// Add to collection
		col->push_back(p);
	}
	cout <<'\n';

	oulong count1 = file->objectCount(cOPersist);
	// detach deeply
	file->detach(col,true);
	file->attach(col,true);

	// Set as root object
	file->setRoot(col);

	oFAssert(count1 == file->objectCount(cOPersist));

	cout << "\n Testing the iterator a++\n" ;
	OIterator it2(file,cOPersist);
	while((p = it2++)){
		cout << p->oId() << " ";
	}
	cout <<'\n';

	// Write the file
	try{
		file->commit(true);

		oFAssert(!file->OFile::isDirty() && file->OUFile::isDirty());

		for(i = 0; i < 5 ;i++){
			cout <<"committing \n";

			// Add a new object each time
			MyClass *mc = new MyClass(i);
			file->attach(mc,true);
			oFAssert(file->isDirty());
			file->commit();

			// Will not refer to it again.
			mc->oSetPurgeable();
		}
	}catch(OFileErr x){
		cout << x.why() << '\n';
	}

	// test purging
	long purged = file->purge();
    cout << purged << " objects purged\n";

	cout << "Saving file...\n";
	file->save("ofile.bak");
	file->saveAs("ofile2.tst");
	cout << "...File saved\n";
	delete file;

	cout << "Reading file.\n";
	file = new OUFile("ofile2.tst",OFILE_OPEN_READ_ONLY, "~ofile2.tst");

	oFAssert(!file->isDirty());

	OIterator colit(file,cMyCollect);
	col = (MyCollect *)*colit;

	cout <<"Collection contains "<< col->size() <<" elements\n";

	cout << "Iterating...\n" ;
	OIterator it1(file,cMyClass);
	MyClass *mc;
	while((mc = (MyClass *)it1++))
		mc->print();


//	oFAssert(!f->isDirty());
//	f->print();
	cout << "...finished\n";


   {
		// Open a file stream
	    fstream out("ofile.xml",ios::out);

		// Output the objects to the the file stream
		OOStreamXML print(file,out,false);
        print.setXMLStyle(OOStreamXML::cSAX);
        print.writeObjects("ofile");
    }
   {
	   // Open a file stream
	   fstream out("ofile.json", ios::out);

	   // Output the objects to the the file stream
	   OOStreamJSON print(file, out, false);
	   print.setXMLStyle(OOStreamJSON::cSAX);
	   print.writeObjects("ofile");
   }

	{
		// Create a new OFile.
		 OUFile fileFromXML("ofileFromXML.db",OFILE_CREATE,"~ofileFromXML.db");

		// Open a file stream
		fstream in("ofile.xml",ios::in|ios::binary);

		// Read the objects from the file stream.
		OIStreamXML read(&fileFromXML,in);
        read.readObjects();

		// Save the file.
		fileFromXML.commit();
		fileFromXML.save();

    }
     // Print it again, using OOStreamXML
    {
		 OUFile fileFromXML("ofileFromXML.db",OFILE_OPEN_READ_ONLY,"~ofileFromXML.db");
		// Open a file stream
	    fstream out("ofile2.xml",ios::out);

		// Open a print stream on the file stream
		OOStreamXML xml(&fileFromXML,out,false);
        xml.setXMLStyle(OOStreamXML::cSAX);
        xml.beginDocument("ofile");

		OIterator it(&fileFromXML);
		OPersist *ob;

		while((ob = it++))
		{
			xml.writeObjectAsXML(ob);
		}

        xml.endDocument();
    }

	} // main loop
	delete file;

	return 0;
}
