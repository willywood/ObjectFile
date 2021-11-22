#ifndef OSET_H
#define OSET_H
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


#include "opersist.h"
#include <set>

#ifdef OFILE_STD_IN_NAMESPACE
using std::set;
using std::binary_function;
#endif

class OFile;
class OOStream;
class OIStream;

template <class T>
struct oDemand_pointer_less : binary_function<T, T, bool> {
	bool operator()(const T& x, const T& y) const
	{ return (OAddress_type)x < (OAddress_type)y; }
};

template <class T>
class OSetT : public std::set<T *,oDemand_pointer_less<T *> >
{
public:
	// Read constructor
	OSetT(OIStream *,const char*);
	// Default Constructor
	OSetT(void){}
	void destroy(void);

	void oAttach(OFile *file,bool);
	void oDetach(OFile *file,bool);

	void oWrite(OOStream *,const char *label = 0)const;

};



template <class T>
OSetT<T>::OSetT(OIStream *in,const char *label)
// Object file read constructor
{
	OIStream::ODefineObject def(in,label);
	// number of elements in list
	long size = in->readLong("size");

	for(long i = 0; i < size;i++)
		// add to list
		this->insert((T *)in->readObject("item"));

}

template <class T>
void OSetT<T>::destroy(void)
// Call this function only if caller is not persistant, since it is forbidden to
// directly delete a persistant object.
{
	for(typename OSetT<T>::iterator it = this->begin();it != this->end(); ++it)
		delete *it;
}

template <class T>
void OSetT<T>::oWrite(OOStream *out,const char *label)const
{
	OOStream::ODefineObject def(out,label);

	// number of elements in list
	out->writeLong((O_LONG)this->size(),"size");

	// for every element in the list
	for(typename OSetT<T>::const_iterator it = this->begin();it != this->end(); ++it)
		// Write out the element
		out->writeObject(*it,"item");

}

template <class T>
void OSetT<T>::oAttach(OFile *file,bool deep)
{
	// for every element in the list
	for(typename OSetT<T>::iterator it = this->begin();it != this->end(); ++it)
		// Insert object into the file
		(*it)->oAttach(file,deep);

}

template <class T>
void OSetT<T>::oDetach(OFile *file,bool deep)
{
	// for every element in the list
	for(typename OSetT<T>::iterator it = this->begin();it != this->end(); ++it)
		// Erase from file but not from this collection
		(*it)->oDetach(file,deep);
}

// Non typesafe set
typedef OSetT<OPersist> OSet;


#endif
