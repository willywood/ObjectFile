#ifndef OBlobP_H
#define OBlobP_H
/*=============================================================================
MIT License

Copyright(c) 2019 willywood

Permission is hereby granted, free of charge, to any person obtaining a copy
of this softwareand associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright noticeand this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
=============================================================================*/



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
	static OFilePos_t currentTotalMemoryUsage(void){return _cache.size();}

	class BlobAllocator
	{
	public:
		char *allocate(size_t length){return new char[length];}
	};
	static BlobAllocator ballocator;

protected:
	oulong _blobLength;   // The current length in bytes of the blob.
private:
	// Default constructor made private so that we do not forget to initialize it.
	OBlobP(void);

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
