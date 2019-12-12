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
//
// OPersist is the class from which all persistant objects must be
// derived. It does not contain any persistant data itself, but defines
// a methodology for persistant classes derived from it.
// The new and delete operators are overloaded to manage an object
// cache.
//
// 
// If OF_REF_COUNT is defined then a reference count is used to
// determine when to set the object purgeable.
// This is useful in a multi-threaded environment.
//
// 

#include "odefs.h"
#include "opersist.h"
#include "ostrm.h"
#include "ofile.h"

// Meta class instance.
OMeta OPersist::_metaClass(cOPersist,(Func)OPersist::New,OClassId_t(0));

// The objectCacheCount is global, so it must be protected by a 
// global mutex.
static OFMutex sMutex;

void *OPersist::operator new(size_t size)
// new for all OPersist objects.
// In addition to allocating regular memory, a object from
// the object cache is allocated. If the cache is full OFile::new_handler
// is called. This can be used to free objects from the cache.
{
	// Do not enter in more than one thread.
    OFGuard guard(sMutex);

	while(OFile::_sObjectCacheCount >= OFile::_sObjectThreshold)
	{
		// Object threshold has been reached.
		if(OFile::_sNew_handler)
			(*OFile::_sNew_handler)();
		else
		{
		    // FIXME We should not return null.
			return 0;
		}
	}

	void *ret = ::operator new(size);
	OFile::_sObjectCacheCount++;
	return ret;
}

void OPersist::operator delete(void *ob)
// delete for all OPersist objects.
{
	// Do not enter in more than one thread.
	OFGuard guard(sMutex);

	OFile::_sObjectCacheCount--;
	::operator delete(ob);
	return;
}



OPersist::OPersist(OIStream *s):_oId(0)
// ObjectFile constructor. This constructor must read in any persistant
// data, from the stream.
{
	// Object is born not purgeable and not dirty.
	_npFlags.inFile = _npFlags.dirty = _npFlags.purgeable = 0;

// OF_REF_COUNT is defined in odefs.h
#ifdef OF_REF_COUNT
	// Object is born with a reference.
	_npFlags.refCnt = 1; 
#endif
	// Set the index of this object in the file to point to this
	// (partially formed) object. This is so that if while constructing
	// the object, there is a reference back to this object, it can
	// be resolved.
	s->setCurrentObject(this);
}

OPersist::OPersist(void):_oId(0)
// Default constructor
{
	// New object that is not in the file must be dirty.
	_npFlags.dirty = 1;
	// Object is born not purgeable and not in file
	_npFlags.inFile = _npFlags.purgeable = 0;

// OF_REF_COUNT is defined in odefs.h
#ifdef OF_REF_COUNT
	// Object is born with a reference.
	_npFlags.refCnt = 1; 
#endif
}

OPersist::OPersist(const OPersist &):_oId(0)
// Copy constructor
{
	// New object that is not in the file must be dirty.
	_npFlags.dirty = 1;
	// Object is born not purgeable and not in file
	_npFlags.inFile = _npFlags.purgeable = 0;
}


OPersist::~OPersist(void)
{
	// It is forbidden to directly delete an object that is in the file, unless of
	// course we are in the process of closing the file.
	oFAssert(OFile::_sPermitObjectDestruction || !_npFlags.inFile);
}

OId OPersist::oId(void)const
{
	// The identity is only relavent if the object is attached to a file.
	// Use oAttached to detect this, and not oId == 0
	oFAssert(oAttached());

	return _oId;
}


void OPersist::oWrite(OOStream *)const
// Write persistant data to the stream. This should always be overidden
// when there is persistant data. The first method called should be the inherited
// oWrite().
{
}

void OPersist::oAttach(OFile *file,bool /* deep */)
// Attach this object to the file. That is, make the object persistant.
// This should be overridden if the object manages the persistance of
// other objects derived from OPersist.
{
	// attach this object to the file - shallow
	file->pInsert(this);
}
void OPersist::oDetach(OFile *file,bool /* deep */)
// Detach this object from the file. That is, make the object non-persistant.
// This should be overridden if the object manages the persistance of
// other objects derived from OPersist.
{
	// detach this object from file - shallow
	file->pErase(this);
	
	// Set the identity to 0 because it is no longer relevant when the
	// object is not in a file.
	// This is a safety feature to prevent problems when objects are
	// detached from one file and then attached to another. The second
	// file may already have such an id or its _uniqueid may not yet
	// have reached the new objects id. If you are worried about eating
	// ids when continuously detaching and attaching objects to the same
	// file you can remove this statement. In that case the object will
	// retain its id.
	// This functionality has now been replaced by OFile::setRetainIdentity()
//	_oId = 0;
}

void OPersist::oSetPurgeable(bool /* deep */,OFile * /* file */)
// Set this object as purgeable. There should be no references to the object
// as it may be deleted. Derived classes may want to override this function
// in order to pass it on to owned objects.
// Derived classes should call the super class last.
// Parameters: deep - true if owned objects are to be set purgeable
//                    (default true)
//             file - the file containing the object.0 means evaluate it.
//                    (default 0) This is only important when using OnDemand.
{
	pSetPurgeable(true);
}



void OPersist::setId(OId id)
// Set the identity of an object while it is detached from the OFile.
// This is very dangerous if you do not know what you are doing. Normally
// you should leave it to ObjectFile to assign Object id's.
{
    // We should not try to set the id of an attached object as this will
	// cause an inconsistency with the index
//	oFAssert(!oAttached());

    _oId = id;
}


// OF_REF_COUNT is defined in odefs.h
#ifdef OF_REF_COUNT
void OPersist::addRef(void)const
// Add a reference and make the object not purgeable.
{
	// cast away const
	((OPersist *)this)->pSetPurgeable(false);
}
void OPersist::removeRef(void)const
// Remove a reference and make the object purgeable if the reference
// count reaches 0.
{
	// cast away const
	((OPersist *)this)->pSetPurgeable(true);
}


void OPersist::pSetPurgeable(bool purgeable)
// private
// Called by OFile::getObject()
// In the multi-threaded model, a reference count is kept. This
// is to prevent purging an object that is being accessed by two or more
// threads.
// If two threads request that an object not be purgeable, then the
// reference count will be 2. Only when they have both set it 
// purgeable will the reference count fall to zero. The object will
// then really be set purgeable.
// To use it sucessfully there must be exactly one call to oSetPurgeable
// for every call to getObject .
// Parameter: purgeable - true if the object can be purged. false
// if not.
{
	if(purgeable)
	{
		// Check that there are not too many purges.
		// Only when closing the file do we permit an extra oSetPurgeable.
		oFAssert(OFile::_sPermitObjectDestruction || _npFlags.refCnt > 0);

		if(--_npFlags.refCnt == 0)
			_npFlags.purgeable = 1;
	}
	else
	{
		_npFlags.refCnt++;
		_npFlags.purgeable = 0;
	}
}

#else
void OPersist::pSetPurgeable(bool purgeable)
// Set the object purgeable.
// Called by OFile::getObject()
// In the single threaded model OSetPurgeable can be called as many
// times as desired for an object, without any adverse consequenses.
// Parameter: purgeable - true if the object can be purged. false
// if not.
{
	_npFlags.purgeable = (purgeable ? 1 : 0);
}
#endif
