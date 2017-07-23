#ifndef EVPERSON_H
#define EVPERSON_H


#include "ofstring.h"
#include "opersist.h"
#include "oiter.h"

// Best put in a header file along with all other class id's.
const OClassId_t cEvPerson = 10;

class EvPerson : public OPersist
{
typedef OPersist inherited;
public:
	// Read constructor
	EvPerson(OIStream *c);
	~EvPerson(void){}
	// Other constructors
	EvPerson(void):_name("Joe"),_weight(70.0),_height(180.0){}

	OMeta *meta(void)const{return &_metaClass;}
	// Define iterator for this class(if you need one)
	typedef OIteratorT<EvPerson,cEvPerson> It;

protected:
	void oWrite(OOStream *)const;
private:
	// ObjectFile Instantiation function
	static OPersist *New(OIStream *s){return new EvPerson(s);}

	static OMeta _metaClass;

	// Persistant data
public:
	ofile_string _name;
	float _weight;
	float _height;
};

#endif
