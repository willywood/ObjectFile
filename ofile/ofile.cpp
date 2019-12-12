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

// OFile is responsible for overall management of the file. It provides
// a set of services that allow its users to attach and detach objects from
// it. These objects must be derived from OPersist.
// 	Multiple file objects may be opened, to manipulate multiple files. An
// object may be connected to only one file at a time. 
//
// Only the most basic and essential file management services are provided.
// This helps keep OFile lean and mean. Other services can be added by
// sub-classing OFile. OUFile is an example subclass.
//
// The fastFind option causes finding by id, on a deep inheritence hierarchy
// to work much faster. It builds an extra index _oList( which of course uses
// memory). The biggest effect of fastFind can be on loading time of a file,
// particularly when there are many object references to be resolved. After
// loading it can be switched off. This releases any memory, used by it.
//
// The fileLength is stored in the header in case there is an end of file
// mark in the file.
//

#include "odefs.h"
#include "ofile.h"
#include "ometa.h"
#include "opersist.h"
#include "ox.h"
#include "ofmemreg.h"
#include <string.h>

// Used to determine the word format of the current processor.
unsigned long OFile::_sProcessorId = 0x01020408;

// Used to determine what coversions must be made to support changes
// in ObjectFile data structurs. This should not interest the application
// developer.
long OFile::_sOFileSourceVersion = 2;

// Set the version number of the application program source files.
// This should be incremented whenever an object schema change is made.
// It can then be compared to the application files userVersion() in
// order to determine what conversions have to be made.
long OFile::_sUserSourceVersion = 1;

// Used to check that an application is not explicitly deleting an object that
// is attached to a file.
bool OFile::_sPermitObjectDestruction = false;

OFile *OFile::_sFileListHead = 0;

// Used to assign a unique file identity to each instace of OFile.
// Identity starts from 1.
int OFile::_sUniqueFileId = 1;

// Maximum number of objects in memory.
long OFile::_sObjectThreshold = LONG_MAX;
// Current number of objects in memory.
long OFile::_sObjectCacheCount = 0;

OFMutex OFile::_sMutex; // Global mutex

// handler for when the object threshold is exceeded.
OFile::New_handler OFile::_sNew_handler = OFile::new_handler;

OFile::New_handler OFile::set_new_handler(OFile::New_handler newNewHandler)
// Static
// Set a user defined new_handler for when the object threshold is
// reached.
// Return value - previous new_handler.
// Note this has nothing to do with the new_handler for memory except
// that it works in a similar way.
{
  	New_handler oldHandler = _sNew_handler;
    _sNew_handler = newNewHandler;
    return oldHandler;
}

void OFile::new_handler()
// Default new_handler for OPersist objects. Activated when the object
// threshold is reached.
// Note this has nothing to do with the new_handler for memory except
// that it works in a similar way.
// The strategy can be redefined at any time by calling OFile::set_new_handler.
// Exceptions: OFileThresholdErr is thrown if after purging there is no
// space left.
{
	// The strategy implemeted in purgeAll() is to purge all objects
	// in memory. This is good when sequentially writing a large file.
	// There are other possible(and maybe better) strategies.
	// for example purge 10% of the object threshold. This would
	// be good when randomly accessing a large file.
	purgeAll();	

	// If we did not succede then throw an exception.
	if(_sObjectCacheCount >= _sObjectThreshold)
		throw OFileThresholdErr("Object threshold exceeded");

	return;
}


void OFile::close(void)
// Physically close the file.
{
	_in.close();
}

void OFile::reopen(const char *fname)
// Physically reopen the current file after it has been closed. 
// This can be dangerous if the file has been changed since it was closed.
{
	// Convert create to open for writing.
	long operation  = _operation;
	if(operation & OFILE_CREATE)
		operation = (operation & ~OFILE_CREATE) | OFILE_OPEN_FOR_WRITING;

	_in.open(fname,operation);
}

void OFile::reopen(void)
// Physically reopen the current file after it has been closed. 
// This can be dangerous if the file has been changed since it was closed.
// This method should be overridden in a sub-class if it is to be used.
// The sub-class should then call reopen(filename)
{
	// No default implementation because we do not know the file name.
	oFAssert(0);
}

