//======================================================================
// 	This software may only be compiled in conjunction with a licence
// agreement with ObjectFile Limited.
// Copyright(c) 1996-99 ObjectFile Ltd. 
//======================================================================
/*
	OIStreamFile is the input stream for reading objects from a file.

	OIStream is a device independent public interface. It can be sub-classed
to define a class that reads from some other device.

	The start of reading an object is notified by start() and the end by finish().
The reading of an object may be interrupted by another start(). However it is more
efficient not to interrupt because the buffer must be flushed, and the file position
moved back and forth.

 Future improvements:
	Performance when the reading of an object, is interrupted by the reading of
another  object, can be improved by double buffering. Two buffers will be
used.  When the stack is pushed the buffer is switched. After popping the
the stack it will be switched back. In this way, two objects can  be buffered
at the same time.

	Other data types should be added if needed.
*/


#include "odefs.h"
#include <string.h>
#include "oistrm.h"
#include "ofile.h"
#include "ox.h"

#ifndef OF_MULTI_THREAD
// There can never be two readfunctions running simulultaneously,so
// save space by making all the streams share the same return buffer
OIStreamFile::StrBuffT OIStreamFile::_strBuffer;
#endif

// Version control methods

// Return the version of this file.
long OIStreamFile::userVersion(void)const{return _file->userVersion();}

// Return the version of the source code.
long OIStreamFile::userSourceVersion(void)const{return _file->userSourceVersion();}

void OIStreamFile::setCurrentObject(OPersist *ob)
// Set the currently being read object to ob. This allows any references
// to the object to be resolved even though it is still being read.
// Called before any of the object is read.
{
	_file->setCurrentIndex(ob);
}

O_LONG OIStreamFile::readLong(const char * /*label */)
// Read a long word (4 bytes)
{
	O_LONG data;
	readData(&data,sizeof(O_LONG));
	if(file()->needSwap())
		OUtilityFunction::swap32((char *)&data);
	return data;
}

O_LONG64 OIStreamFile::readLong64(const char * /*label */)
// Read a long word (8 bytes)
{
	O_LONG64 data;
	readData(&data,sizeof(O_LONG64));
	if(file()->needSwap())
		OUtilityFunction::swap64((char *)&data);
	return data;
}

OFilePos_t OIStreamFile::readFilePos(const char * /*label */)
// Read a long word (4 bytes)
{
	OFilePos_t data;
	readData(&data,sizeof(OFilePos_t));
	if(file()->needSwap())
		OUtilityFunction::swapFilePos((char *)&data);
	return data;
}

float OIStreamFile::readFloat(const char * /*label */)
// Read a float (4 bytes)
{
	float data;
	readData(&data,sizeof(float));
	if(file()->needSwap())
		OUtilityFunction::swap32((char *)&data);
	return data;
}

double OIStreamFile::readDouble(const char * /*label */)
// Read a double (8 bytes)
{
	double data;
	readData(&data,sizeof(double));
	if(file()->needSwap())
		OUtilityFunction::swap64((char *)&data);
	return data;
}

O_SHORT OIStreamFile::readShort(const char * /*label */)
// Read a two byte word.
{
	O_SHORT data;
	readData(&data,sizeof(O_SHORT));
	if(file()->needSwap())
		OUtilityFunction::swap16((char *)&data);
	return data;
}

char OIStreamFile::readChar(const char * /*label */)
// Read a single byte character.
{
	char data;
	readData(&data,1);
	return data;
}

O_WCHAR_T OIStreamFile::readWChar(const char * /*label */)
// Read a single byte character.
{
	return (O_WCHAR_T)readShort();
}

bool OIStreamFile::readBool(const char * /*label */)
// Read a single byte character.
{
	char data;
	readData(&data,sizeof(char));
	return data != 0x0;
}

void OIStreamFile::readCString(char * str,unsigned int maxlen,const char * /*label */)
// Read a null terminated string.
// String is limited to 64k.
// Parameters: str - buffer in which to put string. (Must be long enough)
//             maxlen - maximum string length.
{
	unsigned short len = readShort();
	oFAssert(len <= maxlen);
	readData((void *)str,len);
	str[len] = '\0';
}

