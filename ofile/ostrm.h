#ifndef OSTRM_H
#define OSTRM_H
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
// OOStream is an abstract class for writing objects.
// It can be sub-classed to send data to or across any medium.
///////////////////////////////////////////////////////////////////////////


#include <stdio.h>
//#include "obuf.h"
#include "oio.h"

class OFile;
class FreeList;

class OOStream{
public:

	// This class can be used to ensure that an object definition is terminated.
	class ODefineObject
	{
	public:
		ODefineObject(OOStream *out,const char *label = 0):_out(out)
		{
			out->beginObject(label);
		}
		~ODefineObject()
		{
			_out->endObject();
		}
	private:
		OOStream *_out;
	};
	class OBuffer{
	public:
		OBuffer(void);
		~OBuffer(void);

		oulong  write(const char *buf,oulong size);
		void *str(void)const{return _bufp;}
		void reset(void);
		void *set(void);
		oulong size(void)const{return _start - _bufp;}
		long bufferSize(void)const{return _bufSize;}

	private:
		oulong _bufSize;   // buffer size
		char *_start;     // start of free space
		char *_bufp;      // start of data buffer

	};			
			
	OOStream(OFile *f):_file(f),_VBWritten(false){}
	~OOStream(void){}
	virtual void writeLong(O_LONG data,const char *label = 0) = 0;
	virtual void writeLong64(O_LONG64 data,const char *label = 0) = 0;
	virtual void writeFloat(float data,const char *label = 0) = 0;
	virtual void writeDouble(double data,const char *label = 0) = 0;
	virtual void writeShort(O_SHORT data,const char *label = 0) = 0;
	virtual void writeChar(char data,const char *label = 0) = 0;
	virtual void writeBool(bool data,const char *label = 0) = 0;
	virtual void writeCString(const char * str,const char *label = 0) = 0;
	virtual void writeCString256(const char * str,const char *label = 0) = 0;

	// wchar support  can be removed if not used.
	virtual void writeWChar(O_WCHAR_T data,const char *label = 0) = 0;
	virtual void writeWCString(const O_WCHAR_T * str,const char *label = 0) = 0;
	virtual void writeWCString256(const O_WCHAR_T * str,const char *label = 0) = 0;
	//
	virtual void writeBytes(const void *buf,size_t nBytes,const char *label = 0) = 0;
	virtual void writeBits(const void *buf,size_t nBytes,const char *label = 0) = 0;
	virtual void writeObjectId(OId,const char *label = 0) = 0;
	virtual void writeObject(OPersist *,const char *label = 0) = 0;
	virtual bool writeBlob(void *buf,OFilePos_t mark,unsigned long size,const char *label = 0) = 0;
    virtual void writeBlobHeader(OFilePos_t mark,oulong blobLength) = 0;
	virtual void writeFile(const char *fname,OFilePos_t mark,oulong from,oulong size) = 0;
	virtual void comment(const char *text = 0) = 0;
	virtual void beginObject(const char *label) = 0;
	virtual void endObject(void) = 0;

	OFile *file(void){return _file;}
	// Stream is actually writing to the file.
	virtual bool writing(void)const = 0;

	bool VBWrite(void);

protected:
	OFile *_file;
	bool _VBWritten;
};

///////////////////////////////////////////////////////////////////////////
// OOStreamFile is a concrete subclass of OOStream. It is responsible for
// writing objects to a binary file.
// For efficiency it buffers objects before writing them. Buffering commences 
// with	the method start() and finishes with the method finish().
///////////////////////////////////////////////////////////////////////////


class OOStreamFile: public OOStream {
friend class OPersist;
friend class OFile;
friend class FreeList;
private:
	OOStreamFile(OFile *f);
	OOStreamFile(OFile *f,const char* fname,long operation);
	~OOStreamFile(void);

	void writeLong(O_LONG data,const char *label = 0);
	void writeLong64(O_LONG64 data,const char *label = 0);
	void writeFilePos(OFilePos_t data,const char *label = 0);
	void writeFloat(float data,const char *label = 0);
	void writeDouble(double data,const char *label = 0);
	void writeShort(O_SHORT data,const char *label = 0);
	void writeChar(char data,const char *label = 0);
	void writeBool(bool data,const char *label = 0);
	void writeCString(const char * str,const char *label = 0);
	void writeCString256(const char * str,const char *label = 0);

	// wchar support  can be removed if not used.
	void writeWChar(O_WCHAR_T data,const char *label = 0);
	void writeWCString(const O_WCHAR_T * str,const char *label = 0);
	void writeWCString256(const O_WCHAR_T * str,const char *label = 0);
	//
	void writeBytes(const void *buf,size_t nBytes,const char *label = 0);
	void writeBits(const void *buf,size_t nBytes,const char *label = 0);
	void writeObjectId(OId,const char *label = 0);
	void writeObject(OPersist *,const char *label = 0);
	bool writeBlob(void *buf,OFilePos_t mark,unsigned long size,const char *label = 0);
    void writeBlobHeader(OFilePos_t mark,oulong blobLength);
	void comment(const char *text = 0) {OFILE_UNUSED(text);}
	void beginObject(const char *label){OFILE_UNUSED(label);};
	void endObject(void){};

	void close(void);
	void open(const char *fname,long operation);
	bool setLength(OFilePos_t size);

	void start(OFilePos_t mark,long size,bool calcLength = false);
	long finish(void);
	void writeData(const void *buf,size_t size);
	void writeDataAt(OFilePos_t mark,void *buf,unsigned long size);
	void writeFile(const char *fname,OFilePos_t mark,oulong from,oulong size);
	OFilePos_t length(void)const{return _count;}
	// Stream is actually writing to the file.
	bool writing(void)const{return !_calculateLengthOnly;}

private:
	O_fd _fd;
	OFilePos_t _mark;
	long _toWrite;
	OFilePos_t _fileLength;
	OFilePos_t _count;
	OBuffer _ostr;
	bool _calculateLengthOnly;
	bool _ownsFile;
};


#endif
