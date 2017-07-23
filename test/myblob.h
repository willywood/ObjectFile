#ifndef MYBLOB_H
#define MYBLOB_H


#include "oblob.h"


class MyBlob : public OBlob
{
public:
	MyBlob(OIStream *in);
	MyBlob(oulong size);

};

#endif
