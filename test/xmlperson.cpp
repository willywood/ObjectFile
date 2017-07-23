#include "oisxml.h"

int main(int argc, char* argv[])
{
fstream inFile;
inFile.open("settings.xml",ios::in|ios::binary | ios::nocreate,filebuf::sh_read);
OXMLReader reader;
OIStreamXML oisxml;

reader.setContentHandler(&oisxml);
if(inFile.is_open() && !inFile.fail())
	reader.parse(&inFile);
return 0;
}