OFile::OFile(const char *fname,long operation,const char *magicNumber):
											_oList(0),
											_fList(this),
											_in(this,fname,operation),
											_operation(operation)

// Parameters: fname - file name.
//             operation - file open flags:
// OFILE_CREATE    -  Create a new file. This will overwrite an existing file of the
// same name.
// If the file cannot be created an exception will be thrown.
// OFILE_OPEN_FOR_WRITING - Open an existing file for writing. If the file does not
// exist an exception will be thrown.
// If or'ed with OFILE_CREATE and the file exists, the existing file will be opened.
// OFILE_OPEN_READ_ONLY - Open an existing file readonly. If the file does not exist
// an exception will be thrown.
//
// The following flag may be ore'ed together with one of the previous flags.
// OFILE_FAST_FIND - An extra index will be built, that will allow objects, of 
// unspecified classes, to be found much more quickly than otherwise. 
// This extra speed is particularly useful when many objects have to be connected 
// in a file that is being loaded. It can improve file loading speed enourmously. 
// The index requires extra memory, but this can be released by calling fastFindOff() 
// after the objects have been connected, or at any other time.
// 			   magicNumber - a four byte character string identifying the magic 
// number of the file.(default = 0)
// This is used in many systems to distinguish between different types of files. It
// is also a way of ensuring you are reading the right type of file. If it is 0
// then no magicNumber is written or checked. If it is used and there is no match
// an exception is thrown.
{
	init(fname,magicNumber);
}

#ifdef OF_OLE
OFile::OFile(IStorage *istorage,
			 const char *fname,long operation,
			 unsigned long istorage_mode,
			 const char *magicNumber):
										_fList(this),
										_oList(0),
										_in(this,istorage,fname,istorage_mode),
										_operation(operation)

// Parameters: istorage - an open pen IStorage.
//             fname  - stream name in the istorage.
//             operation - file open flags:
//
// OFILE_FAST_FIND - An extra index will be built, that will allow objects, of 
// unspecified classes, to be found much more quickly than otherwise. 
// This extra speed is particularly useful when many objects have to be connected 
// in a file that is being loaded. It can improve file loading speed enourmously. 
// The index requires extra memory, but this can be released by calling fastFindOff() 
// after the objects have been connected, or at any other time.
//             istorage_mode : STG_ flags. These take precedence over access flags of
// operation.
// 			   magicNumber - a four byte character string identifying the magic 
// number of the file.(default = 0)
// This is used in many systems to distinguish between different types of files. It
// is also a way of ensuring you are reading the right type of file. If it is 0
// then no magicNumber is written or checked. If it is used and there is no match
// an exception is thrown.
{
	init(fname,magicNumber);
}
#endif

