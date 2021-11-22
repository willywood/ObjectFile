#ifndef OISTRM_H
#define OISTRM_H
/*=============================================================================
MIT License

Copyright(c) 2019 willywood

Permission is hereby granted, free of charge, to any person obtaining a copy
of this softwareand associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright noticeand this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
=============================================================================*/


///////////////////////////////////////////////////////////////////////////
// OIStream is an abstract class for reading object data.
// It can be sub-classed to receive data from or across any medium.
///////////////////////////////////////////////////////////////////////////



#include <deque>
#include <vector>
#include <stack>
//#include "obuf.h"
#include "oio.h"

#ifdef OFILE_STD_IN_NAMESPACE
using std::vector;
using std::stack;
using std::deque;
#endif


class OFile;

//typedef long O_LONG;   // Four bytes
//typedef short O_SHORT; // Two bytes



class OIStream{
public:
	// This class can be used to ensure that an object definition is terminated.
	class ODefineObject
	{
	public:
		ODefineObject(OIStream *in,const char *label = 0):_in(in)
		{
			_in->beginObject(label);
		}
		~ODefineObject()
		{
			_in->endObject();
		}
	private:
		OIStream *_in;
	};
	//OIBuffer is used to buffer the input.
	class OIBuffer{
	public:
		OIBuffer(void):_start(_bufp),_dataLength(0){}
		~OIBuffer(void){}

		void *read(long size){OFILE_UNUSED(size); return _bufp;}
		long read(void *buf,oulong size);
		void *set(long);
		long bufferSize(void)const{return sizeof(_bufp);}
		long toRead(void)const{return _dataLength - (long)(_start - _bufp);}

	private:
		char *_start;     // start of data
		char _bufp[2048]; // data buffer.
		long _dataLength;
	};

	OIStream(){}
	~OIStream(void){}

	// These methods have to be public because any class that is streamed (not just decendants
	// of OPersist) can call them. However derived streams can make them private.
	virtual O_LONG readLong(const char *label = 0) = 0;
	virtual O_LONG64 readLong64(const char *label = 0) = 0;
	virtual float readFloat(const char *label = 0) = 0;
	virtual double readDouble(const char *label = 0) = 0;
	virtual O_SHORT readShort(const char *label = 0) = 0;
	virtual char readChar(const char *label = 0) = 0;
	virtual bool readBool(const char *label = 0) = 0;
	virtual void readCString(char * str,unsigned int maxlen,const char *label = 0) = 0;
	virtual char *readCString256(const char *label = 0) = 0;
	virtual char * readCString(const char *label = 0) = 0;
	virtual char * readCStringD(const char *label = 0) = 0;
	// wchar support  can be removed if not used.
	virtual O_WCHAR_T readWChar(const char *label = 0) = 0;
	virtual void readWCString(O_WCHAR_T * str,unsigned int maxlen,const char *label = 0) = 0;
	virtual O_WCHAR_T *readWCString256(const char *label = 0) = 0;
	virtual O_WCHAR_T * readWCString(const char *label = 0) = 0;
	virtual O_WCHAR_T * readWCStringD(const char *label = 0) = 0;
	//
	virtual void readBytes(void *buf,int nBytes,const char *label = 0) = 0;
	virtual void readBits(void *buf,int nBytes,const char *label = 0) = 0;
	virtual OId readObjectId(const char *label = 0) = 0;
	virtual void readObject(OPersist **obp,const char *label = 0) = 0;
	virtual OPersist *readObject(const char *label = 0) = 0;
	virtual void readBlob(void *buf,OFilePos_t mark,unsigned long size) = 0;
	
	virtual OFile *readBlobHeader(OFilePos_t *mark,oulong *blobLength,
								oulong *fileLength,const char *label = 0) = 0;
	// Version control methods. These methods are virtual so that they can be used
	// via the virtual table. They should not be overridden.
	// Return the version of this file.
	virtual long userVersion(void)const = 0;
	// Return the version of the source code.
	virtual long userSourceVersion(void)const = 0;
	// Set the currently being read object to ob. This allows any references
	// to the object to be resolved even though it is still being read.
	// Called before any of the object is read.
	virtual void setCurrentObject(OPersist *ob) = 0;
	virtual void beginObject(const char *label) = 0;
	virtual void endObject(void) = 0;

	// Do not need to override. Return 0 indicates not reading from an OFile.
	// Used by ODemandT
	virtual OFile *file(void)const{return 0;}
};

