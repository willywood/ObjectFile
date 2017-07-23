#include "odefs.h"
#include "templat.h"


OMeta TemplateClass::_metaClass(cTemplateClass,(Func)TemplateClass::New,cOPersist,0);


TemplateClass::TemplateClass(OIStream *in):OPersist(in)
// Read constructor
{

}


void TemplateClass::oWrite(OOStream *out)const
// Write persistant data to the stream. This should always be overidden
// when there is persistant data. The first method called should be the inherited
// oWrite().
{
	inherited::oWrite(out);

}

/*
void TemplateClass::oAttach(OFile *file,bool deep)
// Attach this object to the file. That is, make the object persistant.
// This should be overridden if the object manages the persistance of
// other objects derived from OPersist.
{
	inherited::oAttach(file,deep);
}

void TemplateClass::oDetach(OFile *file,bool deep)
// Detach this object from the file. That is, make the object non-persistant.
// This should be overridden if the object manages the persistance of
// other objects derived from OPersist.
{
	inherited::oDetach(file,deep);
}
void TemplateClass::oSetPurgeable(bool deep,OFile *file)
// Set this object as purgeable. There should be no references to the object
// as it may be deleted. Derived classes may want to override this function
// in order to pass it on to owned objects.
// Derived classes should call the super class last.
// Parameters: deep - true if owned objects are to be set purgeable
//                    (default true)
//             file - the file containing the object.0 means evaluate it.
//                    (default 0) This is only important when using OnDemand.
{
}
*/