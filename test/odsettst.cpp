// Test OnDemandSet
//
// Objects must be in the file before they can be pput in the OnDemandSet
//

#include "odefs.h"
#include <fstream>
#include <iostream>
#include "oufile.h"
#include "oisxml.h"
#include "oosxml.h"
#include "ox.h"
#include "myclass5.h"

#ifdef OFILE_STD_IN_NAMESPACE
using std::cout;
using std::cin;
using std::fstream;
using std::ios;
#endif

int main()
{
OId fatherId;

	try{
		cout << "Writing file\n";

		OUFile file("odsettst.db",OFILE_CREATE, "~odsettst.db");

		MyClass5 *father = new MyClass5();

		// Add ten children and properties to father
		for(int i = 0 ; i < 10; i++)
		{
			OnDemand child(new MyClass5());
			father->addChild(child);

			MyClass5 *house =  new MyClass5();
			father->addHouse(house);
		}

		// attach the father (and all its children and properties) to the file
		file.attach(father);

		oFAssert(file.objectCount(cMyClass5) == 21);

		// detach the father (and all its children and properties) to the file
		file.detach(father);

		oFAssert(file.objectCount(cMyClass5) == 0);

		// attach the father (and all its children and properties) to the file
		file.attach(father);

		// get id for use when reading file
		fatherId = father->oId();

		oFAssert(file.objectCount(cMyClass5) == 21);

		oFAssert(father->children().size() == 10);

		file.commit();

		father->oSetPurgeable(true,&file);

		long purged = file.purge();

		cout << purged << " objects purged\n";

		file.save();

	}catch(OFileErr x){
		cout << x.why() << '\n';
		return -1;
	}


	try{
		cout << "Reading file\n";

		OUFile file("odsettst.db",OFILE_OPEN_READ_ONLY, "~odsettst.db");

		oFAssert(file.objectCount(cMyClass5) == 21);

		MyClass5 *father = (MyClass5 *)file.getObject(fatherId);

		OnDemandSet children = father->children();

		// Iterate over children
		for(OnDemandSet::const_iterator it = children.begin(); it != children.end(); ++it)
		{
			OnDemand child = (*it);
			// Objects are only actually read for (*it)->oId()
			cout << (*it).oId() << " - " << (*it)->oId() << '\n';

			// Remove the child
			father->removeChild(child);
			child.oDetach(&file,true);
		}

		oFAssert(file.objectCount(cMyClass5) == 11);

	   {
			// Open a file stream
			fstream out("odsettest.xml",ios::out);

			// Output the objects to the the file stream
			OOStreamXML print(&file,out,false);
			print.setXMLStyle(OOStreamXML::cSAX);
			print.writeObjects("settest");
		}
		{
			// Create a new OFile.
			OUFile fileFromXML("settestFromXML.db",OFILE_CREATE, "~settestFromXML.db");

			// Open a file stream
			fstream in("odsettest.xml",ios::in|ios::binary);

			// Read the objects from the file stream.
			OIStreamXML read(&fileFromXML,in);
			read.readObjects();

			// Save the file.
			fileFromXML.commit();
			fileFromXML.save();

		}
	   {
			// Open a file stream
			fstream out("odsettest2.xml",ios::out);

			// Output the objects to the the file stream
			OOStreamXML print(&file,out,false);
			print.setXMLStyle(OOStreamXML::cSAX);
			print.writeObjects("settest");
		}

	}catch(OFileErr x){
		cout << x.why() << '\n';
		return -1;
	}

	cout << "Done\n";

	return 0;
}
