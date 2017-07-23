//
// This shows just one way of making a persistant collection. It takes advantage
// of an existing STL collection, by inheriting from it, as well as OPersist.
// In this case the collection holds references to persistant objects. 
// If the oDetach and oAttach methods were not overridden MyCollect
// would  not manage the lifecycle of the objects it holds, i.e. removing 
// the collection from the file would not remove the objects it holds from 
// the file.
//	  Because they ARE overridden, if we detach the collection from the
// file with 'deep = true' then the objects its holds will also be removed
// from the file.
// 	  Note that oDetach does not remove objects from the collection. This
// ensures oAttach can add the objects to the file again.
//
// Using MyCollect.
// 	  All objects must either be in the file or not in the file.
//		

#include "odefs.h"
#include "mycolct.h"
#include "oistrm.h"
#include "ostrm.h"

OMeta MyCollect::_metaClass(cMyCollect,(Func)MyCollect::New,cOPersist,0);

MyCollect::MyCollect(OIStream *in):OPersist(in)
// Object file read constructor
{
	// number of elements in list
	long size = in->readLong();

	for(long i = 0; i < size;i++)
		// add to list
		push_back(in->readObject());

	// Since it is impractical to override every function that modifies
	// the collection, we always make it dirty. Another alternative
	// would be for the callers to explicitly call oSetDirty() when
	// modifying the collection.
	oSetDirty();
}

MyCollect::~MyCollect(void)
{
	if(!oAttached()){
		// delete only if object is not persistant, since it is forbidden to
		// directly delete a persistant object.
		for(MyCollect::iterator it = begin();it != end(); ++it)
			delete *it;
	}
}

void MyCollect::oWrite(OOStream *out)const
{
	inherited::oWrite(out);

	// number of elements in list
	out->writeLong(size(),"size");

	// for every element in the list
	for(MyCollect::const_iterator it = begin();it != end(); ++it)
		// Write out the element
		out->writeObject(*it,"item");

}

void MyCollect::oAttach(OFile *file,bool deep)
{
	oFAssert(!oAttached());

	inherited::oAttach(file,deep);

	if(deep){
		// for every element in the list
		for(MyCollect::iterator it = begin();it != end(); ++it)
			// Insert object into the file
			(*it)->oAttach(file,deep);
	}
}

void MyCollect::oDetach(OFile *file,bool deep)
{
	oFAssert(oAttached());

	if(deep){
		// for every element in the list
		for(MyCollect::iterator it = begin();it != end(); ++it)
			// Erase from file but not from this collection
			(*it)->oDetach(file,deep);
	}
	inherited::oDetach(file,deep);
}



//===================== Streamable collection ==================

// This is an example of a streamable collection that can be used to hold a
// list of objects.
// By giving it a destroy function instead of a destructor, the caller
// can decide whether or ot he will own the objects in the collection
// or just point to them.


MySCollect::MySCollect(OIStream *in)
// Object file read constructor
{
	// number of elements in list
	long size = in->readLong();

	for(long i = 0; i < size;i++)
		// add to list
		push_back(in->readObject());

}

void MySCollect::destroy(void)
// Call this function only if caller is not persistant, since it is forbidden to
// directly delete a persistant object.
{
	for(MySCollect::iterator it = begin();it != end(); ++it)
		delete *it;
}

void MySCollect::oWrite(OOStream *out)const
{
	// number of elements in list
	out->writeLong(size());

	// for every element in the list
	for(MySCollect::const_iterator it = begin();it != end(); ++it)
		// Write out the element
		out->writeObject(*it);

}

void MySCollect::oAttach(OFile *file,bool deep)
{
	if(deep){
		// for every element in the list
		for(MySCollect::iterator it = begin();it != end(); ++it)
			// Insert object into the file
			(*it)->oAttach(file,deep);
	}
}

void MySCollect::oDetach(OFile *file,bool deep)
{
	if(deep){
		// for every element in the list
		for(MySCollect::iterator it = begin();it != end(); ++it)
			// Erase from file but not from this collection
			(*it)->oDetach(file,deep);
	}
}
