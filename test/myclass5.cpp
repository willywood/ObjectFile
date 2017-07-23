

#include "odefs.h"
#include "myclass5.h"
#include "ostrm.h"

OMeta MyClass5::_metaClass(cMyClass5,(Func)MyClass5::New,cOPersist,0);


MyClass5::MyClass5(OIStream *in):OPersist(in),_children(in,"children"),_houses(in,"houses")
// Object file read constructor
{
}


void MyClass5::oWrite(OOStream *out)const
{
	inherited::oWrite(out);

	_children.oWrite(out,"children");
	_houses.oWrite(out,"houses");
}

void MyClass5::oAttach(OFile *file,bool deep)
{
	// inherited called first
	inherited::oAttach(file,deep);

	if(deep)
		_children.oAttach(file,deep);

	_houses.oAttach(file,deep);
}

void MyClass5::oDetach(OFile *file,bool deep)
{
	if(deep)
		_children.oDetach(file,deep);

	_houses.oDetach(file,deep);

	// inherited called last
	inherited::oDetach(file,deep);
}

void MyClass5::oSetPurgeable(bool deep,OFile *file)
{
	// Purge relationship only
	_children.oSetPurgeable(false,file);

	// Purge relationship and object (if deep)
	_houses.oSetPurgeable(deep,file);

	// inherited called last
	inherited::oSetPurgeable(deep,file);
}


void MyClass5::addChild(const OnDemand &child)
{
	_children.insert(child);
	oSetDirty();
}

void MyClass5::removeChild(const OnDemand &child)
{
	_children.erase(child);
	oSetDirty();
}

void MyClass5::addHouse(MyClass5 *house)
{
	_houses.insert(house);
	oSetDirty();
}