void OFile::init(const char *fname,const char *magicNumber)
// Initialize OFile.
// fname - name of storage.
// magicNumber - a four byte character string identifying the magic number of the file.(default = 0)
// This is used in many systems to distinguish between different types of files. It
// is also a way of ensuring you are reading the right type of file. If it is 0
// then no magicNumber is written or checked. If it is used and there is no match
// an exception is thrown.
{
	// Must specify a file name
	oFAssert(fname);

	_oFileMark = 0;
	_oFileLength = 0;
	_oFileVersion = _sOFileSourceVersion;
	_fileProcessorId = _sProcessorId;
	_userVersion = userSourceVersion();
	_dirty = false;
	_next = 0;
	_rootId = 0;
	_autoCommit = false;
	_retainIdentity = false;

	if(OFILE_FAST_FIND & _operation)
		_oList = new ObjectList;

	try{

	// Do not enter in more than one thread.
    OFGuard guard(_mutex);

	// Initialize the meta classes.
	OMeta::initialize();

	// Get the current file length
	_fileLength = _in.fileLength();

	// If the stream(_in) is empty, assume it has just been created.
	if((OFILE_CREATE & _operation) && (_fileLength == 0))
	{
		// New file.
		_fileLength = cHeaderLength;
		_uniqueId = 1;
		_dirty = true;

		_fileProcessorId = _sProcessorId;

		// Initialize the magic number
		for(int i = 0; i < 4 ;i++)
			// If none is specified initialize to all 0's.
			_magicNumber[i] = magicNumber ? magicNumber[i] : (char)0;
	}
	else
	{
		// Readonly or read/write

		// Read file header.
		_in.start(0,sizeof(_fileProcessorId) +
					sizeof(_oFileVersion) +
					sizeof(_userVersion) +
					sizeof(_oFileMark) +
					sizeof(_oFileLength) +
					sizeof(_rootId) +
					sizeof(_fileLength) +
					sizeof(_magicNumber));

		_in.readBytes(&_fileProcessorId,4);

		_oFileVersion = _in.readLong();

		// Check that this is a valid OFile
		if(_oFileVersion > _sOFileSourceVersion || _oFileVersion < 1)
			throw OFileErr("Invalid file format.");

		_userVersion = _in.readLong();
		_oFileMark = _in.readFilePos();
		_oFileLength = _in.readLong();
		_rootId = _in.readObjectId();

		// From version 1.4 the disk file length is stored.
		// _oFileSourceVersion 1 -> 2
		OFilePos_t flen = _in.readFilePos();
		if(flen)
			_fileLength = flen;

		_in.readBytes(_magicNumber,4);

		// Check that this is a valid OFile
		if(magicNumber && (0 != memcmp(_magicNumber,magicNumber,4)))
			throw OFileIOErr("Invalid file format.");

		// Finish reading the header
		_in.finish();

		// Start reading the 'OFile' object.
		_in.start(_oFileMark,_oFileLength);
		_uniqueId = _in.readLong();

		OClassId_t cId;
		while((cId = _in.readShort()) != 0)
		// Read ObjectList and build classList
		{
			long objectCount = _in.readLong();
#ifdef OF_HASH
			_cList.classList(cId).resize(objectCount);
#endif
			for(long i = 0;i < objectCount;i++)
			{
				OId id = _in.readObjectId();
				OFilePos_t mark = _in.readFilePos();
				oulong length = _in.readLong();

				// insert into Object list
				// pair<ClassList::iterator,bool> ret = 
				_cList.classListCr(cId).insert(ClassList::value_type(id,OEnt(mark,length)));
			}
		}
		if(OFILE_FAST_FIND & _operation){
			// Build object list from the class list.
			// This could have been done in the previous loop, but it is better to
			// have the object list in contiguous memory, since it may be go rid of,
			// and we do not want to leave fragmented memory lying around.

			// Get the set of all the classes to be iterated over
			const OMeta::Classes &classes = OMeta::meta(cOPersist)->classes();

			OMeta::Classes::const_iterator begin = classes.begin();
			OMeta::Classes::const_iterator end = classes.end();

			for(OMeta::Classes::const_iterator cSetIt = begin;cSetIt != end;++cSetIt)
			{
				ClassList::const_iterator cBegin = _cList.classList(*cSetIt).begin();
				ClassList::const_iterator cEnd = _cList.classList(*cSetIt).end();

				for(ClassList::const_iterator cListIt = cBegin;
					 cListIt != cEnd;
					 ++cListIt)
					 // Insert in the object list a pointer to the matching element
					 // in the class list.
					 _oList->insert(ObjectList::value_type((*cListIt).first,*cSetIt));
			}
		}
		// Read free list.
		_fList.read(&_in);
		// Finish reading the 'OFile' object.
		_in.finish();
	}

	// Global mutex
    OFGuard sguard(_sMutex);

	// Add this file to the list of files.
	OFile *tail = getTail();
	if(tail)
		tail->_next = this;
	else
		_sFileListHead = this;

	// Assign a unique file identity.
	_fileId = _sUniqueFileId++;

	// This try block is just for _oList. I would use auto_ptr if it
	// was in the same header file in each compiler.
	}catch(...){
		delete _oList;
		throw;
	}

}

OFile::~OFile(void)
// Destructor
{
	// Global mutex
    OFGuard sguard(_sMutex);

	// Clears objects from memory and from the indexes.
	pClear();

	delete _oList;

	// Remove this file from the list of files
	OFile *f = _sFileListHead;
	if(f == this)
		// Remove file by removing head
		_sFileListHead = f->_next;
	else
	{
		while(f)
		{
			if(f->_next == this)
			{
				// Remove file by linking to files next
				f->_next = f->_next->_next;
				break;
			}
			f = f->_next;
		}
	}

}


