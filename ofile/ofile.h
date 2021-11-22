#ifndef OFILE_H
#define OFILE_H
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



// Definitions for threads
#include "ofthread.h"
#include "oio.h"
#ifdef WIN16
#define Allocator long_allocator
#include <lngalloc.h>
#endif

#ifdef OF_HASH
#include <hash_map.h>
#else
#include <map>
#endif
#include <limits.h>
#include <algorithm>
#include "oflist.h"
#include "ostrm.h"
#include "oistrm.h"

#ifdef OFILE_STD_IN_NAMESPACE
using std::map;
using std::pair;
using std::make_pair;
using std::min;
#endif


class OIterator;


class OFile
{

// These friends are defined so as to provide only the necassary
// methods to the user of OFile, and no more.
friend class OIterator;
friend class FreeList;
friend class OPersist;
friend class OOStreamFile;
friend class OIStreamFile;

	enum {cHeaderLength = 100 + 2*(sizeof(OFilePos_t) - sizeof(long))};

class OEnt{
// Node of a class list.
public:
	~OEnt(){}
	OEnt(OPersist *ob,OFilePos_t fMark):_ob(ob),_mark(fMark),_length(0){}
	OEnt(OFilePos_t mark,long length):_ob(0),_mark(mark),_length(length){}

	OPersist *_ob;	 // Pointer to object. 0 if object is not in memory
	OFilePos_t _mark;	 // Objects position in file. 0 if not yet written to file
	oulong _length;	 // length of object in file.
private:
	OEnt(){}

};
public:
// These would benefit from an allocator using a fixed size block heap.
#ifdef OF_HASH
// You may be able to save memory with hash tables instead of sets.
// This is not defined usually.
typedef hash_map<OId,OEnt,hash <OId >,equal_to<OId> >  ClassList;
typedef hash_map<OId,OClassId_t,hash <OId >,equal_to<OId>  > ObjectList;
#else
typedef map<OId,OEnt,less <OId > >  ClassList;
typedef map<OId,OClassId_t,less<OId> > ObjectList;
#endif
private:
class ClassLists{
// The class lists.
public:
	ClassLists(void);
	~ClassLists(void);
	ClassList &classListCr(OClassId_t id);
	ClassList &classList(OClassId_t id)const;
	pair<ClassList::iterator,OClassId_t> find(OId,OClassId_t = cOPersist)const;
private:
	ClassList *_classLists[cOMaxClasses];
	static ClassList _empty;
};

public:
	typedef void (*New_handler)();

	OFile(const char *fname,long operation,const char *magicNumber = 0);
#ifdef OF_OLE
	OFile(IStorage *istorage,const char *fname,long operation,
			unsigned long istorage_mode,
			const char *magicNumber = 0);
#endif

	virtual ~OFile(void);


	// User functions
	void attach(OPersist *,bool deep = true);
	void detach(OPersist *,bool deep = true);

	oulong objectCount(OClassId_t id = cOPersist,bool deep = true);
	OPersist *getObject(const OId,OClassId_t = cOPersist);
	virtual void commit(bool compact = false,bool wipeFreeSpace = false);
	void fastFindOff(void);
	long purge(OClassId_t cId = cOPersist,bool deep = true,long toPurge = LONG_MAX);
	OPersist *restore(OPersist *ob);
	void setObjectOId(OPersist *ob,OId id);

	// Version control methods
	// Return the version of this file.
	long userVersion(void)const{return _userVersion;}
	// Return the version of the source code.
	static long userSourceVersion(void){return _sUserSourceVersion;}
	// Set the version of the source code.
	static void setUserSourceVersion(long v){_sUserSourceVersion = v;}

	bool needSwap(void)const{return	_sProcessorId != _fileProcessorId;}

	virtual bool isDirty(void);
	bool isReadOnly(void)const{return (OFILE_OPEN_READ_ONLY & _operation) == OFILE_OPEN_READ_ONLY;}
	const char *magicNumber(void){return _magicNumber;}

	void setAutoCommit(bool autoCommit = true){_autoCommit = autoCommit;}
	bool isAutoCommit(void)const{return _autoCommit;}

	// This function should be used with care.
	void setRetainIdentity(bool retainIdentity){_retainIdentity = retainIdentity;}
	bool retainIdentity(void)const{return _retainIdentity;}

