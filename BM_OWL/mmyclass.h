#include "opersist.h"

#define DECLARE_CLASS(n)                                       \
class MyClass##n : public OPersist                                \
{                                                              \
typedef OPersist inherited;                                    \
public:                                                        \
	MyClass##n (OIStream *c);                                       \
	~MyClass##n (void);                                             \
	MyClass##n (int);                                             \
	OMeta *meta(void)const{return &_metaClass;}                 \
protected:                                                     \
	void oWrite(OOStream *)const;                               \
private:                                                       \
	static OPersist *New(OIStream *s){return new MyClass##n (s);}  \
	static OMeta _metaClass;                                    \
	char *_dum;                                              \
};                                                              

DECLARE_CLASS(10)
DECLARE_CLASS(11)
DECLARE_CLASS(12)
DECLARE_CLASS(13)
DECLARE_CLASS(14)
DECLARE_CLASS(15)
DECLARE_CLASS(16)
DECLARE_CLASS(17)
DECLARE_CLASS(18)
DECLARE_CLASS(19)
DECLARE_CLASS(20)
DECLARE_CLASS(21)
DECLARE_CLASS(22)
DECLARE_CLASS(23)
DECLARE_CLASS(24)
DECLARE_CLASS(25)
DECLARE_CLASS(26)
DECLARE_CLASS(27)
DECLARE_CLASS(28)
DECLARE_CLASS(29)
DECLARE_CLASS(30)
DECLARE_CLASS(31)
DECLARE_CLASS(32)
DECLARE_CLASS(33)
DECLARE_CLASS(34)
DECLARE_CLASS(35)
DECLARE_CLASS(36)
DECLARE_CLASS(37)
DECLARE_CLASS(38)
DECLARE_CLASS(39)
DECLARE_CLASS(40)
DECLARE_CLASS(41)
DECLARE_CLASS(42)
DECLARE_CLASS(43)
DECLARE_CLASS(44)
DECLARE_CLASS(45)
DECLARE_CLASS(46)
DECLARE_CLASS(47)
DECLARE_CLASS(48)
DECLARE_CLASS(49)
DECLARE_CLASS(50)
DECLARE_CLASS(51)
DECLARE_CLASS(52)
DECLARE_CLASS(53)
DECLARE_CLASS(54)
DECLARE_CLASS(55)
DECLARE_CLASS(56)
DECLARE_CLASS(57)
DECLARE_CLASS(58)
DECLARE_CLASS(59)
DECLARE_CLASS(60)
DECLARE_CLASS(61)
DECLARE_CLASS(62)
DECLARE_CLASS(63)
DECLARE_CLASS(64)
DECLARE_CLASS(65)
/*
DECLARE_CLASS(66)
DECLARE_CLASS(67)
DECLARE_CLASS(68)
DECLARE_CLASS(69)
DECLARE_CLASS(70)
DECLARE_CLASS(71)
DECLARE_CLASS(72)
DECLARE_CLASS(73)
DECLARE_CLASS(74)
DECLARE_CLASS(75)
DECLARE_CLASS(76)
DECLARE_CLASS(77)
DECLARE_CLASS(78)
DECLARE_CLASS(79)
DECLARE_CLASS(80)
DECLARE_CLASS(81)
DECLARE_CLASS(82)
*/
