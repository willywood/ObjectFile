#ifndef MYCLASS5_H
#define MYCLASS5_H

#include "opersist.h"
#include "odset.h"

const OClassId_t cMyClass5 =  15;

class MyClass5 : public OPersist
{
typedef OPersist inherited;
public:
	MyClass5(void):_children(),_houses(){}
	MyClass5(OIStream * c);
	~MyClass5(void){};

	OMeta *meta(void)const{return &_metaClass;}

	void oAttach(OFile *file,bool);
	void oDetach(OFile *file,bool);

	void oSetPurgeable(bool deep = true,OFile *file = 0);


	void addChild(const OnDemand &);
	void removeChild(const OnDemand &child);

	void addHouse(MyClass5 *);

	const OnDemandSet &children()const{return _children;}

protected:
	void oWrite(OOStream *)const;

private:
	// Instantiation function
	static OPersist *New(OIStream *s){return new MyClass5(s);}
	static OMeta _metaClass;

	OnDemandSet _children;	// Related only
	OnDemandSet _houses;    // Owned
};

#endif