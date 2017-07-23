#ifndef MYHGB_H
#define MYHGB_H


#include "OHGBlob.h"


class MyBlob : public OHGBlob
{
public:
	MyBlob(OIStream *in);
	MyBlob(oulong size);

};

#endif