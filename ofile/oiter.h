#ifndef OITER_H
#define OITER_H
//======================================================================
// 	This software may only be compiled in conjunction with a licence
// agreement with ObjectFile Limited.
// Copyright(c) 1996-99 ObjectFile Ltd.
//======================================================================


#include "ofile.h"
#include "ometa.h"

class OIterator{
public:

	OIterator(OFile *ofile,OClassId_t classId = cOPersist,bool deep = true);
//	OIterator(OFile *ofile,OId id,OClassId_t classId = cOPersist,bool deep = true);
	void reset(void);
	OPersist *begin(void);
	OPersist *end(void);
	OPersist *object(OId);

	OPersist* operator*();
	OPersist* operator++();     // prefix  ++a
	OPersist* operator++(int);  // postfix  a++

private:
	const OMeta::Classes &_classes;
	OFile *_oFile;
	OMeta::Classes::const_iterator _cSetIt;
	OFile::ClassList::iterator _cListIt;
};


// Template to allow typesafe access to the iterator.
template <class T,OClassId_t TcId>
class OIteratorT: public OIterator{
public:

	OIteratorT(OFile *ofile,bool deep = true):
		OIterator(ofile,TcId,deep){}
//	OIteratorT(OFile *ofile,OId id,bool deep = true):
//		OIterator(ofile,id,TcId,deep){}

	void reset(void){OIterator::reset();}

	// dynamic_cast is required to cast to a sub-class of a virtual base.
	T *begin(void){return dynamic_cast<T *>(OIterator::begin());}
	T *end(void){return dynamic_cast<T *>(OIterator::end());}

	T *object(OId id){return dynamic_cast<T *>(OIterator::object(id));}

	T* operator*(){return dynamic_cast<T *>(OIterator::operator*());}
	T* operator++(){return dynamic_cast<T *>(OIterator::operator++());}     // prefix  ++a
	T* operator++(int i){return dynamic_cast<T *>(OIterator::operator++(i));}  // postfix  a++

};

#endif



