//
// Person shows the power of variable length records. There is a lot of string data
// which in a fixed length scheme, would take up a lot of file space. In Object file
// only the bare minimum of file space is used.
//
// Also demonstrated is a 1-1 bidirectional relationship (_spouse).
// This is a case where an OnDemand is very useful. If we want to examine the weight
// of 500 randomly selected people, it is unnessasary to read their spouses into
// memory. The _spouse is therefore implemented as an OnDemand.

#include "odefs.h"
#include "ofstring.h"
#include "person.h"


//======================= P E R S O N =====================================

OMeta Person::_metaClass(cPerson,(Func)Person::New,cOPersist,0);


Person::Person(OIStream *in):OPersist(in),_spouse(in,"_spouse"),_watch(in,"_watch")
{
	{
		OIStream::ODefineObject n(in,"_name");
		_name._first = in->readCString256("_first");
		_name._family = in->readCString256("_family");
	}
	_street = in->readCString256("_street");
	_district = in->readCString256("_district");
	_city = in->readCString256("_city");
	_country = in->readCString256("_country");
	_zipCode = in->readLong("_zipCode");
	_weight = in->readDouble("_weight");
	_height = in->readFloat("_height");
	_age = in->readShort("_age");
	_carOwner = in->readBool("_carOwner");
	in->readBytes(&_flags,2,"_flags");
	in->readBits(&_bitFlags,2,"_bitFlags");

}

Person::~Person(void)
{
	// If the object is not in the file, destroy owned persistant objects.
	if(!oAttached())
		_watch.Destroy();
}

void Person::oWrite(OOStream *out)const
// Write persistant data to the stream. This should always be overidden
// when there is persistant data. The first method called should be the inherited
// oWrite().
{
	inherited::oWrite(out);

	_spouse.oWrite(out,"_spouse");
	_watch.oWrite(out,"_watch");
	{
		OOStream::ODefineObject n(out,"_name");
		out->writeCString256(_name._first.c_str(),"_first");
		out->writeCString256(_name._family.c_str(),"_family");
	}
	out->writeCString256(_street.c_str(),"_street");
	out->writeCString256(_district.c_str(),"_district");
	out->writeCString256(_city.c_str(),"_city");
	out->writeCString256(_country.c_str(),"_country");
	out->writeLong(_zipCode,"_zipCode");
	out->writeDouble(_weight,"_weight");
	out->writeFloat(_height,"_height");
	out->writeShort(_age,"_age");
	out->writeBool(_carOwner,"_carOwner");
	out->writeBytes(&_flags,2,"_flags");
	out->writeBits(&_bitFlags,2,"_bitFlags");

}

void Person::oAttach(OFile *file,bool deep)
// Insert this object into the file. That is, make the object persistant.
// This should be overridden if the object manages the persistance of
// other objects derived from OPersist.
{
	inherited::oAttach(file,deep);

	if(deep)
		_watch.oAttach(file,true);
}

void Person::oDetach(OFile *file,bool deep)
// Erase this object from the file. That is, make the object non-persistant.
// This should be overridden if the object manages the persistance of
// other objects derived from OPersist.
{
	if(deep)
		_watch.oDetach(file,true);

	inherited::oDetach(file,deep);
}

void Person::oSetPurgeable(bool deep,OFile *file)
// Set this object as purgeable. There should be no references to the object
// as it may be deleted.
{
	// Purge relationship and object (if deep)
	_watch.oSetPurgeable(deep,file);

    // Purge relationship only
    _spouse.oSetPurgeable(false,file);

	inherited::oSetPurgeable(deep,file);
}

Person *Person::setSpouse(Person *newSpouse)
// Set the persons spouse to newSpouse.
// Return the previous spouse.
{
	// Set the Person dirty because _spouse does not do it.
	oSetDirty();
	return _spouse.set(newSpouse);
}


void Person::oPrint(std::ostream &out)const
{
	out << oId() << '.' << _name._first.c_str() << ' ' << _name._family.c_str() << ",\n"
		<< _street.c_str() << ',' << _district.c_str() << ',' << _city.c_str() << ",\n"
		<< _country.c_str() << ",\nSpouse:"
		<< (_spouse.object() ? _spouse->_name._first.c_str() : "none") <<"\n\n";
}


//======================= W A T C H =====================================

// A very simple class.

OMeta WristWatch::_metaClass(cWristWatch,(Func)WristWatch::New,cOPersist,0);

WristWatch::WristWatch(OIStream *in):OPersist(in)
{
	_model = in->readCString256();
}

void WristWatch::oWrite(OOStream *out)const
{
	inherited::oWrite(out);

	out->writeCString256(_model.c_str(),"_model");
}

WristWatch::~WristWatch()
{}

