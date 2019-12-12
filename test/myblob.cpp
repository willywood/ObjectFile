#include "odefs.h"
#include "myblob.h"

#ifdef WIN16
#define O_HUGE _huge
#else
#define O_HUGE
#endif

char const cData[] = "1234567890";

MyBlob::MyBlob(oulong size):OBlob(size)
{
	char  O_HUGE *p = (char  *)ballocator.address(getBlob());
	// Add test data
	for(oulong i = 0; i < _blobLength;i++)
	{
		*p++ = cData[i % 10];
	}
}

MyBlob::MyBlob(OIStream *in):OBlob(in)
{
	// Access the blob via getBlob
	char  O_HUGE *p = (char  *)ballocator.address(getBlob());
	for(oulong i = 0; i < _blobLength;i++)
	{
		// Test the data
		oFAssert(*p == cData[i % 10]);
		p++;
	}
}


