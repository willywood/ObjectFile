#ifndef OBLOBT_H
#define OBLOBT_H
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
// OBlobT is actually a template, that can be instantiated for various
// types of memory handles, and suitable allocators for them.
//
// Use:
//
// 1.  Since OBlobT holds a pointer to file containing it, it must be
// told when it is being attached and detached from the file. So always
// call the OAttach and ODetach methods in the containing classes
// overidden virtal functions of the same name.
//
// 2.  If the blob is changed by accessing the pointer returned by getBlob()
// then oSetDirty() should be called for the blob, to ensure the changes get saved.
//
// 3.  Many compilers require that you instantiate the template in one
// of the source files. If not, your compilation will succeed, but link
// will fail. This can be done as follows:
// For the following blob type
//       typedef OBlobT<char,allocator<char> > OBlob;
// write:
//       typedef OBlob dummy_OBlob;
// in one of your .cpp files.
// There are usually various other compiler switches for template instantiation,
// but this technique seems quite portable.
//
//======================================================================

#include <string.h>
#include "ostrm.h"
#include "ofile.h"


template <class T,class BlobAllocator>
class OBlobT{
friend class OFile;
public:
	typedef typename BlobAllocator::handle BlobT;

	// Default constructor
	OBlobT(void);

	OBlobT(OIStream *in);

	OBlobT(BlobT blob,oulong size);

	OBlobT(oulong size);
	// Copy constructor
	OBlobT(const OBlobT &from);

	~OBlobT(void);

	OBlobT& operator =(const OBlobT& from);

	void read(OIStream *in);

	// Use this if you are going to change the blob data.
	BlobT getBlob(void)const;
	// Use this if you are not going to change the blob data.
	const BlobT const_getBlob(void)const{return (const BlobT)getBlob();}

	void setBlob(BlobT blob,oulong size);
	void copyToBlob(const BlobT from,oulong size);
	oulong size(void)const{return _blob ? _blobLength : _fileLength;}

	void oAttach(OFile *file);
	void oDetach(OFile *file = 0);
	void oSetDirty(bool d = true){_dirty = d;}
	bool oAttached(void)const{return _file != 0;}

	void oWrite(OOStream *,const char *label = 0)const;
	bool purge(void);


	static BlobAllocator ballocator;

protected:
	oulong _blobLength;   // The current length in bytes of the blob.
private:
	OFile *_file;         // The file to which the blob is attached.
						  // _file == 0 id not currently attached to a file.
	OFilePos_t _mark;        // File position of the blobs data.
	oulong _fileLength;       // Current length in the file of the blobs data.
	BlobT _blob;          // Handle of blob in memory. _blob == 0 if not
						  // currently in memory, or no data.
	bool _dirty;
};


template <class T,class BlobAllocator>
OBlobT<T,BlobAllocator>::OBlobT(void):_blobLength(0), _file(0),_mark(0),
									  _fileLength(0),_blob(0),
									  _dirty(true)
// Default constructor
{}

template <class T,class BlobAllocator>
OBlobT<T,BlobAllocator>::OBlobT(BlobT blob,oulong size):_blobLength(size), _file(0),_mark(0),
 														_fileLength(0),_blob(blob),
 														_dirty(true)
// Existing data constructor. Takes over responsibility for managing the 
// data pointed to by blob.
{}


template <class T,class BlobAllocator>
OBlobT<T,BlobAllocator>::OBlobT(OIStream *in):_blobLength(0), _mark(0), _blob(0),_dirty(false)
											  
// Read from file constructor.
// The blob data is not actually read yet. It will be read only
// when accessed.
{
	read(in);
}

template <class T,class BlobAllocator>
void OBlobT<T,BlobAllocator>::read(OIStream *in)
// Read from file.
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

		// cast away const on
		((OBlobT<T,BlobAllocator> *)this)->
		// Create some memory. 
		_blob = ballocator.allocate(_fileLength);
		// Read from file.
		in->readBlob(ballocator.address(_blob),_mark,_fileLength);

		// cast away const
		((OBlobT<T,BlobAllocator> *)this)->
		_blobLength = _fileLength;
	}
	_dirty = false;
}


template <class T,class BlobAllocator>
OBlobT<T,BlobAllocator>::OBlobT(oulong size):_blobLength(size), _file(0),_mark(0),
											 _fileLength(0),
											 _dirty(true)
// Empty blob constructor
{
	_blob = ballocator.allocate(size);
}

template <class T,class BlobAllocator>
OBlobT<T,BlobAllocator>::OBlobT(const OBlobT<T,BlobAllocator> &from):
											  _file(0),_mark(0),
											  _fileLength(0),_dirty(true)
// Copy constructor.
{
	// get the blob into memory.
	from.const_getBlob();

	if(from._blobLength != 0)
	{
		_blob = ballocator.allocate(from._blobLength);
#ifdef WIN16
		hmemcpy(ballocator.address(getBlob()),ballocator.address(from.const_getBlob()),from._blobLength);  // huge
#else
		memcpy(ballocator.address(getBlob()),ballocator.address(from.const_getBlob()),from._blobLength);
#endif
	}
	else
		_blob = 0;

	_blobLength = from._blobLength;
}

template <class T,class BlobAllocator>
OBlobT<T,BlobAllocator>::~OBlobT(void)
{
	if(_blob)
		ballocator.deallocate(_blob);
}


template <class T,class BlobAllocator>
OBlobT<T,BlobAllocator>& OBlobT<T,BlobAllocator>::
					operator =(const OBlobT<T,BlobAllocator> &from)
