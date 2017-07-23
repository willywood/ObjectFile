#include "odefs.h"
#include "myhgb.h"

#ifdef WIN16
#define HUGE _huge
#else
#define HUGE
#endif


MyBlob::MyBlob(oulong size):OHGBlob(size)
{
	char  HUGE *p = (char  *)allocator.address(getBlob());
	for(long i = 0; i < _blobLength;i++)
		*p++ = 'x';
}

MyBlob::MyBlob(OIStream *in):OHGBlob(in)
{
	// Access the blob via getBlob
	char  HUGE *p = (char  *)allocator.address(getBlob());
	for(long i = 0; i < _blobLength;i++)
		oFAssert(*p++ == 'x');
}


