//
// Transport and its sub-classes demonstrates the ability in ObjectFile
// to handle multiple inheritance and virtual base classes. 
// Construction from the input stream is handled beautifully by the C++ 
// constructors.
//
// 	The only extra work over and above what C++ requires is as follows:
//
// Virtual base class:
// Use the VBWrite() function in the oWrite method of the virtual bases 
// sub-classes to determine when to call the inherited oWrite.
//
// Multiple inheritance:
// 	In oWrite of the sub-class that multiply inherits, call ALL inherited
// oWrite methods.
//

#include "odefs.h"
//#include "cstring.h"
#include "trans.h"


//======================= P E R S O N =====================================

OMeta Transport::_metaClass(cTransport,(Func)Transport::New,cOPersist,0);
OMeta Petrol::_metaClass(cPetrol,(Func)Petrol::New,cTransport,0);
OMeta Electric::_metaClass(cElectric,(Func)Electric::New,cTransport,0);

// Multiply inherits from Electric and Petrol
OMeta Hybrid::_metaClass(cHybrid,(Func)Hybrid::New,cElectric,cPetrol,0);

//////////////////////////////////////////////////////////////////////////

Transport::Transport(OIStream *in):OPersist(in)
{
	_model = in->readCString256();
	_range = in->readLong();
	_cost = in->readLong();
	_numPassengers = in->readShort();
}

void Transport::oWrite(OOStream *out)const
// Write persistant data to the stream. This should always be overidden
// when there is persistant data. The first method called should be the inherited
// oWrite().
{
	inherited::oWrite(out);

	out->writeCString256(_model.c_str(),"_model");
	out->writeLong(_range,"_range");
	out->writeLong(_cost,"_cost");
	out->writeShort(_numPassengers,"_numPassengers");
}

//////////////////////////////////////////////////////////////////////////

Petrol::Petrol(OIStream *in):Transport(in)
{
	_petrolConsumption = in->readFloat();
}

void Petrol::oWrite(OOStream *out)const
// Write persistant data to the stream. This should always be overidden
// when there is persistant data. The first method called should be the inherited
// oWrite().
{
	// Check whether to write the virtual base 
	if(out->VBWrite())
		inherited::oWrite(out);

	out->writeFloat(_petrolConsumption,"_petrolConsumption");
}

//////////////////////////////////////////////////////////////////////////


Electric::Electric(OIStream *in):Transport(in)
{
	_chargeTime = in->readFloat();
}


void Electric::oWrite(OOStream *out)const
// Write persistant data to the stream. This should always be overidden
// when there is persistant data. The first method called should be the inherited
// oWrite().
{
	// Check whether to write the virtual base 
	if(out->VBWrite())
		inherited::oWrite(out);

	out->writeFloat(_chargeTime,"_chargeTime");
}

//////////////////////////////////////////////////////////////////////////

Hybrid::Hybrid(OIStream *in):Transport(in),inherited1(in),inherited2(in)
{
	_powerSaving = in->readFloat();
}


void Hybrid::oWrite(OOStream *out)const
// Write persistant data to the stream. This should always be overidden
// when there is persistant data. The first method called should be the inherited
// oWrite().
{
	// Write both superclasses
	inherited1::oWrite(out);
	inherited2::oWrite(out);

	out->writeFloat(_powerSaving,"_powerSaving");
}