// Assignment operator
{
	// Get rid of old data.
	if(_blob)
		ballocator.deallocate(_blob);

	_blob = 0;

	if(from._blobLength != 0)
	{
		_blob = ballocator.allocate(from._blobLength);
#ifdef WIN16
		hmemcpy(ballocator.address(getBlob()),ballocator.address(from.const_getBlob()),from._blobLength);  // huge
#else
		memcpy(ballocator.address(getBlob()),ballocator.address(from.const_getBlob()),from._blobLength);
#endif
	}

	_blobLength = from._blobLength;
	oSetDirty();
	return *this;
}


template <class T,class BlobAllocator>
void OBlobT<T,BlobAllocator>::oWrite(OOStream *out,const char *label)const
// Write to stream. Only the _mark and _fileLength are written to the
// current stream position. The data of the blob is written to a
// separate, specially allocated part of the file.
// We cast away the const in this method, so that it can be called from within
// OPersist::oWrite(), without any warnings.
{
	// This is the case when we are not writing to an OFile. 
	if(!out->file())
	{
		// Make sure the blob is in memory
		void *blobp = ballocator.address(((OBlobT *)this)->const_getBlob());

		out->writeBlobHeader(_mark,_blobLength);
		// Always write the blob to ensure everthing is written.
		out->writeBlob(blobp,0,_blobLength,label);
		return;
	}

	// If this is not the case you proably failed to attach it to a file.
	oFAssert(_file == out->file());

	if(_dirty && _mark && (_blobLength != _fileLength))
	{
	// This should not be entered on the second pass of commit , otherwise
	// it can corrupt the free list calculation.

		// Free the space in the file if the length has changed
		_file->freeSpace(_mark,_fileLength);
		// Cast away const
		((OBlobT<T,BlobAllocator> *)this)->_mark = 0;
		// Cast away const
		((OBlobT<T,BlobAllocator> *)this)->_fileLength = 0;
	}

	if(_mark == 0)
	{
		// getspace if we need to
		// Cast away const
		((OBlobT<T,BlobAllocator> *)this)->_mark = _file->getSpace(_blobLength);
		((OBlobT<T,BlobAllocator> *)this)->_fileLength = _blobLength;
	}

	out->writeBlobHeader(_mark,_fileLength);
	// Write the blob part only if it has changed.
	if(_dirty)
		((OBlobT<T,BlobAllocator> *)this)->_dirty = !out->writeBlob(ballocator.address(_blob),_mark,_fileLength,label);
}

template <class T,class BlobAllocator>
typename OBlobT<T,BlobAllocator>::BlobT  OBlobT<T,BlobAllocator>::getBlob(void)const
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
			// cast away const on
			((OBlobT<T,BlobAllocator> *)this)->
			// Create some memory. 
			_blob = ballocator.allocate(_fileLength);
			// Read from file.
			_file->readBlob(ballocator.address(_blob),_mark,_fileLength);
			// cast away const
			((OBlobT<T,BlobAllocator> *)this)->
			_blobLength = _fileLength;
		}
	}
	return(_blob);
}

template <class T,class BlobAllocator>
void OBlobT<T,BlobAllocator>::setBlob(BlobT blob,oulong size)
// Set the blobs data, to blob. The blob takes over responsibility for
// the data, so it must not be deleted by anyone else. Any previous data
// will be deallocated.
{
	if(_blob)
		// Deallocate previous blobs memory
		ballocator.deallocate(_blob);

	_blobLength = size;
	_blob = blob;
	oSetDirty();
}

template <class T,class BlobAllocator>
void OBlobT<T,BlobAllocator>::copyToBlob(const BlobT from,oulong size)
// Set the blobs data, to a copy of the given blob. Any previous data
// will be deallocated.
{
	// Must not copy from the same blob!
	oFAssert(_blob != from);

	// Do this first because if it throws an exception we want to leave the
	// object in a valid state.
	BlobT newBlob = ballocator.allocate(size);

	if(_blob)
		// Deallocate previous blobs memory
		ballocator.deallocate(_blob);

	_blob = newBlob;
	
#ifdef WIN16
	hmemcpy(ballocator.address(getBlob()),ballocator.address(from),size);  // huge
#else
	memcpy(ballocator.address(getBlob()),ballocator.address(from),size);
#endif

	_blobLength = size;
	oSetDirty();
}


template <class T,class BlobAllocator>
void OBlobT<T,BlobAllocator>::oAttach(OFile *file)
// Attach to a file.
{
	oFAssert(!_file);

	// Attach to new file
	_file = file;

	oSetDirty();
}

template <class T,class BlobAllocator>
void OBlobT<T,BlobAllocator>::oDetach(OFile * /* file = 0 */)
// Detach from a file. Can be called directly. Parameter not relevant.
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


template <class T,class BlobAllocator>
bool OBlobT<T,BlobAllocator>::purge(void)
// Purge the blob from memory. It is the containing objects responsibility to
// determine whether the blob can be purged, and to call this method.
// Purge can be called by the blob allocator. This means all the blob
// methods should set _blob to zero after deallocating to ensure memory
// is not freed twice.
// Return true if blob was purged.
{
	// Should not be trying to purge an object if it is not in the file.
	oFAssert(_file);
	if(!_dirty)
	{
		if(_blob){
			// Deallocate previous blobs memory
			ballocator.deallocate(_blob);
			_blob = 0;
			_blobLength = 0;
		}
		return true;
	}else
		return false;
}

// Define the static allocator.
template <class T,class BlobAllocator>
BlobAllocator OBlobT<T,BlobAllocator>::ballocator;

#endif