void OFile::pClear(void)
// Clears objects from memory and from the indexes.
// This should be protected by a global mutex.
{
	// _sPermitObjectDestruction is used by the OPersist destructor and the
	// oSetPurgeable method to verify that
	// the programmer does not try to delete objects that are persistant.
	bool save_permitObjectDestruction = _sPermitObjectDestruction;
	_sPermitObjectDestruction = true;

	// Although it is unlikely that an exception will be thrown while
	// deleting the files objects, it is possible. We therefore make sure
	// that we leave the still open file in as valid a state as possible.
	// However if objects were not committed, before deleting the file,
	// then it is possible that there data will be lost.
	//
	try
	{
		OClassId_t cId;
		// Set all objects purgeable. This clears any links to
		// persistent object
		for(cId = 1; cId <= cOMaxClasses; cId++)
		{
			for(ClassList::iterator it = _cList.classList(cId).begin(); 
				it != _cList.classList(cId).end();++it)
			{
				// Set purgeable only if in memory, but even if already purgeable
				OPersist *ob = (*it).second._ob;
				if(ob)
					ob->oSetPurgeable(true,this);
			}
		}
		// Delete all objects from memory.
		for(cId = 1; cId <= cOMaxClasses; cId++)
		{
			for(ClassList::iterator it = _cList.classList(cId).begin(); 
				it != _cList.classList(cId).end();++it)
			{
				// Delete only if in memory;
				OPersist *ob = (*it).second._ob;
				if(ob)
				{
					delete ob;
					// Clean up the pointer just in case an exception is thrown.
					(*it).second._ob = 0;
				}

			}
			// Empty the class list
//			_cList.classList(cId).clear();
			_cList.classList(cId).erase(_cList.classList(cId).begin(),_cList.classList(cId).end());
		}
	}catch(...){
		_sPermitObjectDestruction = save_permitObjectDestruction;
		throw;
	}

	_sPermitObjectDestruction = save_permitObjectDestruction;

	// Clear the object list if there is one.
	if(_oList)
//		_oList->clear();
		_oList->erase(_oList->begin(),_oList->end());
}

void OFile::reset(void)
// Reset the file to be completely empty. Free all space in the file.
{
	// Global mutex
    OFGuard sguard(_sMutex);

	_rootId = 0;

	// Clears objects from memory and from the indexes.
	pClear();

	// Clear the free list
	_fList.clear();

	// Clear the file object
	_oFileMark = 0; 
	_oFileLength = 0;
	
	_fileLength = cHeaderLength;
	_uniqueId = 1;
	_dirty = true;
}


OFile *OFile::oFileOf(OPersist *ob)
// Static
// Return the file in which the given object exists.
// Undefined if object is not in any file.
// Parameter: ob - object whose file is to be returned.
// If a single file only is to be used, this function can immediatly
// return _fileListHead without looking up the object.
{
	oFAssert(ob && ob->oAttached());

	// Global mutex
    OFGuard sguard(_sMutex);

	OFile *f = _sFileListHead;

	// Look for object by id in every file, until found.
	while(f)
	{
		pair<ClassList::iterator,OClassId_t>ret =
			f->_cList.find(ob->oId(),ob->meta()->id());
		// It must be de-referenced already and have a
		// matching memory address.
		if(ret.second && ob == (*(ret.first)).second._ob)
			return f;
		f = f->_next;
	}

	// Must be in some file
	oFAssert(0);

   return 0;
}

void OFile::setRoot(OPersist *root)
// Set the root object.
// Parameter root - A pointer to a persistent object or 0 to clear the root.
{
	if(root)
	{
		// root must be attached
		oFAssert(root->oAttached());

		_rootId = root->oId();
	}
	else
		_rootId = 0;

	// root has changed so file is dirty
	_dirty = true;
}

OPersist *OFile::getRoot(OClassId_t cid)
// Return a pointer to the root Object.
// Parameter: cid - class id of the root object(default cOPersist)
{
	return getObject(_rootId,cid);
}


