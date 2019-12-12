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
// OBlobP is used for blobs that can be represeted as character
// buffers.
// It is useful for systems that cannot compile the template version
// OBlobT
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
//======================================================================


#include "odefs.h"
#include <string.h>
#include "oblobp.h"
#include "ostrm.h"
#include "ofile.h"


OBlobP::BlobAllocator OBlobP::ballocator;
OFMemoryRegister OBlobP::_cache;

OBlobP::OBlobP(void):_blobLength(0),_file(0),_mark(0),
					 _fileLength(0),
					 _blob(0),_dirty(true)
// Default constructor
{}

OBlobP::OBlobP(char * blob, size_t size):_blobLength(size),_file(0),_mark(0),
										_fileLength(0),_blob(blob),
										_dirty(true)
// Existing data constructor. Takes over responsibility for managing the 
// data pointed to by blob.
{
	// Add to global cache
	_cache.add(_blobLength);
}


OBlobP::OBlobP(OIStream *in):_blobLength(0),_mark(0),_blob(0),_dirty(false)
// Read from file constructor.
// The blob data is not actually read yet. It will be read only
// when accessed.
{
	read(in);
}

void OBlobP::read(OIStream *in)
// Read the blob from the file.
// This should only be used on an empty, unattached blob. It is useful
// for creating the blob in two phases.
// The blob data is not actually read yet. It will be read only
// when accessed.
{
	// Must be an empty blob
	oFAssert(!_blob && !_mark && !_blobLength);

	_file = in->readBlobHeader(&_mark,&_blobLength,&_fileLength);

	// Check if blob is attached to an OFile. 
	if(!_file)
	{
		// Read the blob into memory.

		((OBlobP*) this)->_blob = new char[_fileLength];
		// Read from file.
		in->readBlob(_blob,_mark,_fileLength);

		((OBlobP *)this)->_blobLength = _fileLength;

		// Add to global cache
		_cache.add(_blobLength);
	}
	_dirty = false;
}

OBlobP::OBlobP(size_t size):_blobLength(size),_file(0),_mark(0),
							_fileLength(0),
							_dirty(true)
// Empty blob constructor
{
	_blob = new char[size];
	// Add to global cache
	_cache.add(_blobLength);
}

OBlobP::OBlobP(const OBlobP &from):_file(0),_mark(0),
								   _fileLength(0),_dirty(true)
// Copy constructor.
{
	// get the blob into memory.
	from.const_getBlob();

	if(from._blobLength != 0)
	{
		_blob = new char[from._blobLength];
		memcpy(getBlob(),from.const_getBlob(),from._blobLength);
	}
	else
		_blob = 0;

	_blobLength = from._blobLength;

	// Add to global cache
	_cache.add(_blobLength);
}

OBlobP::~OBlobP(void)
// Destructor
{
	delete []_blob;

	// Subtract from global cache
	_cache.subtract(_blobLength);
}


OBlobP& OBlobP::operator =(const OBlobP &from)
// Assignment operator
{
	// Get rid of old data.
	delete []_blob;

	// Subtract from global cache
	_cache.subtract(_blobLength);

	// get the blob into memory.
	from.const_getBlob();

	if(from._blobLength != 0)
	{
		_blob = new char[from._blobLength];
		memcpy(getBlob(),from.const_getBlob(),from._blobLength);
	}
	else
		_blob = 0;

	_blobLength = from._blobLength;
	// Add to global cache
	_cache.add(_blobLength);

    _dirty = true;

	return *this;
}


