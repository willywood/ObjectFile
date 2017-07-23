#include "odefs.h"
#include "odrel.h"


OMeta Relation::_metaClass(cRelation,(Func)Relation::New,cOPersist,0);


Relation::Relation(OIStream *in):OPersist(in),_name(in->readCString256("_name")),
                                              _spouse(in,"_spouse")
// Read constructor
{
}


void Relation::oWrite(OOStream *out)const
// Write persistant data to the stream. This should always be overidden
// when there is persistant data. The first method called should be the inherited
// oWrite().
{
	inherited::oWrite(out);

    out->writeCString256(_name.c_str(),"_name");
	_spouse.oWrite(out,"_spouse");

}

void Relation::oSetPurgeable(bool deep,OFile *file)
{
	// Purge relationship only
	_spouse.oSetPurgeable(false,file);

	// inherited called last
	inherited::oSetPurgeable(deep);
}
