#ifndef TEMPLAT_H
#define TEMPLAT_H

#include "opersist.h"
//#include "oiter.h"

// Best put in a header file along with all other class id's.
const ClassId cTemplateClass = <unique id (10 - 32000)>;

class TemplateClass : public OPersist
{
typedef OPersist inherited;
public:
	// Read constructor
	TemplateClass(OIStream *c);
	~TemplateClass(void){}
	// Other constructors
	TemplateClass(void){}

	// Remove comments if you need to override.
//	void oAttach(OFile *,bool);
//	void oDetach(OFile *,bool);
//	void oSetPurgeable(bool deep,OFile *file);

	OMeta *meta(void)const{return &_metaClass;}
	// Define iterator for this class(if you need one). Needs header oiter.h
//	typedef OIteratorT<TemplateClass,cTemplateClass> It;

protected:
	void oWrite(OOStream *)const;
private:
	// ObjectFile Instantiation function
	static OPersist *New(OIStream *s){return new TemplateClass(s);}

	static OMeta _metaClass;

	// Persistant data
};

#endif
