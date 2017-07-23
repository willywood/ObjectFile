//======================================================================
// 	This software may only be compiled in conjunction with a licence
// agreement with ObjectFile Limited.
// Copyright(c) 1996-99 ObjectFile Ltd. 
//======================================================================
//
// Continued from OFile.cpp
//

#include "odefs.h"
#include "ofile.h"
#include "ometa.h"
#include "ostrm.h"
#include "opersist.h"
#include "ox.h"
#include "oiter.h"
#include "opersist.h"

// Instantiate the empty classlist.
OFile::ClassList OFile::ClassLists::_empty;

OFile::ClassLists::ClassLists(void)
// Constructor
{

	for(int i = 0; i < cOMaxClasses; i++)
		_classLists[i] = 0;
}

OFile::ClassLists::~ClassLists(void)
// Destructor
{
	for(int i = 0; i < cOMaxClasses; i++)
		delete _classLists[i];
}


OFile::ClassList &OFile::ClassLists::classListCr(OClassId_t id)
// Accessor for ClassLists. Create one if it does not exist.
{
	// Create class lists as needed
	if(_classLists[id - 1] == 0)
		_classLists[id - 1] = new ClassList;
	return *_classLists[id - 1];
}

OFile::ClassList &OFile::ClassLists::classList(OClassId_t id)const
// Accessor for ClassLists. If there is none return the empty classList.
{
	if(_classLists[id - 1] == 0)
		return _empty;
	else
		return *_classLists[id - 1];
}

pair<OFile::ClassList::iterator,OClassId_t> OFile::ClassLists::find(OId id,OClassId_t cId)const
{
	ClassList::iterator it; // = end();

	// Make a set of all the classes to be iterated over
	const OMeta::Classes &classes = OMeta::meta(cId)->classes();

	OMeta::Classes::const_iterator begin = classes.begin();
	OMeta::Classes::const_iterator end = classes.end();

	for(OMeta::Classes::const_iterator cSetIt = begin;cSetIt != end;++cSetIt)
	{
		it = classList(*cSetIt).find(id);
		if(it != classList(*cSetIt).end())
			return make_pair(it,(OClassId_t)*cSetIt);

	}
	return make_pair(it,(OClassId_t)0);
}

OFilePos_t OFile::allocateObject(ClassList::iterator it,long objectLength)
// Private - Allocate space in the file for the object.
// Return the position in the file.
{
	// Fill in the object entry of the object
	if((*it).second._length != objectLength){
		// Objects size has changed so release its old space
		if((*it).second._mark)
			_fList.freeSpace((*it).second._mark,(*it).second._length);
		// and find a new place for it
		(*it).second._mark = _fList.getSpace(objectLength);
		(*it).second._length = objectLength;
	}
	return (*it).second._mark;
}


