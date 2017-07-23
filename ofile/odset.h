#ifndef ODSET_H
#define ODSET_H
//======================================================================
// 	This software may only be compiled in conjunction with a licence
// agreement with ObjectFile Limited.
// Copyright(c) 1996-2000 ObjectFile Ltd. 
//======================================================================

//	OnDemandSet
// This is a set designed to hold OnDemands.
// Objects must all be either attached or not attached to the file.
// Mixing the two is not allowed
//
#include <set>
#include "odemand.h"

#ifdef OFILE_STD_IN_NAMESPACE
using std::set;
using std::binary_function;
#endif

class OFile;
class OOStream;
class OIStream;

template <class T>
struct oDemand_less : binary_function<T, T, bool> {
	bool operator()(const T& x, const T& y) const;
};

template <class T>
	bool oDemand_less<T>::operator()(const T& x, const T& y) const
	{
		if(x.oAttached())
		{
			// Mixing attached and not attached objects is not allowed
			oFAssert(y.oAttached());
			// Sort by persistant identity if attached
			return x.oId() < y.oId(); 
		}
		else
		{
			// Mixing attached and not attached objects is not allowed
			oFAssert(!y.oAttached());
			// otherwise by object pointer. This must be cast to a type
			// that can be compared, because C pointers may only be 
			// normally be tested for inequality.
			return (OAddress_type)x.object() < (OAddress_type)y.object();
		}
	}


template <class T>
class OnDemandSetT : public set<T,oDemand_less<T> >
{
public:
	// Read constructor
	OnDemandSetT(OIStream *,const char *label=0);
	// Default Constructor
	OnDemandSetT(void){}
	// Copy constructor
	OnDemandSetT(const OnDemandSetT &x):set<T,oDemand_less<T> >(x){}

	void destroy(void);

	void oAttach(OFile *file,bool);
	void oDetach(OFile *file,bool);

	void oSetPurgeable(bool deep = true,OFile *file = 0);

	void oWrite(OOStream *,const char *label=0)const;

};



template <class T>
OnDemandSetT<T>::OnDemandSetT(OIStream *in,const char* label)
// Object file read constructor
{
	OIStream::ODefineObject def(in,label);
	// number of elements in list
	long size = in->readLong("size");

	for(long i = 0; i < size;i++)
		// add to list
		this->insert(T(in,"item"));

}

template <class T>
void OnDemandSetT<T>::destroy(void)
// Call this function only if caller is not persistant, since it is forbidden to
// directly delete a persistant object.
{
	typename OnDemandSetT<T>::iterator it;

	for(it = this->begin();it != this->end(); ++it)
		delete (*it).object();
}


template <class T>
void OnDemandSetT<T>::oWrite(OOStream *out,const char *label)const
{
	OOStream::ODefineObject def(out,label);
	// number of elements in list
	out->writeLong(this->size(),"size");

	// for every element in the list
	typename OnDemandSetT<T>::const_iterator it;
    for(it = this->begin();it != this->end(); ++it)
		// Write out the element
		(*it).oWrite(out,"item");

}

template <class T>
void OnDemandSetT<T>::oAttach(OFile *file,bool deep)
{
	OnDemandSetT copy(*this);

	// Empty this set
	this->erase(this->begin(),this->end());

	// for every element in the list
	typename OnDemandSetT<T>::const_iterator it;
	for(it = copy.begin();it != copy.end(); ++it)
	{
		// Attach object to the file. This changes the id.
		(*it).oAttach(file,deep);
		// insert back into set
		this->insert(*it);
	}

}

template <class T>
void OnDemandSetT<T>::oDetach(OFile *file,bool deep)
// Detach OnDemands from file.
// You would not normally want to do this. If the objects are not
// in the file, then they must all be in memory. Memory may not be
// big enough to hold them.
{
	OnDemandSetT copy(*this);

	// Empty this set
	this->erase(this->begin(),this->end());

	// for every element in the list
	typename OnDemandSetT<T>::const_iterator it;
    for(it = copy.begin();it != copy.end(); ++it)
	{
		// Detach from file. This changes the id.
		(*it).oDetach(file,deep);
		// insert back into set
		this->insert(*it);
	}
}

template <class T>
void OnDemandSetT<T>::oSetPurgeable(bool deep,OFile *file)
// Set all OnDemands in the collection as purgeable.
{
	// for every element in the list
	typename OnDemandSetT<T>::iterator it;
	for(it = this->begin();it != this->end(); ++it)
	{
		// Set the OnDemand purgeable
		(*it).oSetPurgeable(deep,file);
	}
}



// Non typesafe set
typedef OnDemandSetT<OnDemand> OnDemandSet;


#endif