void OIStreamFile::readWCString(O_WCHAR_T * str,unsigned int maxlen,const char * /*label */)
// Read a null terminated string.
// String is limited to 64k.
// Parameters: str - buffer in which to put string. (Must be long enough)
//             maxlen - maximum string length.
{
	unsigned short len = readShort();
	oFAssert(len <= maxlen);
	for(int i = 0;i < len;i++)
		str[i] = readWChar();
	str[len] = 0;
}

char * OIStreamFile::readCString256(const char * /*label */)
// Read a null terminated string. 
// String is limited to 256 chars including null terminator.
// Parameters: str - buffer in which to put string
//             maxlen - maximum string length.
{
	unsigned char len = readChar();
	if(len)
		readData((void *)_strBuffer.str,len);
	_strBuffer.str[len] = '\0';
	return _strBuffer.str;
}

O_WCHAR_T * OIStreamFile::readWCString256(const char * /*label */)
// Read a null terminated string. 
// String is limited to 256 chars including null terminator.
// Parameters: str - buffer in which to put string
//             maxlen - maximum string length.
{
	unsigned char len = readChar();
	if(len)
	{
		for(int i = 0;i < len;i++)
			_strBuffer.wstr[i] = readWChar();
	}
	_strBuffer.wstr[len] = 0;
	return _strBuffer.wstr;
}

char *OIStreamFile::readCString(const char * /*label */)
// Read a null terminated string.
// Parameters: str - buffer in which to put string
//             maxlen - buffer length.
// Return value: char buffer containing string. User must delete it.
{
	unsigned short len = readShort();
	char *ret = new char[len + 1];
	if(len)
		readData((void *)ret,len);
	ret[len] = '\0';
	return ret;
}

O_WCHAR_T *OIStreamFile::readWCString(const char * /*label */)
// Read a null terminated string.
// Parameters: str - buffer in which to put string
//             maxlen - buffer length.
// Return value: char buffer containing string. User must delete it.
{
	unsigned short len = readShort();
	O_WCHAR_T *ret = new O_WCHAR_T[len + 1];
	if(len)
	{
		for(int i = 0;i < len;i++)
			ret[i] = readWChar();
	}
	ret[len] = 0;
	return ret;
}


char *OIStreamFile::readCStringD(const char * /*label */)
// Read a null terminated string.
// Parameters: str - buffer in which to put string
//             maxlen - buffer length.
// Return value: char buffer containing string. User must NOT delete it. It
// is deleted on the next call to this method or whne finish is called. i.e.
// use it immediatly.
{
	delete []_returnString;
	_returnString =  readCString();

	return _returnString;
}

O_WCHAR_T *OIStreamFile::readWCStringD(const char * /*label */)
// Read a null terminated string.
// Parameters: str - buffer in which to put string
//             maxlen - buffer length.
// Return value: char buffer containing string. User must NOT delete it. It
// is deleted on the next call to this method or whne finish is called. i.e.
// use it immediatly.
{
	delete []_wreturnString;
	_wreturnString =  readWCString();

	return _wreturnString;
}

void OIStreamFile::readBytes(void *buf,int len,const char * /*label */)
// Read a number of bytes.
// Parameters: buf - buffer in which to put bytes
//             len - number of bytes to read.
{
	readData(buf,len);
}

void OIStreamFile::readBits(void *buf,int len,const char * /*label */)
// Read a number of bits.
// Parameters: buf - buffer in which to put bytes
//             len - number of bytes to read.
{
	// This should do bit swapping to make it platform independent.
	readData(buf,len);

	if(file()->needSwap())
	{
		// Invert bits in the buffer
		unsigned char *bufc = (unsigned char *)buf;
		for(int i = 0; i < len; i++)
		{
			*bufc =  OUtilityFunction::cInvertedBits[*bufc];
			bufc++;
		}
	}
}


void OIStreamFile::readObject(OPersist **obp,const char * /*label */)
// Read an object deferred until finish()
// Parameter: Address of a pointer in which to place the reference to the
//            object. Reference is only written when finish() is called.
{
	OId id = readObjectId();
	OSmartPtr sp = {id,obp};
	OSmartPtrs &smartPtrs = _readObjects.top()._sp;
	smartPtrs.push_back(sp);
}

