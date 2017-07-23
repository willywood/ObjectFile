#ifndef PERSON_H
#define PERSON_H

#include "odemand.h"
#include "opersist.h"
#include "oiter.h"
#include <iostream>
#include "ofstring.h"

const OClassId_t cWristWatch = 11;

class WristWatch : public OPersist
{
typedef OPersist inherited;
public:
	WristWatch(OIStream *c);
	WristWatch(const char *model):_model(model){}
	~WristWatch();

	const char *modelName(void){return _model.c_str();}
	OMeta *meta(void)const{return &_metaClass;}


protected:
	void oWrite(OOStream *)const;
private:
	// Instantiation function
	static OPersist *New(OIStream *s){return new WristWatch(s);}

	static OMeta _metaClass;

	// Persistant data
	ofile_string _model;
};


const OClassId_t cPerson = 10;

class Person : public OPersist
{
typedef OPersist inherited;
public:
	class Name
	{
	public:
		ofile_string _first;
		ofile_string _family;
	};

	Person(OIStream *c);
	~Person(void);
	Person(const char *fname,const char *sname,const char *street,
	       const char *district,const char *city,const char *country,
		   long zip,double weight,float height,
		   int age,Person *spouse,WristWatch *watch):
			   					  _street(street),
		   						  _district(district),
		   						  _city(city),
			   					  _country(country),
		   						  _zipCode(zip),
			   					  _weight(weight),
		   						  _height(height),
		   						  _age(age),
								  _carOwner(true),
								  _flags(65000),
								  _bitFlags(215),
		   						  _spouse(spouse),
		   						  _watch(watch)
		   						  {_name._first = fname;_name._family=sname;}



	void oAttach(OFile *,bool);
	void oDetach(OFile *,bool);
	void oSetPurgeable(bool deep = true,OFile *file = 0);

	OMeta *meta(void)const{return &_metaClass;}
	// Define iterator for this class
	typedef OIteratorT<Person,cPerson> It;

	Person *setSpouse(Person *newSpouse);
	Person *spouse(void){return _spouse.object();}
	const char * watch(void){return _watch->modelName();}
	void oPrint(std::ostream &out)const;
protected:
	void oWrite(OOStream *)const;
private:
	// ObjectFile Instantiation function
	static OPersist *New(OIStream *s){return new Person(s);}

	static OMeta _metaClass;

	// Persistant data
	Name _name;
	ofile_string _street;
	ofile_string _district;
	ofile_string _city;
	ofile_string _country;
	long _zipCode;
	double  _weight;
	float _height;
	int _age;
	bool _carOwner;
	unsigned short _flags;
	unsigned short _bitFlags;
	OnDemandT<Person> _spouse;      // Read into memory when and if needed.
	OnDemandT<WristWatch> _watch;   // Read into memory when and if needed.
};

#endif