///////////////////////////////////////////////////////////////////////////
// OIStreamFile is a concrete subclass of OIStream. It is responsible for
// reading objects from a binary file.
// For efficiency it reads a whole object into a buffer at a time. 
// Buffering commences 
// with	the method start() and finishes with the method finish().
///////////////////////////////////////////////////////////////////////////

class OIStreamFile: public OIStream {

struct OSmartPtr
{
	OId _id;
	OPersist **_obp;
};

typedef deque<OSmartPtr> OSmartPtrs;

class StrmInfo{
public:
	StrmInfo():_mark(0), _toRead(0) {}
	~StrmInfo(){}
	StrmInfo(OFilePos_t mark,long toRead):_mark(mark),_toRead(toRead){}
	OSmartPtrs _sp;
	OFilePos_t    _mark;
	long      _toRead;

	// These are needed for some STL vector implementations
    int operator < (const StrmInfo &si) const {
       OFILE_UNUSED(si);
       return 0;
    }
    int operator ==(const StrmInfo &si) const {
       OFILE_UNUSED(si);
       return 0;
    }
 };


// Standard definition for stack
typedef stack<StrmInfo,vector<StrmInfo> > OSPtrStack;
// HP definition for stack
//typedef stack<vector<StrmInfo> > OSPtrStack;

public:
	OIStreamFile(OFile *file,const char* fname,long operation);
#ifdef OF_OLE
	OIStreamFile(OFile *file,IStorage *istorage,const char* fname,unsigned long istorage_mode);
#endif
	OIStreamFile(OFile *file);
	~OIStreamFile();

	O_LONG readLong(const char *label = 0);
	O_LONG64 readLong64(const char *label = 0);
	OFilePos_t readFilePos(const char *label = 0);
	float readFloat(const char *label = 0);
	double readDouble(const char *label = 0);
	O_SHORT readShort(const char *label = 0);
	char readChar(const char *label = 0);
	bool readBool(const char *label = 0);
	void readCString(char * str,unsigned int maxlen,const char *label = 0);
	char *readCString256(const char *label = 0);
	char * readCString(const char *label = 0);
	char * readCStringD(const char *label = 0);
	// wchar support  can be removed if not used.
	O_WCHAR_T readWChar(const char *label = 0);
	void readWCString(O_WCHAR_T * str,unsigned int maxlen,const char *label = 0);
	O_WCHAR_T *readWCString256(const char *label = 0);
	O_WCHAR_T * readWCString(const char *label = 0);
	O_WCHAR_T * readWCStringD(const char *label = 0);
	//
	void readBytes(void *buf,int nBytes,const char *label = 0);
	void readBits(void *buf,int nBytes,const char *label = 0);
	OId readObjectId(const char *label = 0);
	void readObject(OPersist **obp,const char *label = 0);
	OPersist *readObject(const char *label = 0);
	void readBlob(void *buf,OFilePos_t mark,unsigned long size);
	OFile *readBlobHeader(OFilePos_t *mark,oulong *blobLength,
								oulong *fileLength,const char *label = 0);


	O_fd *fd(void){return &_fd;}
	void close(void);
	void open(const char *fname,long operation);
	OFilePos_t fileLength(){ return o_fileLength(_fd);}

	void start(OFilePos_t mark,long size);
	void finish(void);
	void abort(void);

	void commit(long mark);

	void readDataAt(OFilePos_t mark,void *buf,unsigned long size);
	void readData(void *buf,size_t size);

	// Return the version of this file.
	long userVersion(void)const;
	// Return the version of the source code.
	long userSourceVersion(void)const;

	void setCurrentObject(OPersist *p);
	void beginObject(const char *label){OFILE_UNUSED(label);}
	void endObject(void){}

	OFile *file(void)const{return _file;}

protected:
	OFile *_file;

private:
	OSPtrStack _readObjects;
	OIBuffer _ostr;
	O_fd _fd;
	OFilePos_t _mark;
	long _toRead;
	bool _ownsFile;	  // Stream owns the file stream
	bool _fileOpen;   // State of file stream
	char *_returnString;
	O_WCHAR_T *_wreturnString;
#ifndef OF_MULTI_THREAD
	// There can never be two readfunctions running simulultaneously,so
	// save space by making all the streams share the same return buffer
	static 
#endif
	union StrBuffT			  // Return buffer for reading strings
	{
		O_WCHAR_T wstr[256];
		char str[256];
	}_strBuffer;      

};


// This is a purely functional class. It encapsulates functions that are 
// used by ObjectFile. 
class OUtilityFunction
{
public:
	static void swap16(char *);
	static void swap32(char *);
	static void swap64(char *);
	static void swapFilePos(char *);
	static const unsigned char	cInvertedBits[256];
};

#endif
