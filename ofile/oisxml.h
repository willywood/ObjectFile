#ifndef OISXML_H
#define OISXML_H
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


#include <map>
#include "oistrm.h"
#include "oxmlreader.h"

// OIStreamXML is a concrete subclass of OIStream. 

#ifdef OFILE_STD_IN_NAMESPACE
using std::map;
using std::less;
#endif

class BLOBHandler;

class OIStreamXML: public OIStream {


typedef map<OId,OPersist *,less<OId > > ObjectList;


public:
	OIStreamXML(OFile *f, std::istream &in);
	~OIStreamXML();


	void readObjects(OClassId_t classId = cOPersist,bool deep = true);

		// Location info
	int getLine(void)const{return _reader.getLine();}
	int getColumn(void)const{return _reader.getColumn();}

private:
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

	void finish(void);

	// Return the version of this file.
	long userVersion(void)const;
	// Return the version of the source code.
	long userSourceVersion(void)const;

	void setCurrentObject(OPersist *p);

	void beginObject(const char *label);
	void endObject(void);

	// This stream does not read directly into a file.
	OFile *file(void)const{return 0;}

private:
	void parseNext();

private:
	OFile *_file;			 // File to which objects are added.
	OXMLReader _reader;
	std::istream &_in;			  // Input stream

	BLOBHandler *_blobHandler;
	ObjectList _readObjects;  // List of objects that have been read.
	OId _currentOId;
	long _userVersion;        // The version of the currently read object.
	OId _uniqueId;			  // Used to generate OId's for objects that do not have them.
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
		int len;
	}_strBuffer;      

};

#endif  // OISXML_H
