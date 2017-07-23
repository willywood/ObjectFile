// Test OSet

#include "odefs.h"
#include "odrel.h"
#include <iostream>
#include "oufile.h"
#include "ox.h"
#include "oosxml.h"
#include "oisxml.h"

using namespace std;

int main()
{
	try{
		cout << "Writing file\n";

		OUFile file("relation.db",OFILE_CREATE,"~relation.db");

		for(int i = 0; i < 100 ; i++)
		{
			Relation *husband = new Relation("John");
			Relation *wife = new Relation("Jane");
			husband->setSpouse(wife);
			wife->setSpouse(husband);

			file.attach(husband);
			file.attach(wife);

			file.detach(husband);
			file.detach(wife);

			file.attach(husband);
			file.attach(wife);
		}

		file.commit();

		file.save();

	}catch(OFileErr x){
		cout << x.why() << '\n';
		return -1;
	}


	try{
		cout << "Reading file\n";

		OUFile file("relation.db",OFILE_OPEN_READ_ONLY,"~relation.db");

		Relation::It it(&file);

		int count = 0;

		Relation *r;
		while((r = it++))
		{
			cout << r->name() << " - " << r->spouse()->name() <<'\n';
			count++;
		}
		oFAssert(count == 200);

   {
		// Open a file stream
	    fstream out("relation.xml",ios::out);

		// Open a print stream on the file stream
		OOStreamXML print(&file,out);
        print.setXMLStyle(OOStreamXML::cSAX);
        print.writeObjects("relation");
    }
	{
		 OUFile fileFromXML("relationFromXML.db",OFILE_CREATE, "~relationFromXML.db");

		// Open a file stream
		fstream in("relation.xml",ios::in|ios::binary);

		// Open a print stream on the file stream
		OIStreamXML read(&fileFromXML,in);
        read.readObjects(1);
		fileFromXML.commit();

		fileFromXML.save();

		Relation::It it(&fileFromXML);

		int count = 0;

		Relation *r;
		while((r = it++))
		{
			cout << r->name() << " - " << r->spouse()->name() <<'\n';
			count++;
		}
		oFAssert(count == 200);
    }
     // Print it again, using OOStreamPrint
    {
		 OUFile fileFromXML("relationFromXML.db",OFILE_OPEN_READ_ONLY, "~relationFromXML.db");
		// Open a file stream
	    fstream out("relation2.xml",ios::out);

		// Open a print stream on the file stream
		OOStreamXML print(&fileFromXML,out);
        print.setXMLStyle(OOStreamXML::cSAX);
        print.writeObjects("relation");
    }

	}catch(OFileErr x){
		cout << x.why() << '\n';
		return -1;
	}

	cout << "Done\n";

	return 0;
}
