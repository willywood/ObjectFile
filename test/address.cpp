//
// A small address database to demonstrate basic use of ObjectFile.
//

#include "odefs.h"
#include "ofstring.h"
#include <fstream>
#include "oufile.h"
#include "ox.h"
#include "person.h"


using namespace std;

int main(){

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
				ofile_string firstName,surname,address,district,city,country;
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

				// Commit unwritten records
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
					// Detach the record
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
				while((p = it++))
				{
					// Print ascii representation of the object.
					p->oPrint(cout);
				}
			}
			break;
		case '4' :  // Exit

			// Commit unwritten records
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
	return 0;		 
}