void OBlobP::oWrite(OOStream *out,const char *label)const
// Write to stream. Only the _mark and _length are written to the
// current stream position. The data of the blob is written to a
// separate, specially allocated part of the file.
// We cast away the const in this method, so that it can be called from within
// OPersist::oWrite(), without any warnings.
{
	// This is the case when we are not writing to an OFile. 
	if(!out->file())
	{
		// Make sure the blob is in memory
		char *blobp = ((OBlobP *)this)->getBlob();

		out->writeBlobHeader(_mark,_blobLength);
		// Always write the blob to ensure everthing is written.
		// Blob is written uncompressed
		out->writeBlob(blobp,0,_blobLength,label);
		return;
	}

	// If this is not the case you probably failed to attach it to a file.
	oFAssert(_file == out->file());

	if(_dirty && _mark && (_blobLength != _fileLength))
	{
	// This should not be entered on the second pass of commit , otherwise
	// it can corrupt the free list calculation.

		// Free the space in the file if the length has changed
		_file->freeSpace(_mark,_fileLength);
		// Cast away const
		((OBlobP *)this)->_mark = 0;
		// Cast away const
		((OBlobP *)this)->_fileLength = 0;
	}

	if(_mark == 0)
	{
		// getspace if we need to
		// Cast away const
		((OBlobP *)this)->_mark = _file->getSpace(_blobLength);
		((OBlobP *)this)->_fileLength = _blobLength;
	}

	out->writeBlobHeader(_mark,_fileLength);

	// Write the blob part only if it has changed.
	if(_dirty)
		// Set dirty false if data was actually written
		((OBlobP *)this)->_dirty = !out->writeBlob(_blob,_mark,_fileLength,label);

}

char * OBlobP::getBlob(void)const
// Get the blob data. This returns a handle to the data INSIDE the blob.
// This must be handled responsibly. If you directly change the data
// you must call oSetDirty().
// getBlob() actually allocates memory and reads the data from the file.
// Returns adress of blob data or 0 if there is none.
{
	if(!_blob && _file)
	{
		// Multiple processes on the same file should not enter at the same time.
		OFGuard(_file->mutex());

		if(_mark && _fileLength)
		{
			// Blob is not in memory
			// Create some memory.
			((OBlobP*) this)->_blob = new char[_fileLength];
			// Read from file.
			_file->readBlob(_blob,_mark,_fileLength);

			((OBlobP *)this)->_blobLength = _fileLength;

			// Add to global cache
			_cache.add(_blobLength);
		}
	}
	return(_blob);
}

void OBlobP::setBlob(char * blob,size_t size)
// Set the blobs data, to blob. The blob takes over responsibility for
// the data, so it must not be deleted by anyone else. Any previous data
// will be deallocated.
{
	// Deallocate previous blobs memory
	delete []_blob;

	// Subtract from global cache
	_cache.subtract(_blobLength);

	_blobLength = size;
	_blob = blob;

	// Add to global cache
	_cache.add(_blobLength);

	oSetDirty();
}

void OBlobP::copyToBlob(const char *from,oulong size)
// Set the blobs data, to a copy of the given blob. Any previous data
// will be deallocated.
{
	// Must not copy from the same blob!
	oFAssert(_blob != from);

	// Do this first because if it throws an exception we want to leave the
	// object in a valid state.
	char *newBlob = new char[size];

	if(_blob)
	{
		// Deallocate previous blobs memory
		delete []_blob;

		// Subtract from global cache
		_cache.subtract(_blobLength);
	}
	
	_blob = newBlob;;
	memcpy(getBlob(),from,size);

	_blobLength = size;
	
	// Add to global cache
	_cache.add(_blobLength);
	
	oSetDirty();
}


void OBlobP::oAttach(OFile *file)
// Attach to a file.
{
	oFAssert(!_file);

	// Attach to new file
	_file = file;

	oSetDirty();
}

void OBlobP::oDetach(OFile *)
// Detach from a file.
{
	oFAssert(_file);

	// get blob into memory
	getBlob();

	if(_mark){
		// Free the space in the file.
		_file->freeSpace(_mark,_fileLength);
		_mark = 0;
		_fileLength = 0;
	}

	_file = 0;
}


bool OBlobP::purge(void)
// Purge the blob from memory. It is the containing objects responsibility to
// determine whether the blob can be purged, and to call this method.
// Return true if blob was purged.
{
	// Should not be trying to purge an object if it is not in the file.
	oFAssert(_file);
	if(!_dirty)
	{
		// Deallocate previous blobs memory
		delete []_blob;

		// Subtract from global cache
		_cache.subtract(_blobLength);

		_blob = 0;
		_blobLength = 0;
		return true;
	}else
		return false;
}


size_t OBlobP::size(void)const
// Return the size of the blob data.
{
	if(_blob)
		return _blobLength;
	else
		return _fileLength;
}

