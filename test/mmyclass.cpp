#include "odefs.h"
#include <string.h>
#include "mmyclass.h"
#include "oistrm.h"
#include "ostrm.h"

#define DEFINE_CLASS(n) \
OMeta MyClass##n::_metaClass(n,(Func)MyClass##n::New,cOPersist,0); \
MyClass##n::MyClass##n(OIStream *in):OPersist(in)  \
{											 \
	_dum = in->readCString();				 \
}											 \
MyClass##n::MyClass##n(int nc)						 \
{											 \
int i;										 \
	_dum = new char[(size_t)nc+1];					 \
											 \
	for(i = 0; i < nc; i++)				 \
		_dum[i] = i % 10 + 48;				 \
											 \
	_dum[i] = '\0';							 \
}											 \
MyClass##n::~MyClass##n(void)						 \
{ delete _dum;}								 \
											 \
void MyClass##n::oWrite(OOStream *out)const	 \
{											 \
	inherited::oWrite(out);					 \
											 \
	out->writeCString(_dum);  \
}

DEFINE_CLASS(10)
DEFINE_CLASS(11)
DEFINE_CLASS(12)
DEFINE_CLASS(13)
DEFINE_CLASS(14)
DEFINE_CLASS(15)
DEFINE_CLASS(16)
DEFINE_CLASS(17)
DEFINE_CLASS(18)
DEFINE_CLASS(19)
DEFINE_CLASS(20)
DEFINE_CLASS(21)
DEFINE_CLASS(22)
DEFINE_CLASS(23)
DEFINE_CLASS(24)
DEFINE_CLASS(25)
DEFINE_CLASS(26)
DEFINE_CLASS(27)
DEFINE_CLASS(28)
DEFINE_CLASS(29)
DEFINE_CLASS(30)
DEFINE_CLASS(31)
DEFINE_CLASS(32)
DEFINE_CLASS(33)
DEFINE_CLASS(34)
DEFINE_CLASS(35)
DEFINE_CLASS(36)
DEFINE_CLASS(37)
DEFINE_CLASS(38)
DEFINE_CLASS(39)
DEFINE_CLASS(40)
DEFINE_CLASS(41)
DEFINE_CLASS(42)
DEFINE_CLASS(43)
DEFINE_CLASS(44)
DEFINE_CLASS(45)
DEFINE_CLASS(46)
DEFINE_CLASS(47)
DEFINE_CLASS(48)
DEFINE_CLASS(49)
DEFINE_CLASS(50)
DEFINE_CLASS(51)
DEFINE_CLASS(52)
DEFINE_CLASS(53)
DEFINE_CLASS(54)
DEFINE_CLASS(55)
DEFINE_CLASS(56)
DEFINE_CLASS(57)
DEFINE_CLASS(58)
DEFINE_CLASS(59)
DEFINE_CLASS(60)
DEFINE_CLASS(61)
DEFINE_CLASS(62)
DEFINE_CLASS(63)
DEFINE_CLASS(64)
DEFINE_CLASS(65)
DEFINE_CLASS(66)
DEFINE_CLASS(67)
DEFINE_CLASS(68)
DEFINE_CLASS(69)
DEFINE_CLASS(70)
DEFINE_CLASS(71)
DEFINE_CLASS(72)
DEFINE_CLASS(73)
DEFINE_CLASS(74)
DEFINE_CLASS(75)
DEFINE_CLASS(76)
DEFINE_CLASS(77)
DEFINE_CLASS(78)
DEFINE_CLASS(79)
DEFINE_CLASS(80)
DEFINE_CLASS(81)
DEFINE_CLASS(82) 
