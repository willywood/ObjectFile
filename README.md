# ObjectFile
Persistence for C++ objects

# Introduction
ObjectFile was developed in order to satisfy the need for object persistence within application programs. This need is often far removed from that of a full blown OODBMS system. For many applications a full OODBMS would be a burden, both to application performance and to the development process. 
ObjectFile is designed to be just another set of classes in your application. It has no pre-compiler and no binaries to link with. This makes it easy to integrate into your development environment.

Full documentation is in OFILEMAN.pdf

Key features are:
  * Very fast. It beats the CrystalDiskMark Random disk benchmark on a i7 laptop with SATA III ssd.
  * Full source code for easy porting and debugging.
  * Database versioning supported.
  * Byte order conversion supported for cross-platform support.
  * Memory efficient.
  * Objects can be in or out of memory.
  * BLOB support.
  * XML object and full database representation can be generated.
  * Import and export objects from xml.
  * Proven in a large desktop application.
  * Open source and free to use.


# Getting started - Linux
To build the examples on Linux:

**cd ofile/projects/Linux**

**make**

This will produce executable .out files for the examples.
Note: The compiler stl is used and not the provided code.

# Example Program
We will jump in at the deep end, by looking at an example program that uses ObjectFile. Do not worry if there is anything you do not understand. It will become clear after reading the rest of the manual. 
The program is a simple address database. It allows you to add, delete and list the addresses. It uses the Person class, which follows.
The ObjectFile related code is printed in bold text.
 
 ~~~~
#include "odefs.h"
#include <cstring.h>
#include <fstream.h>
#include "oufile.h"
#include "ox.h"
#include "person.h"

main(){

	try
	{

	cout << "ObjectFile Address Database Demo V0.0\n\n";

	// Create the file if it does not already exist, otherwise open it.
	OUFile *addressDB = new OUFile("address.db",OFILE_OPEN_FOR_WRITING|OFILE_CREATE);

	while(true)
	{
		char buf[256];
		char  menuItem;

		cout << " 1. Add\n"
			 << " 2. Remove\n"
			 << " 3. List\n"
			 << " 4. Exit\n"
		     << "Select ?";
		cin >> menuItem;

		switch(menuItem)
		{
		case '1' :  // Add
			{
				string firstName,surname,address,district,city,country;
				OId spouseId = 0;
                
               cin.getline(buf,256);
				cout << "First name ? ",cin.getline(buf,256),firstName = buf;
				cout << "Surname ? ",cin.getline(buf,256), surname = buf;
				cout << "Address ? ",cin.getline(buf,256), address = buf;
				cout << "District ? ",cin.getline(buf,256), district = buf;
				cout << "City ? ", cin.getline(buf,256), city = buf;
				cout << "Country ? ",cin.getline(buf,256), country = buf;
				cout << "Spouse id.(0 = none) ? ",cin >> spouseId;

				// See if there is a spouse in the file.
				Person *spouse = (Person *)addressDB->getObject(spouseId);

				// Create the object.
				Person *p = new Person(
											firstName.c_str(),
											surname.c_str(),
											address.c_str(),
											district.c_str(),
											city.c_str(),
											country.c_str(),
											0,0.0,0.0,0,
											spouse,0);
				// Add object to file.
				addressDB->attach(p);

				if(spouse)
					spouse->setSpouse(p);

				// Commit unwritten objects. This means data can never be lost.
				addressDB->commit();
			}
			break;
		case '2' :  // Remove
			{
				OId id = 0;

				cout << "Remove record id ?",cin >> id;

				Person *p = (Person *)addressDB->getObject(id,cPerson);
				if(p)
				{
					// Detach the object
					addressDB->detach(p);

					// Make sure the spouse does not point to a deleted person.
					Person *spouse = p->spouse();
					if(spouse)
						spouse->setSpouse(0);

					delete p;
				}
				else
					cout << "Record not found !\n";
			}
			break;
		case '3' :  // List
			{
				// Person iterator
				Person::It it(addressDB);
				Person *p;

				// Iterate over all Person objects.
				while(p = it++)
				{
					// Print ascii representation of the object.
					p->oPrint(cout);
				}
			}
			break;
		case '4' :  // Exit

			// Commit unwritten objects
			addressDB->commit();
			// Save file.
			addressDB->save();

			// Close the file
			delete addressDB;

			return 1;

		default:
			break;
		} // end switch
	} // end while


	} // end try
	catch(OFileErr x)
	{
		// Primitive error handling
		cout << x.why() <<'\n';
		return 0;
	}
			 
}
~~~~
 