	static OFile *oFileOf(OPersist *ob);

	// Object cache management.
	static void setObjectThreshold(long t){_sObjectThreshold = t;}
	static long getObjectThreshold(void){return _sObjectThreshold;}
	static long getObjectCacheCount(void){return _sObjectCacheCount;}
	static void new_handler();
	static New_handler set_new_handler(New_handler newNewHandler);
	static long purgeAll(void);

	// File list access.
	static OFile *getFirstOFile(void){return _sFileListHead;}
	OFile *getNextOFile(void)const{return _next;}

	// root access
	void setRoot(OPersist *);
	OPersist *getRoot(OClassId_t cid = cOPersist);

	int id(void){return _fileId;}
	static OFile *getOFile(int fileId);
	static void closeAll(void);

	// Freelist management functions.Used by OBlob. These must be used with extreme
    // caution, otherwise they can screw up the file.
  	OFilePos_t getSpace(oulong length){return _fList.getSpace(length);}
	void freeSpace(OFilePos_t mark,oulong length){_fList.freeSpace(mark,length);}
	// Read a blob of binary data from the given position in the file.
   	void readBlob(void *buf,OFilePos_t mark,unsigned long size){_in.readBlob(buf,mark,size);}

	// Physically close the current file
	void close(void);
	// Physically reopen the current file
	virtual void reopen(void);

protected:
	void reopen(const char *fileName);
	OFilePos_t getLength(void)const{return _fileLength;}  // Lazy file length
	void reset(void);

private:
	void init(const char *fname,const char *magicNumber = 0);
	// Used by friend: OPersist
	void pInsert(OPersist *);
	void pErase(OPersist *);

	OPersist *getObject(ClassList::iterator it,OClassId_t);
	// Used by friend: FreeList
	void setLength(OFilePos_t len){_fileLength = len;}
	void increaseLengthBy(oulong len);
	long size(void)const;
	FreeList *freeList(void){return &_fList;}
	// Accessors for Object File only
	O_fd *fd(void){return _in.fd();}
	void pClear();


private:
	void write(OOStreamFile *)const;
	OFilePos_t allocateObject(ClassList::iterator it,long objectLength);
	void setCurrentIndex(OPersist *p){(*_currentIndex).second._ob = p;}
	static OFile *getTail();

private:
	static long _sOFileSourceVersion;	   // 
	static long _sUserSourceVersion;	   //
	static bool _sPermitObjectDestruction; //
	static unsigned long _sProcessorId;	   //
	static OFile *_sFileListHead;		   // File list head

	static long _sObjectThreshold;          // Maximum number of objects in memory
	static long _sObjectCacheCount;         // Current number of objects in memory
	static New_handler _sNew_handler;       // handler for when the object threshold
										    // is exceeded.
	static int _sUniqueFileId;              // First avaialable unique identity of OFile.

	ObjectList *_oList;	 // Object list (used by fastFind option)
	ClassLists _cList;	 // Class list
	FreeList _fList;	 // Free list
	OIStreamFile _in;	 // Input stream to disk file.

	OId _uniqueId;		 // First available unique object identity in file.
	OFilePos_t _fileLength;  // Length of the file in bytes(lazy).
	OFilePos_t _oFileMark;	 // File position of the OFile object.
	long _oFileVersion;	 // Version of file
	long _oFileLength;   // Length of the OFile object
	unsigned long _fileProcessorId;
	long _userVersion;	 // User version of file
    ClassList::iterator _currentIndex;
	OFile *_next;        // Maintain a null terminated linked list of files.
	OId _rootId;         // Identity of root object.
	char _magicNumber[4];// File identification.
	bool _dirty;         // true if objects have been changed but not commited
	bool _autoCommit;	 // Allow file to automatically commit.
	bool _retainIdentity;// Retain the identity of objects when detaching from
						 // the file. Default is false.

protected:
	long _operation;     // Flags that were used when opening this file.
	int _fileId;		 // Unique file identifier.
					     // This is not persistent so next time the file
					     // is opened it will have a new identity.

	OFMutex _mutex;      // per file mutex
    static OFMutex _sMutex; // Global mutex
public:
	OFMutex &mutex(void){return _mutex;}
};



#ifdef WIN16
#undef Allocator
#endif

#endif
