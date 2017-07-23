#ifndef TRANS_H
#define TRANS_H
			 
#include "odemand.h"
#include "opersist.h"
#include "oiter.h"
#include "ofstring.h"

const OClassId_t cTransport = 20;
const OClassId_t cPetrol    = 21;
const OClassId_t cElectric  = 22;
const OClassId_t cHybrid    = 23;

class Transport : public OPersist
{
typedef OPersist inherited;
public:
	Transport(OIStream *c);
	Transport(const char *model,long range,short passengers,long cost):
		_model(model),_range(range),_numPassengers(passengers),_cost(cost){}

	const char *modelName(void){return _model.c_str();}
	OMeta *meta(void)const{return &_metaClass;}

protected:
	void oWrite(OOStream *)const;
private:
	// Instantiation function
	static OPersist *New(OIStream *s){return new Transport(s);}

	static OMeta _metaClass;

	// Persistant data
	ofile_string _model;
	long _range;
	long _cost;
	short _numPassengers;
};


class Petrol : public virtual Transport
{
typedef Transport inherited;
public:
	Petrol(OIStream *c);
	Petrol(const char *model,long range,short passengers,long cost,float consumption):
		Transport(model,range,passengers,cost),_petrolConsumption(consumption){}

	OMeta *meta(void)const{return &_metaClass;}

protected:
	void oWrite(OOStream *)const;
private:
	// Instantiation function
	static OPersist *New(OIStream *s){return new Petrol(s);}

	static OMeta _metaClass;

	// Persistant data
	float _petrolConsumption;
};


class Electric : public virtual Transport
{
typedef Transport inherited;
public:
	Electric(OIStream *c);
	Electric(const char *model,long range,short passengers,long cost,float time):
		Transport(model,range,passengers,cost),_chargeTime(time){}

	float chargeTime(void){return _chargeTime;}

	OMeta *meta(void)const{return &_metaClass;}

	// Define iterator for this class
	typedef OIteratorT<Electric,cElectric> It;

protected:
	void oWrite(OOStream *)const;
private:
	// Instantiation function
	static OPersist *New(OIStream *s){return new Electric(s);}

	static OMeta _metaClass;

	// Persistant data
	float _chargeTime;
};



class Hybrid : public virtual Transport,public Petrol,public Electric
{
typedef Petrol inherited1;
typedef Electric inherited2;
public:
	Hybrid(OIStream *c);
	Hybrid(const char *model,long range,short passengers,long cost,
			float consumption,float time,float powerSaving):
		Petrol(model,range,passengers,cost,consumption),
		Electric(model,range,passengers,cost,time),
		Transport(model,range,passengers,cost),_powerSaving(powerSaving){}

	OMeta *meta(void)const{return &_metaClass;}

	// Define iterator for this class
	typedef OIteratorT<Hybrid,cHybrid> It;

protected:
	void oWrite(OOStream *)const;
private:
	// Instantiation function
	static OPersist *New(OIStream *s){return new Hybrid(s);}

	static OMeta _metaClass;

	// Persistant data
	float _powerSaving;
};


#endif
