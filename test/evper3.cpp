#include "odefs.h"
#include "evper3.h"


OMeta EvPerson::_metaClass(cEvPerson,(Func)EvPerson::New,cOPersist,0);


EvPerson::EvPerson(OIStream *in):OPersist(in)
{
	_name = in->readCString256();

	if( in->userVersion() < 3){
		in->readFloat();

		if( in->userVersion() > 1)
			in->readFloat();     // Read height only if file verion > 1
		oSetDirty();             // Ensure the object gets written
	}

}


void EvPerson::oWrite(OOStream *out)const
// Write persistant data to the stream. This should always be overidden
// when there is persistant data. The first method called should be the inherited
// oWrite().
{
	inherited::oWrite(out);

	out->writeCString256(_name.c_str(),"_name");

}
