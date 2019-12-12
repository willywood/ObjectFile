#ifndef OITER_H
#define OITER_H
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



