#ifndef ONDEMAND_H
#define ONDEMAND_H
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


// OnDemandT is a template for a class that is used as a holder
// for objects that do not need to be in memory until accessed. 
// This can save memory and load time for an application. The
// cost for using OnDemandT rather than a pointer is an extra 4 bytes
// of memory.
//
// Like all ObjectFile objects it is safe to use OnDemandT for objects
// whether they are attached or not attached to the file, after taking
// the necassary precautions.
//
//	States:
//	  (i) id = 0   u._p points to a valid object (in or out of file), or
//    (ii)id > 0   u._file points to a valid OFile, and id contains an
//                 object identity.
//    (iii)id = 0 and _u.p/_u.file = 0   no object is held.
//
// In future this class will be made into a template wrapper, in order
// to avoid code bloat.
//
// If OF_REF_COUNT is defined, a reference to the object is held.


#include "ofile.h"
#include "opersist.h"

class OPersist;

template <class T>
class OnDemandT
{
public:
	// ObjectFile constructor
	OnDemandT(OIStream *in,const char *label = 0);

	// Default constructor
	OnDemandT(T *p = 0);

	// if OF_REF_COUNT is not defined then we can just use the
	// compiler generated versions of the following methods.
#ifdef OF_REF_COUNT
	// Copy constructor
	OnDemandT(const OnDemandT &);
	~OnDemandT(void);
	OnDemandT &operator =(const OnDemandT &);
#endif

	void Destroy(void);

	T *object(OClassId_t id = cOPersist)const;

	// Call private function to get round MSVC compiler bug.
	T *set(T *p){return privateSet(p);}


	T *	operator ->(void)const
	// Return the object. asserts if there is'nt one.
	{
		T *p = object();
		oFAssert(p);
		return p;
	}

	OId oId(void)const;

	T &operator *()const
	// Dereference the object
	{
		T *p = object();
		oFAssert(p);
		return *p;
	}

	bool oAttached(void)const;

	void purge(OFile *file = 0)const;

	void oSetPurgeable(bool deep = true,OFile *file = 0)const;

	void oWrite(OOStream *out,const char *label = 0)const
	{
		out->writeObjectId(oId(),label);
	}

	void oAttach(OFile *file,bool deep)const;
	void oDetach(OFile *file,bool deep)const;

private:
	T *privateSet(T *p);
	
protected:
	OId _id;         // id of unresolved object
	union u_tag{
		OFile *file; // pointer to OFile if  id != 0
		T *p;		 // pointer to object if id == 0
	}_u;
};		


template <class T>
OnDemandT<T>::OnDemandT(OIStream *in,const char *label)
// ObjectFile constructor
{
	if(in->file())
	{	// reading from an OFile
		_id = in->readObjectId();
		_u.file = _id ? in->file() : 0;
	}
	else
	{
		// reading from another stream
		_id = 0;
		_u.p = (T *)in->readObject(label);
	}
}

template <class T>
OnDemandT<T>::OnDemandT(T *p):_id(0)
// Default constructor
{
	_u.p = p;
}

template <class T>
void OnDemandT<T>::Destroy(void)
// Remember not to destroy objects that are persistant.
{
	// If resolved
	if(!_id)
		delete _u.p;
}

#ifdef OF_REF_COUNT

template <class T>
OnDemandT<T>::OnDemandT(const OnDemandT<T> &od)
// Copy constructor
// Always leave the copy in the unresolved state, to ensure
// that a persistent reference is added for it.
{
	_id = od._id;
	_u.file = od._u.file;
	
	// If only the pointer is copied we must add a reference, because
	// it will be removed by the destructor.
	if(!_id && _u.p)
		_u.p->addRef();
}


template <class T>
OnDemandT<T>::~OnDemandT(void)
{
	// If resolved
	if(!_id && _u.p)
		_u.p->removeRef();
}
		

template <class T>
OnDemandT<T> &OnDemandT<T>::operator =(const OnDemandT<T> &od)
// Always leave the copy in the unresolved state, to ensure
// that a persistent reference is added for it.
{
	// Get rid of the reference to the current object
	if(!_id && _u.p)
		_u.p->removeRef();

	_id = od._id;
	_u.file = od._u.file;
	
	// If only the pointer is copied we must add a reference, because
	// it will be removed by the destructor.
	if(!_id && _u.p)
		_u.p->addRef();

	return *this;
}
#endif // OF_REF_COUNT

template <class T>
T *OnDemandT<T>::object(OClassId_t id)const
// Return the object. 0 is returned if there is'nt one.
// A single reference only is held by OnDemand, so you can
// call this method many times without adding references.
{
	if(_id)
	{
		// u.file is a valid file
		((OnDemandT<T> *)this)->_u.p = (T *)_u.file->getObject(_id,id);
		oFAssert(_u.p);
		((OnDemandT<T> *)this)->_id = 0;
	}
	return _u.p;
}

template <class T>
T *OnDemandT<T>::privateSet(T *p)
// Set object and return the previous object.
{
	T *previous = object();
	_u.p = p;
#ifdef OF_REF_COUNT
	// Now we hold a reference to p.
	p->addRef();
#endif
	return previous;
}

template <class T>
bool OnDemandT<T>::oAttached(void)const
// Return true if the held object is attached to a file.Does not
// fetch the object into memory
{
	return _id ? true : (_u.p ? _u.p->oAttached() : false);
}


template <class T>
OId OnDemandT<T>::oId(void)const
// Return identity, without fetching the object into memory.
{
	if(_id)
		return _id;
	else
		return _u.p ? _u.p->oId() : 0;
}

template <class T>
void OnDemandT<T>::purge(OFile *file)const
// Purge the memory reference. Leaves a persistant reference.
// This does not effect the object in any way. i.e it does not
// make it purgeable.
// Parameter: file - The file that contains the object. 
// If zero then evaluate it.(default is 0)
{
	// If resolved
	if(!_id && _u.p)
	{
		// purge the object from this handle. Cast away the const.
		((OnDemandT<T> *)this)->_id = _u.p->oId();
		((OnDemandT<T> *)this)->_u.file = file ? file : OFile::oFileOf(_u.p);
	}
}



template <class T>
void OnDemandT<T>::oSetPurgeable(bool deep,OFile *file)const
// Set the held object as purgeable.
// Parameter: file - The file that contains the object. 
// If zero then evaluate it.(default is 0)
// purgeable - true(default) to set purgeable.
{
	// If resolved
	if(!_id && _u.p)
	{
		// Object must be attached to a file to be made purgeable.
		oFAssert(_u.p->oAttached());

		if(deep)
			_u.p->oSetPurgeable(deep,file);
#ifdef OF_REF_COUNT
		else
			_u.p->removeRef();
#endif
		purge(file);
	}
}

template <class T>
void OnDemandT<T>::oAttach(OFile *file,bool deep)const
{
	file->attach(_u.p,deep);
}

template <class T>
void OnDemandT<T>::oDetach(OFile *file,bool deep)const
{
	file->detach(object(),deep);
}


// A general non-type-safe OnDemandT instantiation.
typedef OnDemandT<OPersist> OnDemand;

#endif
