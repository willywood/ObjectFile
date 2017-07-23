#include "odefs.h"
#include "evperson.h"


OMeta EvPerson::_metaClass(cEvPerson,(Func)EvPerson::New,cOPersist,0);


EvPerson::EvPerson(OIStream *in):OPersist(in)
{
	_name = in->readCString256();
	_weight = in->readFloat();

}


void EvPerson::oWrite(OOStream *out)const
// Write persistant data to the stream. This should always be overidden
// when there is persistant data. The first method called should be the inherited
// oWrite().
{
	inherited::oWrite(out);

	out->writeCString256(_name.c_str());
	out->writeFloat(_weight);

}