OPersist *OIStreamFile::readObject(const char * /*label */)
// Read an object immediatly. This is the least efficient method, if more
// that one object is to be read , because it means jumping back and forth
// in the file.
// This may be overcome in the future by double buffering.
// Return value: Pointer to object or 0 if null reference.
{
	OId id = readObjectId();
	return id ? _file->getObject(id) : 0;
}

OId OIStreamFile::readObjectId(const char * /*label */)
// Read an object identity.
// Note: Assumes OId is defined as long.
{
	return readLong();
}

void OIStreamFile::readBlob(void *buf,OFilePos_t mark,unsigned long size)
// Blob gets written straight to the file.
// Does not get added to _ostr.
{
	readDataAt(mark,buf,size);
}

OFile *OIStreamFile::readBlobHeader(OFilePos_t *mark,oulong *blobLength,
								oulong *fileLength,const char * /*label */)
// Read a blob header
// Return a pointer to the OFile in which the blob is located or 0 if
// it is not located in an OFile. In this case it is read by the next
// call to readBlob().
// *mark - return the file position
// *blobLength - return length of memory required for blob. This is zero
//              if the blob does not have to be immediatly allocated when
//              calling readBlob(i.e it an OFile)
// *fileLength - return the length in bytes of the blob in the file.
{
	*mark = readFilePos();
	*fileLength = readLong();
	*blobLength = 0;
	return _file;
}

// ========================= P R I V A T E =======================================
OIStreamFile::OIStreamFile(OFile *f,const char* fname,long operation):
								_file(f),
								_toRead(0),_ownsFile(true),
								_returnString(0),_wreturnString(0)
// Constructor giving ownership of the file to this stream.
{
	_fd = o_fopen(fname,operation);
	_fileOpen = true;
}

#ifdef OF_OLE
OIStreamFile::OIStreamFile(OFile *f,IStorage *istorage,const char* fname,
							unsigned long istorage_mode):
								_file(f),
								_toRead(0),_ownsFile(true),
								_returnString(0),_wreturnString(0)
// Constructor giving ownership of the file to this stream.
{
	_fd = o_fopen(istorage,fname,istorage_mode);
	_fileOpen = true;
}
#endif

OIStreamFile::OIStreamFile(OFile *f):_file(f),_fd(*f->fd()),
									_toRead(0),_ownsFile(false),
									_returnString(0),_wreturnString(0)
// Constructor without ownership of the file.
{
	_fileOpen = true;
}

OIStreamFile::~OIStreamFile()
{
	if(_ownsFile)
		// Close the file.
		close();

	delete []_returnString;
	delete []_wreturnString;
}

void OIStreamFile::open(const char *fname,long operation)
{
	oFAssert(!_fileOpen);
	_fd = o_fopen(fname,operation);
	_fileOpen = true;
}

void OIStreamFile::close(void)
// Close the file.
{
	if(_fileOpen)
	{
		o_fclose(_fd);
		_fileOpen = false;
	}
}


void OIStreamFile::readDataAt(OFilePos_t mark,void *buf,unsigned long size)
// Read data from a specified position in the file.
// Parameters: mark - byte in file to start reading
//             buf  - buffer into which the data is to be read.
//             size - size in bytes of the data.
// Debugging tip: Set a break point on the throw statements and examine the
// stack when you stop there.
{
const char *message = "Invalid file data format";

	// Reopen the file if it has been closed.
	if(!_fileOpen)
		_file->reopen();

	// Set the file position
	int ierr = o_fseek(*_file->fd(),mark,SEEK_SET);
	if(ierr)
		throw OFileIOErr(message);

	// Read the data.
	long err = o_fread(buf,size,1,*_file->fd());
	// Trying to read more data from an object than was written to it.
	if(1 != err)
		throw OFileIOErr(message);
}

void OIStreamFile::readData(void *buf,size_t size)
// Read data from the buffer. If the buffer is empty, fill it up again from
// the file.
{
	long dread;
	char* bufp = (char *)buf;
	while((dread = _ostr.read(bufp,size)) != size)
	{
		// Check that we are not reading beyond the length of the object.
		oFAssert(_toRead > 0);

		bufp += dread;
		size -= dread;
		long canRead = min(_toRead,_ostr.bufferSize());

		readDataAt(_mark,_ostr.set(canRead),canRead);

		_mark += canRead;
		_toRead -= canRead;
	}
}