OFile *OFile::getTail()
// Static , Private
// Return the last OFile in the list. If the list is empty retun 0.
{
	if(_sFileListHead)
	{
		OFile *next = _sFileListHead;
		while(next->_next)
			next = next->_next;
		return next;
	}
	else
		return 0;
}


OFile *OFile::getOFile(int fileId)
// Static
// Return the file pointer of the OFile with identity fileId. Return
// 0 if it does not exist.
{
	OFile *f = getFirstOFile();
	while(f)
	{
		if(f->id() == fileId)
			return f;
		f = f->getNextOFile();
	}
	return 0;
}

void OFile::closeAll(void)
// Static
// Close all OFiles.
{
	// delete from the back of the list.
	while(getFirstOFile())
		delete getTail();
}

void OFile::fastFindOff(void)
// Switch off fastFind and release any memory used by it.
{
	delete _oList;
	_oList = 0;
	_operation = _operation & ~OFILE_FAST_FIND;
}
void OFile::attach(OPersist *ob,bool deep)
// Insert an object into the file.
// Parameter: deep - true - send the message oAttach to the object.
{
	if(ob)
		ob->oAttach(this,deep);
}

void OFile::pInsert(OPersist *ob)
// Private.
// Insert an object into the file. Set the file dirty.
// Parameter: ob - Pointer to object to be inserted.
{
	if(!ob->oAttached()){
		// Do not enter in more than one thread.
    	OFGuard guard(_mutex);

		// Set it dirty
		ob->oSetDirty();

		ob->oSetInFile(true);

		// If the object does not have an identity give it a unique identity.
		if(!ob->hasIdentity())
			ob->setId(_uniqueId++);
		else
			// Make sure the _uniqueId is bigger than the assigned identity, to
			// ensure that it will not be generated again.
			_uniqueId = max(_uniqueId,(ob->oId() + (OId)1));

		// insert into Object list
		pair<ClassList::iterator,bool>ret = _cList.classListCr(ob->meta()->id()).insert(ClassList::value_type(ob->oId(),OEnt(ob,0)));
		oFAssert(ret.second);
		if(OFILE_FAST_FIND & _operation)
			_oList->insert(ObjectList::value_type(ob->oId(),ob->meta()->id()));

		// File is dirty
		_dirty = true;
	}
}

void OFile::detach(OPersist *ob,bool deep)
// Erase an object from the file. This will invalidate any iterator pointing
// to that object.
// Parameter: deep - true - send the message oDetach to the object.
{
	if(ob)
		ob->oDetach(this,deep);
}

void OFile::pErase(OPersist *ob)
// Private.
// Erase an object from the file. This will invalidate any iterator pointing
// to that object. Set the file dirty.
// Parameter: ob - Pointer to object to be erased.
{
	if(ob->oAttached())
	{
		// Do not enter in more than one thread.
    	OFGuard guard(_mutex);

		ClassList::iterator it = _cList.classList(ob->meta()->id()).find(ob->oId());
		// Attempt to detach object for a second time !
		oFAssert(it != _cList.classList(ob->meta()->id()).end());

		// Free the space in the file
		const OEnt &oe = (*it).second;
		if(oe._mark && oe._length)
			_fList.freeSpace(oe._mark,oe._length);

		// Erase from the class list
		_cList.classList(ob->meta()->id()).erase(it);

		if(OFILE_FAST_FIND & _operation)
			// Erase from object list.
			_oList->erase(ob->oId());

		ob->oSetInFile(false);

		if(!_retainIdentity)
		{
	// Set the identity to 0 because it is no longer relevant when the
	// object is not in a file.
	// This is a safety feature to prevent problems when objects are
	// detached from one file and then attached to another. The second
	// file may already have such an id or its _uniqueid may not yet
	// have reached the new objects id.
			ob->setId(0);
		}

		// File is dirty
		_dirty = true;
	}
}

oulong OFile::objectCount(OClassId_t id,bool deep)
// Return  the number of objects of the class id in the file.
// Parameter deep : true - include all its subclasses.
{
	const OMeta::Classes &classes = OMeta::meta(id)->classes(deep);

	oulong count = 0;
	for(OMeta::Classes::const_iterator it = classes.begin(); it != classes.end();++it)
		count += (oulong)_cList.classList((*it)).size();
	return count;
}


