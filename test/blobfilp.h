#ifndef BlobFile_H
#define BlobFile_H

//===================================================================
//	BlobFile describes a file.
//
//===================================================================
#include "opersist.h"
#include "oiter.h"
#include "oblobp.h"
#include "ofstring.h"



// Best put in a header file along with all other class id's.
const OClassId_t cBlobFile = 10;

class BlobFile : public OPersist
{
typedef OPersist inherited;
public:
	// Read constructor
	BlobFile(OIStream *c);
	// Other constructors
	BlobFile(const char *fname);
	BlobFile(BlobFile &);

	~BlobFile(void){}

	BlobFile *clone(void)const;
	void writeNoCR(void);

	void oAttach(OFile *,bool);
	void oDetach(OFile *,bool);

	OMeta *meta(void)const{return &_metaClass;}
	// Define iterator for this class.
	typedef OIteratorT<BlobFile,cBlobFile> It;

	long size(void)const{ return _data.size();}
	const char *name(void)const{return _name.c_str();}
protected:
	void oWrite(OOStream *)const;
private:
	// ObjectFile Instantiation function
	static OPersist *New(OIStream *s){return new BlobFile(s);}

	static OMeta _metaClass;

	// Persistant data
	ofile_string _name;		// File name
	OBlobP _data;	// File contents

};

#endif