void OIStreamFile::start(OFilePos_t mark,long size)
// Start reading an object.
// Parameters: mark - Poisition of object in file or 0 if the object has no data.
//             size - size in bytes of the objects data.
{
	// Save the stream state on a stack, in case we are in the middle of reading
	// an existing object.
	_readObjects.push(StrmInfo(_mark - _ostr.toRead(),_ostr.toRead() + _toRead));

	// Set the file position
	_mark = mark;

	// Objects can have a size of 0
	if(size){
		// Size of object
		_toRead = size;

		long canRead = min(_toRead,_ostr.bufferSize());

		readDataAt(_mark,_ostr.set(canRead),canRead);

		_toRead -= canRead;
		_mark += canRead;
	}
}

void OIStreamFile::finish(void)
// Finish reading an object
{

	// Make a copy on the stack of the stream information, because it can be changed
	// by getObject.
	StrmInfo info(_readObjects.top()); 
	_readObjects.pop();

	// Resolve all the read objects
	OSmartPtrs &smartPtrs = info._sp;
	for(OSmartPtrs::iterator it = smartPtrs.begin();it != smartPtrs.end();++it)
	{
		OSmartPtr &sp = *it;
		*sp._obp = sp._id ? _file->getObject(sp._id) : 0;
	}

	// Check that we have read all of the object.
//	oFAssert(_toRead == 0);

	// Start a new buffer just in case there is anything left unread from the 
	// previous object.
	_ostr.set(0);

	// Restore the stream state from the stack so that we can carry on reading the
	// object that was interrupted by another start.
	_mark = info._mark;
	_toRead = info._toRead;

	// Remove the return string.
	delete []_returnString;
	_returnString = 0;
}

void OIStreamFile::abort(void)
// Abort reading an object.
// This is called when an object cannot finish its read constructor.
{
	// Restore the stream state from the stack so that we can carry on reading the
	// object that was interrupted by another start.
	_mark = _readObjects.top()._mark;
	_toRead = _readObjects.top()._toRead;
	_readObjects.pop();
}


//
// The following routines have not been tested, due to lack of another platform.
// They swap between big-endian and little-endian processor word format.
// If anyone tests them please let me know if they work.
//

// This is a lookup table used to invert bits of a bytes.
const unsigned char	OUtilityFunction::cInvertedBits[256]	= 
{
	0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0, 
	0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0,
	0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8, 
	0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8,
	0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4, 
	0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
	0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC, 
	0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC,
	0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2,
	0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2,
	0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA, 
	0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
	0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6, 
	0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6,
	0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE, 
	0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
	0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1, 
	0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
	0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9, 
	0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9,
	0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5, 
	0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
	0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED, 
	0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
	0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3, 
	0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3,
	0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB, 
	0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
	0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7, 
	0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7,
	0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF, 
	0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF
};

void OUtilityFunction::swap16(char *val)
// Swap a 16 bit integer.
{
	char temp = val[0];
	val[0] = val[1];
	val[1] = temp;
}

void OUtilityFunction::swap32(char *val)
// Swap a 32 bit integer or float.
// Can be sped up by using BSWAP on INTEL 80486
{
	char temp0 = val[0];
	char temp1 = val[1];

	val[0] = val[3];
	val[1] = val[2];
	val[2] = temp1;
	val[3] = temp0;
}


void OUtilityFunction::swap64(char *val)
// Swap a 64 bit double.
{
	char temp0 = val[0];
	char temp1 = val[1];
	char temp2 = val[2];
	char temp3 = val[3];

	val[0] = val[7];
	val[1] = val[6];
	val[2] = val[5];
	val[3] = val[4];
	val[4] = temp3;
	val[5] = temp2;
	val[6] = temp1;
	val[7] = temp0;
}

void OUtilityFunction::swapFilePos(char *val)
// Swap a 64 bit double.
{
	if(sizeof(OFilePos_t) == 8)
		swap64(val);
	else
		swap32(val);
}


void *OIStream::OIBuffer::set(long dataLength)
{
	_dataLength = dataLength;
	_start = _bufp;
	return (void *)_bufp;
}

long OIStream::OIBuffer::read(void *buf,oulong size)
{
	long remaining = _dataLength - (_start - _bufp);

	long canRead = (size > remaining) ? remaining : size;

	memcpy(buf,_start,canRead);
	_start += canRead;
	return canRead;
}