OPersist *OFile::getObject(const OId oId,OClassId_t cId)
// Returns a pointer to the object with identity oId.
// Note: The cId parameter can be the searched objects class, or a
//       super-class of it. The more precisely it is specified, the
//       faster the function will work.
{

	if(OFILE_FAST_FIND & _operation)
	{
		ObjectList::const_iterator oret = _oList->find(oId);
		ObjectList::const_iterator end = _oList->end();
		if(oret == end)
			// Object not found
			return 0;
		cId = (*oret).second;
	}

	pair<ClassList::iterator,OClassId_t>ret = _cList.find(oId,cId);

	if(ret.second == 0)
		// Object not found
		return 0;

	return getObject(ret.first,ret.second);
}

OPersist *OFile::getObject(ClassList::iterator it,OClassId_t cId)
// Private.
// Get an object from its iterator.
{
	// Do not enter in more than one thread.
    OFGuard guard(_mutex);

	if((*it).second._ob)
	{
		OPersist *ob = (*it).second._ob;
		// Object is not purgeable because we are referencing it.
		ob->pSetPurgeable(false);
		return ob;
	}
	else
	{
		// Start reading object
		_in.start((*it).second._mark,(*it).second._length);

		// Set the current index so that OPersist's constructor can update it
		_currentIndex = it;

		OMeta *meta = OMeta::meta(cId);
		OPersist *ob;
		try
		{
			ob =  meta->construct(_in);
		}catch(...){
			// Abort reading of this object.
			_in.abort();
			// clean the index, because the object was not constructed.
			(*it).second._ob = 0;
			throw;
		}
		(*it).second._ob = ob;
		// Terminate reading object.
		_in.finish();

	    ob->setId((*it).first);

		// Set inFile now we are sure it exists.
		ob->oSetInFile(true);

		return ob;
	}
}

OPersist *OFile::restore(OPersist *ob)
// Restore an object with data from the file. The objects address is
// invalidated.
// Parameter ob: The object to be restored.
// Return value: the restored object. This may have different address from the original.
//               If the object was never in the file 0 is returned.
{
	oFAssert(ob);

	if(ob->oDirty())
	{
		// Do not enter in more than one thread.
    	OFGuard guard(_mutex);

		// Recover from the file.
		OId id = ob->oId();
		OClassId_t cId = ob->meta()->id();

		ClassList::iterator it = _cList.classList(cId).find(id);

		// The old object is deleted
		OPersist *ob = (*it).second._ob;
		delete ob;
		(*it).second._ob = 0;

		// Re-read the object.
		return getObject(id,cId);
	}
	else
		// Return the existing object because it was not dirty.
		return ob;
	
}


long OFile::purge(OClassId_t cId,bool deep,long toPurge)
// Purge from memory objects of the file that  are marked purgeable.
// Parameters: cId - Types of objects to purge.
//             deep -  if true include sub-classes.
//			   toPurge - number of objects to purge.
// Return the number of bytes purged. 
{
	// Do not enter in more than one thread.
    OFGuard guard(_mutex);

	long objectsPurged = 0;

	// Set to neutralize the check on illegel object destruction by the application.
	bool save_permitObjectDestruction = _sPermitObjectDestruction;
	_sPermitObjectDestruction = true;

	try{
		const OMeta::Classes &classes = OMeta::meta(cId)->classes(deep);

		OMeta::Classes::const_iterator begin = classes.begin();
		OMeta::Classes::const_iterator end = classes.end();

		for(OMeta::Classes::const_iterator cSetIt = begin;cSetIt != end;++cSetIt)
		{
			ClassList::iterator cBegin = _cList.classList(*cSetIt).begin();
			ClassList::iterator cEnd = _cList.classList(*cSetIt).end();

			for(ClassList::iterator cListIt = cBegin;
				 cListIt != cEnd;
				 ++cListIt){
			
			   OPersist *ob = (*cListIt).second._ob;
			   if(ob && ob->oPurgeable() && !ob->oDirty())
			   {
			   // Purge the object from memory.
			      delete ob;
				  (*cListIt).second._ob = 0;
				  objectsPurged++;
			   }
			   if(objectsPurged >= toPurge)
			      break;
			}
			if(objectsPurged >= toPurge)
				break;
		}
	}catch(...){
		_sPermitObjectDestruction = save_permitObjectDestruction;
		throw;
	}

	_sPermitObjectDestruction = save_permitObjectDestruction;
	return objectsPurged;
}

