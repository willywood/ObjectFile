//
//  Simple Command line benchmark program for ObjectFile.
//
// Note that this benchmark is far too simple to be representative
// of any real application. The results therefore have no real
// meaning.
//
//

#include "odefs.h"
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "ofile.h"
#include "oiter.h"
#include "ox.h"
#include "mmyclass.h"

#ifdef OFILE_STD_IN_NAMESPACE
using std::cout;
using std::cin;
#endif

#include "OBlobP.h"


class MyBlob : public OBlobP
{
public:
	MyBlob(OIStream *in):OBlobP(in){}
	MyBlob(oulong size):OBlobP(size){}

};


const OClassId_t cFill = 11;

class Fill: public OPersist
{
typedef OPersist inherited;
public:
	Fill(oulong size):_fill(size){}
	Fill(OIStream *in):_fill(in)
	{
	}
	char *getBlob(){_fill.oSetDirty();return _fill.getBlob();}
//	void setblob(char *data,oulong size){_fill.copyToBlob(data,size);}

protected:
	void oWrite(OOStream *out)const
	{
		inherited::oWrite(out);
		_fill.oWrite(out);
	}
	void oAttach(OFile *f,bool deep)
	{
		inherited::oAttach(f,deep);
		_fill.oAttach(f);
	}
	void oDetach(OFile *f,bool deep)
	{
		_fill.oDetach(f);
		inherited::oDetach(f,deep);
	}

private:
	// Instantiation function
	static OPersist *New(OIStream *s){return new Fill(s);}

	static OMeta _metaClass;
private:
	MyBlob _fill;
};


OMeta Fill::_metaClass(cFill,(Func)Fill::New,cOPersist,0);

//static char blobdata[1024*1024*1024/2] = {0};

int main()
{

	OFile *file = 0;
	int fileCount=0;
	int size = 1024*1024*1023/2;
	Fill *fill = 0;

	while(true)
	{
		
		char filename[30];
		sprintf(filename,"testdata%d.dat",fileCount);

		// Create a file if one is not open
		if(!file)
			file = new OFile(filename,OFILE_OPEN_FOR_WRITING|OFILE_CREATE,"ofbm");

		try
		{
			while(true)
			{
				fill = new Fill(size);
				memset(fill->getBlob(),0,size);
				file->attach(fill);
				fill->oSetPurgeable();
				time_t startTime = clock();
				cout << "Writing:"<< size << " - ";
				file->commit();
				cout << (clock() - startTime)/CLOCKS_PER_SEC << " seconds \n";
				file->purge();
			}
		}
		catch (OFileErr &err)
		{
			if(!strcmp(err.why(),"Attempt to increase the file beyond the maximum permitted."))
			{
				// Start a new file
				fileCount++;
				// Close the current file
				delete file;
				file = 0 ;
			}
			if(!strcmp(err.why(),"Failed to allocate space on the disk for the file."))
			{
				if(size != 1023)
				{
					// Dispose of the fill object
					file->detach(fill);
					delete fill;
					// Try a smaller blob size
					size = size/2;
				}else{
					delete file;
					file = 0 ;
					// Break out of main loop and terminate.
					break;
				}
			}
			cout << err.why() << '\n';
		}
	}
}

