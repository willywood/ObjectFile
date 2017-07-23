#ifndef OBlobP_H
#define OBlobP_H
//======================================================================
// 	This software may only be compiled in conjunction with a licence
// agreement with ObjectFile Limited.
// Copyright(c) 1996-99 ObjectFile Ltd. 
//======================================================================


//======================================================================
// OBlob : BLOB stands for binary large object. It is useful for storing
// large blocks of binary data.
// 
// OBlobP is used for blobs that can be represented as character
// buffers.
// It is useful for systems that cannot compile the template version
// OBlobT.
//
// Use:
//
// 1.  Since OBlobP holds a pointer to file containing it, it must be
// told when it is being attached and detached from the file. So always
// call the OAttach and ODetach methods in the containing classes
// overidden virtal functions of the same name.
//
// 2.  If the blob is changed by accessing the pointer returned by getBlob()
// then oSetDirty() should be called for the blob, to ensure the changes get saved.
//
//
//======================================================================

#include <stddef.h>
#include "ofmemreg.h"
class OIStream;
class OOStream;
class OFile;

class OBlobP{
public:

	// Default constructor
	OBlobP(void);

	OBlobP(OIStream *in);

	OBlobP(char* blob,size_t size);

	OBlobP(size_t size);
	// Copy constructor
	OBlobP(const OBlobP &from);

	~OBlobP(void);

	OBlobP& operator =(const OBlobP& from);

	void read(OIStream *in);

	// Use this if you are going to change the blob data.
	char * getBlob(void)const;
	// Use this if you are not going to change the blob data.
	const char *const_getBlob(void)const{return (const char *)getBlob();}

	void setBlob(char * blob,size_t size);
	void copyToBlob(const char *from,oulong size);
	size_t size(void)const;

	void oAttach(OFile *file);
	void oDetach(OFile *file);
	void oSetDirty(bool d = true){_dirty = d;}

	void oWrite(OOStream *,const char *label = 0)const;
	bool purge(void);

	// Total memory usage of all instances.
	static unsigned long currentTotalMemoryUsage(void){return _cache.size();}

	class BlobAllocator
	{
	public:
		char *allocate(size_t length){return new char[length];}
	};
	static BlobAllocator ballocator;

protected:
	oulong _blobLength;   // The current length in bytes of the blob.
private:
	OFile *_file;         // The file to which the blob is attached.
						  // _file == 0 id not currently attached to a file.
	OFilePos_t _mark;        // File position of the blobs data.
	oulong _fileLength;   // Current length in the file of the blobs data.
	char * _blob;         // Handle of blob in memory. _blob == 0 if not
						  // currently in memory, or no data.
	bool _dirty;

	static OFMemoryRegister _cache; // Keeps track of memory usage.
};


#endif
