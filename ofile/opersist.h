#ifndef OPERSIST_H
#define OPERSIST_H
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


#include "ofile.h"
#include "ometa.h"

class OOStream;
class OIStream;

class OFile;

class OPersist  /* Derive from a framework super-class here */
{
public:
friend class OFile;

	// Default Constructor
	OPersist(void);
	// Read constructor
	OPersist(OIStream *);
	// Copy constructor
	OPersist(const OPersist &);

	virtual ~OPersist(void);

	void *operator new(size_t size);
	void operator delete(void *ob);

	OId oId(void)const;
	virtual long oSize(void)const{return -1;}

	virtual void oAttach(OFile *,bool deep);
	virtual void oDetach(OFile *,bool deep);

	virtual OMeta *meta(void)const{return &_metaClass;}

	bool oDirty(void)const{return _npFlags.dirty == 1;}
	void oSetDirty(void){_npFlags.dirty = 1;}

	virtual void oSetPurgeable(bool deep = true,OFile *file = 0);

	bool oAttached(void)const{return _npFlags.inFile == 1;}
	virtual void oWrite(OOStream *)const;


#ifdef OF_REF_COUNT
	void addRef(void)const;
	void removeRef(void)const;
#endif


//	void copy(OPersist *,bool deep);
//	virtual OPersist *clone(OPersist *,bool deep);

protected:
  	void setId(OId id);

private:
	void oSetClean(void){_npFlags.dirty = 0;}
	void oSetInFile(bool inFile){_npFlags.inFile = inFile ? 1 : 0;}
	bool hasIdentity(void)const{return _oId != 0;} 
	void pSetPurgeable(bool purgeable);
	bool oPurgeable(void)const{return _npFlags.purgeable == 1;}

private:
	OId _oId;	 // Object identity

	struct{      // Non-persistant flags

// OF_REF_COUNT is defined in odefs.h
#ifdef OF_REF_COUNT
		unsigned short refCnt;         // Reference count
#endif
		unsigned int dirty     : 1;    // Object has changed since reading
		unsigned int purgeable : 1;    // Object can be purged from memory
		unsigned int inFile    : 1;    // Object is in the file
		unsigned int           : 5;	   // Not used.
	}_npFlags;

	// Instantiation function
	static OPersist *New(OIStream *s){return new OPersist(s);}
	// Meta class
	static OMeta _metaClass;
};

// OProtect is an exception safe way of making an object purgeable.
// When it goes out of scope the object is made purgeable.
// It assumes the object is not purgeable to start with.
// Use:
//       OProtect p(ob);

class OProtect
{
public:
	OProtect(OPersist *ob):_object(ob){}
	~OProtect(void){_object->oSetPurgeable();}
private:
	OPersist *_object;
};



#endif

