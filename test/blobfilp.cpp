// You should replace this string class with your own favourite string class
// before compiling.
#include "odefs.h"
#include "ofstring.h"
#include "blobfilp.h"
#include <fstream>

using namespace std;

// Declare BlobFile meta class.
OMeta BlobFile::_metaClass(cBlobFile,(Func)BlobFile::New,cOPersist,0);


BlobFile::BlobFile(const char *fname):_name(fname)
// Construct from a file.
{
	ifstream file(fname,ios::in|ios::binary);
	file.seekg(0L,ios::end);
	long size = file.tellg();
	char *datap = OBlobP::ballocator.allocate(size);

	file.seekg(0L,ios::beg);
	char ch;
	long i = 0;
	while((ch = file.get()) && !file.eof())
		datap[i++] = ch;

	// datap is now owned by the blob.
	_data.setBlob(datap,size);
}


BlobFile::BlobFile(BlobFile &from):OPersist(from), _name(from._name), _data(from._data)
// Copy constructor
{
}

void BlobFile::writeNoCR(void)
// Write to a file ommitting the CR character. Useful for converting DOS text
// files to UNIX.
{
	ofstream file(_name.c_str(),ios::out|ios::binary);

	file.seekp(0L,ios::beg);

	const char *datap = _data.const_getBlob();
	long size = _data.size();

	for(long i = 0;i < size; i++)
	{
		char c = *datap++;
		if(c != '\x0d')
			file.put(c);
	}
	// purge the blob from memory.
	_data.purge();
}


BlobFile::BlobFile(OIStream *in):OPersist(in),_data(in)
// Read constructor
{
	_name = in->readCString256();
}


void BlobFile::oWrite(OOStream *out)const
// Write persistant data to the stream. This should always be overidden
// when there is persistant data. The first method called should be the inherited
// oWrite().
{
	inherited::oWrite(out);
	_data.oWrite(out);
	out->writeCString256(_name.c_str());
}

void BlobFile::oAttach(OFile *file,bool deep)
// Attach this object to the file. That is, make the object persistant.
// This should be overridden if the object manages the persistance of
// other objects derived from OPersist.
{
	inherited::oAttach(file,deep);
	_data.oAttach(file);
}

void BlobFile::oDetach(OFile *file,bool deep)
// Detach this object from the file. That is, make the object non-persistant.
// This should be overridden if the object manages the persistance of
// other objects derived from OPersist.
{
	inherited::oDetach(file,deep);
	_data.oDetach(file);
}