void OFile::commit(bool /* compact */,bool wipeFreeSpace)
// Commit the file to the disk.
// Parameters: compact - Obsolete - see OUFile::compact()
// wipeFreeSpace - writes a character('\xFE) over the free space. 
//				   default is false. This should
//                 improve the compression ratio when zipped.
//
// commit makes 2 passes over the objects. The first pass just 
// calculates the length.
// The second actually writes the objects.
// Exceptions: OFileErr is thrown if the file cannot be written.
{
long objectLength;
OFilePos_t mark = 0;
OClassId_t cId;

	// Should not be committing a readonly file.
	oFAssert(!isReadOnly());

	// Do not enter in more than one thread.
    OFGuard guard(_mutex);

	OOStreamFile out(this);

	// ===================   PASS 1   =====================

	// De-allocate the space for the indexes. This is so that no holes
	// are left.
	if(_oFileMark)
		_fList.freeSpace(_oFileMark,_oFileLength);

	// Just calculate the file length
	// visit each object
	for(cId = 1; cId <= cOMaxClasses; cId++){
		for(ClassList::iterator it = _cList.classList(cId).begin(); it != _cList.classList(cId).end();++it){

			OPersist *ob = (*it).second._ob;

			if(!ob || !ob->oDirty())
				// No need to write an object that has not been read or has not been changed.
				continue;

			if((objectLength = ob->oSize()) == -1){

				// Object of unknown size so
				// calculate the length of the object entry.
				out.start(mark,objectLength,true);
				ob->oWrite(&out);
				objectLength = out.finish();

			}

			// Fill in the object entry of the object
			mark = allocateObject(it,objectLength);
		}
	}

	// Now allocate space for the indexes and freelist. The problem here is that we must allocate
	// space before writing the object. This is because the free list must be
	// determined before writing it, otherwise we would lose the last change.
	// The free list can only get smaller by calling getSpace, so we may
	// get a little bit more than we really need


	// Calculate size of OFile data
	long fileSize = size();
	_oFileLength = 	fileSize + (oulong)_fList.size();

	// and get space for it
	_oFileMark = _fList.getSpace(_oFileLength);

	// Now that we know the length of the file we require, try to set it.
	// We do this here so that if we fail we will not be left with a half
	// written file.
	if(!out.setLength(_fileLength))
		throw OFileErr("Failed to allocate space on the disk for the file.");

	// ===================   PASS 2   =====================

	// Actually write the object
	// visit each object
	for(cId = 1; cId <= cOMaxClasses; cId++)
	{
		for(ClassList::iterator it = _cList.classList(cId).begin();
		    it != _cList.classList(cId).end();
		    ++it)
		{

			OPersist *ob = (*it).second._ob;

			if(!ob || !ob->oDirty())
				// No need to write an object that has not been read or has not been changed.
				continue;

			// Write it to the output stream.
			out.start((*it).second._mark,(*it).second._length);
			ob->oWrite(&out);
			out.finish();

			// Object is now safely on file.
			ob->oSetClean();
		}
	}

	// Update file version
	_userVersion = _sUserSourceVersion;
	_oFileVersion = _sOFileSourceVersion;

	// Write the header - size 100 bytes.
	out.start(0,cHeaderLength);

	// Ensure the processorid does not get swapped.
	out.writeBytes(&_fileProcessorId,4);
	out.writeLong(_oFileVersion);
	out.writeLong(_userVersion);
	out.writeFilePos(_oFileMark);
	out.writeLong(_oFileLength);
	out.writeObjectId(_rootId);
	out.writeFilePos(_fileLength);
	out.writeBytes(_magicNumber,4);

	for(int i = 5*sizeof(long)+ 2*sizeof(OFilePos_t)+sizeof(OId); i < cHeaderLength ; i += sizeof(0L))
		out.writeLong(0);

	out.finish();

	// We must recalculate the length because the freelist size might
	// have got smaller.
	out.start(_oFileMark,fileSize + _fList.size());
	write(&out);
	// Write the free list.
	_fList.write(&out,wipeFreeSpace);
	out.finish();

	// File is no longer dirty
	_dirty = false;
}


long OFile::size(void)const
// Return the size of OFile as required in the file.
{
	long nObjects = 0;
	OClassId_t nClasses = 0;
	for(OClassId_t cId = 1; cId <= cOMaxClasses; cId++){
		nObjects += (long)_cList.classList(cId).size();
		if(_cList.classList(cId).size())
			nClasses++;
	}

	return			 	sizeof(_uniqueId) +
						nObjects*(sizeof(OId)+ 		// OEnt - Object entries
								  sizeof(OFilePos_t)+
								  sizeof(long))+
						nClasses*(sizeof(short) +  	// class id
								  sizeof(long))+    // Class list headers
						sizeof(short);              // terminator

}


void OFile::increaseLengthBy(oulong len)
// Expand the file length by len bytes.
// Throw OFileIOErr if the file will exceed the maximum permitted.
{
	// Check that we do not exceed the maximum. Condition is reordered
	// to avoid overflow. If len is greater than cOFileMaxLength then
	// we can get underflow. However this is unlikely.
	//	if(_fileLength + len + 10 > cOFileMaxLength())
	//  cOFileMaxLength() is defined in odefs.h
	// The 10 is to allow for any end of file mark that may be needed.
	if(_fileLength > (cOFileMaxLength() - 10 - len))
		throw OFileErr("Attempt to increase the file beyond the maximum permitted.");

	_fileLength += len;
}
