#ifndef RELATION_H
#define RELATION_H

#include "ofstring.h"
#include "opersist.h"
#include "oiter.h"
#include "odemand.h"

// Best put in a header file along with all other class id's.
const OClassId_t cRelation = 20;

class Relation : public OPersist
{
typedef OPersist inherited;
public:
	// Read constructor
	Relation(OIStream *c);
	virtual ~Relation(void){}
	// Other constructors
	Relation(const char *name):_name(name){}

	void oSetPurgeable(bool deep,OFile *file);

	void setSpouse(Relation *p){_spouse.set(p);}
	Relation *spouse(void){return _spouse.object();}
	const char *name(void){return _name.c_str();}

	OMeta *meta(void)const{return &_metaClass;}
	// Define iterator for this class(if you need one). Needs header oiter.h
	typedef OIteratorT<Relation,cRelation> It;

protected:
	void oWrite(OOStream *)const;
private:
	// ObjectFile Instantiation function
	static OPersist *New(OIStream *s){return new Relation(s);}

	static OMeta _metaClass;

	// Persistant data
	ofile_string _name;
	OnDemandT<Relation> _spouse;
};

#endif