long OFile::purgeAll(void)
// Static
// Purge all files. If a file is marked autoCommit the purge process
// may also commit the file.
// The strategy implemeted in purgeAll() is to purge all objects
// in memory. This is good when sequentially writing a large file.
// Return value - number of bytes purged.
{
	long objectsPurged = 0;
	OFile *f = _sFileListHead;

	// Try to recover memory just by purging.
	while(f)
	{
		objectsPurged += f->purge();
		f = f->_next;
	}

	// If we did not succede in purging then try committing first.
	if(0 == objectsPurged)
	{
		f = _sFileListHead;
		while(f)
		{
			// We can only commit if the file is marked autoCommit
			// and is not read only.
			if(f->_autoCommit && !f->isReadOnly())
			{
				try
				{
					f->commit();
					objectsPurged += f->purge();
				}catch(OFileErr)
				{ // Failed to commit do nothing
				}
			}
			f = f->_next;
		}
	}

	return objectsPurged;
}

void OFile::write(OOStreamFile *out)const
// Private.
// Write the OFile 'object'. This includes the indexes and the free list.
{
	out->writeLong(_uniqueId);

	// Write the object entries.
	for(OClassId_t cId = 1; cId <= cOMaxClasses; cId++)
	// For each class
	{
		if(_cList.classList(cId).begin() != _cList.classList(cId).end())
		{

			// Write the class header
			out->writeShort((short)cId);
			out->writeLong((O_LONG)_cList.classList(cId).size());

			// Write object headers
			for(ClassList::iterator it = _cList.classList(cId).begin(); it != _cList.classList(cId).end();++it)
			{
				out->writeObjectId((*it).first);
				out->writeFilePos((*it).second._mark);
				out->writeLong((*it).second._length);
			}
		}
	}
	out->writeShort(0);  // terminator
}

bool OFile::isDirty(void)
// Return true if this OFile is not consistent with its disk file.
// If a single object in the file is dirty then true is returned.
// Also if an attach or detach operation has been made true is returned.
{
	// Once found to be dirty no need to check again.
	if(_dirty)
		return true;


	// Iterate over all objects. If one is found to be dirty then the
	// the file is dirty.
	const OMeta::Classes &classes = OMeta::meta(cOPersist)->classes(true);

	OMeta::Classes::const_iterator begin = classes.begin();
	OMeta::Classes::const_iterator end = classes.end();

	for(OMeta::Classes::const_iterator cSetIt = begin;cSetIt != end;++cSetIt)
	{
		ClassList::iterator cBegin = _cList.classList(*cSetIt).begin();
		ClassList::iterator cEnd = _cList.classList(*cSetIt).end();

		for(ClassList::iterator cListIt = cBegin;
			 cListIt != cEnd;
				 ++cListIt)
		{		
		    OPersist *ob = (*cListIt).second._ob;
		    if(ob && ob->oDirty())
			{
				_dirty = true;
				return true;
			}				
		}
	}
	return false;
}

void OFile::setObjectOId(OPersist *ob,OId id)
// Set the object identity of ob to id.
// This method would not normally be used. It could be useful
// whne you want to change an objects identity to that of an
// object you are removing, so as to keep unresolved references
// to that object consistent.
{
	// Assert if the id is already in use.
	oFAssert(!_cList.find(id,cOPersist).second);

	bool attached = ob->oAttached();

	// Remove from file
	if(attached)
		pErase(ob);

	// set the id
	ob->setId(id);

	// Re-attach to file if it was previously attached.
	if(attached)
		pInsert(ob);
}

void OFMemoryRegister::subtract(long nBytes)
// Subtract from the memory register. This does a consistency check.
{
	unsigned long prev = _nBytes;

	_nBytes -= nBytes;

	// There is a bug somewhere if we are using a negative amount of memory.
	oFAssert(prev >= _nBytes);
}

void OFMemoryRegister::add(long nBytes)
// Add to the memory register.
{
	_nBytes += nBytes;
}
