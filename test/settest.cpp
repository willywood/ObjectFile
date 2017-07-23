// Test OSet

#include "odefs.h"
#include <iostream>
#include "oufile.h"
#include "oisxml.h"
#include "oosxml.h"
#include "ox.h"
#include "myclass.h"

#ifdef OFILE_STD_IN_NAMESPACE
using std::cout;
using std::cin;
using std::fstream;
using std::ios;
#endif


int main()
{
	try{
		cout << "Writing file\n";

		OUFile file("settest.db",OFILE_CREATE);

		MyClass3 *father = new MyClass3();

		// Add ten children and properties to father
		for(int i = 0 ; i < 10; i++)
		{
			father->addChild(new MyClass3());
			father->addProperty(new MyClass3());
		}

		// attach the father (and all its children and properties) to the file
		file.attach(father);

		// detach the father (and all its children and properties) to the file
		file.detach(father);

		oFAssert(file.objectCount(cMyClass3) == 0);

		// attach the father (and all its children and properties) to the file
		file.attach(father);

		oFAssert(file.objectCount(cMyClass3) == 21);

		oFAssert(father->children().size() == 10);

		// Test copy constructor of set
		OSet children1(father->children());
		oFAssert(children1.size() == 10);

		// Test copy operator of set
		OSet children2 = father->children();
		oFAssert(children2.size() == 10);

		file.commit();

		file.save();

	}catch(OFileErr x){
		cout << x.why() << '\n';
		return -1;
	}


	try{
		cout << "Reading file\n";

		OUFile file("settest.db",OFILE_OPEN_READ_ONLY);


		oFAssert(file.objectCount(cMyClass3) == 21);

	   {
			// Open a file stream
			fstream out("settest.xml",ios::out);

			// Output the objects to the the file stream
			OOStreamXML print(&file,out,false);
			print.setXMLStyle(OOStreamXML::cSAX);
			print.writeObjects("settest");
		}
		{
			// Create a new OFile.
			OUFile fileFromXML("settestFromXML.db",OFILE_CREATE);

			// Open a file stream
			fstream in("settest.xml",ios::in|ios::binary);

			// Read the objects from the file stream.
			OIStreamXML read(&fileFromXML,in);
			read.readObjects();

			// Save the file.
			fileFromXML.commit();
			fileFromXML.save();

		}
	   {
			// Open a file stream
			fstream out("settest2.xml",ios::out);

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
