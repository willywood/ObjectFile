//======================================================================
// 	This software may only be compiled in conjunction with a licence
// agreement with ObjectFile Limited.
// Copyright(c) 1996-99 ObjectFile Ltd. 
//======================================================================
/*
	OOStreamFile is the output stream for writing objects to a file.

	OOStream is a device independent public interface. It can be sub-classed
to define a class that writes to some other device.

	The start of writing an object is notified by start() and the end by finish().

	More data types should be added if required.
*/


#include "odefs.h"
#include <string.h>
#include <wchar.h>
#include "ostrm.h"
#include "ofile.h"
#include "opersist.h"
#include "ox.h"
#include "oistrm.h"

bool OOStream::VBWrite(void)
// Check whether to write virtual base class.
// Return value : true if it should be written; false if it has already been written.
{
	if(_VBWritten)
		return(false);
	else
	{
		_VBWritten = true;
		return(true);
	}
}


void OOStreamFile::writeLong(O_LONG data,const char * /* label */)
// Write a long word (4 bytes)
// label - a pointer to a descriptive label for the attribute or 0.
{
	if(file()->needSwap())
		OUtilityFunction::swap32((char *)&data);

	writeData(&data,sizeof(O_LONG));
}

void OOStreamFile::writeLong64(O_LONG64 data,const char * /* label */)
// Write a 8 byte long word (8 bytes)
// label - a pointer to a descriptive label for the attribute or 0.
{
	// Do not want to write 64 bit longs if we do not have them.
	oFAssert(sizeof(O_LONG64) == 8);

	if(file()->needSwap())
		OUtilityFunction::swap64((char *)&data);

	writeData(&data,sizeof(O_LONG64));
}

void OOStreamFile::writeFilePos(OFilePos_t data,const char * /* label */)
// Write a long word (4 bytes)
// label - a pointer to a descriptive label for the attribute or 0.
{
	if(file()->needSwap())
		OUtilityFunction::swapFilePos((char *)&data);

	writeData(&data,sizeof(OFilePos_t));
}

void OOStreamFile::writeFloat(float data,const char * /* label */)
// Write a float (4 bytes)
// label - a pointer to a descriptive label for the attribute or 0.
{
	if(file()->needSwap())
		OUtilityFunction::swap32((char *)&data);

	writeData(&data,sizeof(float));
}

void OOStreamFile::writeDouble(double data,const char * /* label */)
// Write a double (8 bytes)
// label - a pointer to a descriptive label for the attribute or 0.
{
	if(file()->needSwap())
		OUtilityFunction::swap64((char *)&data);

	writeData(&data,sizeof(double));
}

void OOStreamFile::writeShort(O_SHORT data,const char * /* label */)
// Write a two byte word.
// label - a pointer to a descriptive label for the attribute or 0.
{
	if(file()->needSwap())
		OUtilityFunction::swap16((char *)&data);

	writeData(&data,sizeof(O_SHORT));
}

void OOStreamFile::writeChar(char data,const char * /* label */)
// Write a single byte character.
// label - a pointer to a descriptive label for the attribute or 0.
{
	writeData(&data,1);
}

void OOStreamFile::writeWChar(O_WCHAR_T data,const char * /* label */)
// Write a single wide character.
// label - a pointer to a descriptive label for the attribute or 0.
{
	unsigned short sdata = data;
	writeShort(sdata);
}

void OOStreamFile::writeBool(bool data,const char * /* label */)
// Write a bool.
// label - a pointer to a descriptive label for the attribute or 0.
{
static char f = 0x0;
static char t = 0x1;
	writeData(data ? &t : &f,1);
}

void OOStreamFile::writeCString(const char * str,const char * /* label */)
// Write a null terminated string.
// Two bytes are used for size, giving a maximum length of 64k.
// label - a pointer to a descriptive label for the attribute or 0.
{
	unsigned int len = strlen(str);
	oFAssert(len < USHRT_MAX);
	writeShort((unsigned short)len);
	writeData(str,len);
}

void OOStreamFile::writeWCString(const O_WCHAR_T * str,const char * /* label */)
// Write a null terminated wide character string.
// Two bytes are used for size, giving a maximum length of 64k.
// label - a pointer to a descriptive label for the attribute or 0.
{
	unsigned int len = wcslen(str);
	oFAssert(len < USHRT_MAX);
	writeShort((unsigned short)len);
	// write as shorts to preserve byte ordering
	for(unsigned int i = 0;i < len;i++)
		writeWChar(str[i]);
}

void OOStreamFile::writeCString256(const char * str,const char * /* label */)
// Write a null terminated string.
// One byte is used for the length giving a maximum length of 255 characters.
// label - a pointer to a descriptive label for the attribute or 0.
{
	unsigned int len = strlen(str);
	oFAssert(len < 256);
	writeChar((unsigned char)len);
	writeData(str,len);
}

void OOStreamFile::writeWCString256(const O_WCHAR_T * str,const char * /* label */)
// Write a null terminated wide character string.
// One byte is used for the length giving a maximum length of 255 characters.
// label - a pointer to a descriptive label for the attribute or 0.
{
	unsigned int len = wcslen(str);
	oFAssert(len < 256);
	writeChar((unsigned char)len);
	// write as shorts to preserve byte ordering
	for(unsigned int i = 0;i < len;i++)
		writeWChar(str[i]);
}

void OOStreamFile::writeBytes(const void *buf,size_t nBytes,const char * /*label */)
// Write an array of bytes.
// buf is a pointer to the array. nBytes is the number of bytes to be written.
// label - a pointer to a descriptive label for the attribute or 0.
{
	writeData(buf,nBytes);
}

void OOStreamFile::writeBits(const void *buf,size_t nBytes,const char * /*label */)
// Write an array of bits.
// buf is a pointer to the array. nBytes is the number of bytes to be written.
// label - a pointer to a descriptive label for the attribute or 0.
{
	unsigned char *bufc = (unsigned char *)buf;
	if(file()->needSwap())
	{
		for(size_t i = 0; i < nBytes; i++)
		{
			unsigned char inverted = OUtilityFunction::cInvertedBits[*bufc++];
			writeData(&inverted,1);
		}
	}
	else
		writeData(buf,nBytes);
}

void OOStreamFile::writeObjectId(OId id,const char * /* label */)
// Write an object identity.
// Assumes OId is 4 bytes.
// label - a pointer to a descriptive label for the attribute or 0.
{
	writeLong(id);
}


void OOStreamFile::writeObject(OPersist *ob,const char * /* label */)
// Write an object identity.
// Parameter ob: Object in file or 0 for no object.
// label - a pointer to a descriptive label for the attribute or 0.
{
	if(ob){
		writeObjectId(ob->oId());
	}else
		writeObjectId(0);
}


bool OOStreamFile::writeBlob(void *buf,OFilePos_t mark,unsigned long size,const char * /* label */)
// Blob gets written straight to the file.
// Returns true if data was actually written. false otherwise.
// Does not get added to _ostr.
// label - a pointer to a descriptive label for the attribute or 0.
{
	writeDataAt(mark,buf,size);
	return _calculateLengthOnly ? false : true ;
}

void OOStreamFile::writeBlobHeader(OFilePos_t mark,oulong blobLength)
// Write a blob header
// mark - Te file position
// blobLength - Length of memory used by blob.
{
	writeFilePos(mark,"_mark");
	writeLong(blobLength,"_blobLength");
}

// ========================= P R I V A T E =======================================

OOStreamFile::OOStreamFile(OFile *f):OOStream(f),
		                      _fd(*f->fd()),_count(0),_ownsFile(false)
{
	_fileLength = o_fileLength(_fd);
}

OOStreamFile::OOStreamFile(OFile *f,const char* fname,long operation):
								OOStream(f),_count(0),_ownsFile(true)
{
	_fd = o_fopen(fname,operation);
	_fileLength = o_fileLength(_fd);
}

void OOStreamFile::open(const char *fname,long operation)
{
	_fd = o_fopen(fname,operation);
}

void OOStreamFile::close(void)
// Close the file.
{
	o_fclose(_fd);
}

OOStreamFile::~OOStreamFile(void)
{
	if(_ownsFile)
		// Close the file.
		o_fclose(_fd);
	else
		// just flush it.
		o_fflush(_fd);
}

bool OOStreamFile::setLength(OFilePos_t size)
// Set the file length to size bytes.
// Return - true on success; false on failure.
{
	if(!o_setLength(_fd,size))
		return false;

	_fileLength = size;
	return true;
}


void OOStreamFile::writeData(const void *buf,size_t size)
// Write data to the output buffer. If the buffer gets full then empty it
// to the file.
{
	_count += (OFilePos_t)size;

	if(_calculateLengthOnly)
		return;

	OFilePos_t dwritten;
	char *bufp = (char *)buf;
	while((dwritten = _ostr.write((const char *)bufp,size)) != size)
	{
		bufp += dwritten;
		size -= dwritten;
		long canWrite = min(_toWrite,_ostr.bufferSize());

		writeDataAt(_mark,_ostr.set(),canWrite);
		_mark += canWrite;
		_toWrite -= canWrite;
	}
}

void OOStreamFile::writeDataAt(OFilePos_t mark,void *buf,unsigned long size)
// Write data to the specified position in the file.
{
	if(_calculateLengthOnly || (size == 0))
		return;

	// Make sure the file is long enough as on some platforms you cannot write
	// beyond the end of the file.
	OFilePos_t required = mark + size;
	if(required > _fileLength)
	{
		if(!setLength(required))
			throw OFileIOErr("Write failure.");
	}

	// Set the file position
	int serr = o_fseek(_fd,mark,SEEK_SET);
	if(serr)
		oFAssert(serr ==  0);

	// Should handle huge data ???
	long err = o_fwrite(buf,size,1,_fd);
	if(err != 1)
		throw OFileIOErr("Write failure.");
}

void OOStreamFile::writeFile(const char *fname,OFilePos_t mark,oulong from,oulong size)
// Blob gets written straight to the file.
// Does not get added to _ostr.
{
	if(_calculateLengthOnly || (size == 0))
		return;

	O_fd fd = o_fopen(fname,OFILE_OPEN_READ_ONLY);
	// Set the file position
	int serr = o_fseek(_fd,mark,SEEK_SET);
	if(serr)
		oFAssert(serr ==  0);
	serr = o_fseek(fd,from,SEEK_SET);
	if(serr)
		oFAssert(serr ==  0);

	const oulong cBufSize = 1024;
	long n;
	char buf[cBufSize];

	for(oulong i = 0; i < size; i += cBufSize)
	{		
		n = o_fread(buf,1,max(cBufSize,size % cBufSize),fd);
		if(o_fwrite(buf,1,n,_fd) != n)
			oFAssert(0);
	}

	o_fclose(fd);
}

void OOStreamFile::start(OFilePos_t mark,long size,bool calcLength)
// Start writing an object
// Parameters:mark - Position in file to write object.
//			  size - size of data in object; -1 if unknown.
//            calcLength - calculate length only. Does not actually write
//                         anything. mark is ignored in this case.
{
	// virtual base has not been written
	_VBWritten = false;
	_count = 0;
	_calculateLengthOnly = calcLength;
	if(calcLength)
		return;
	_mark = mark;
	_toWrite = size;
	_ostr.reset();
}

long OOStreamFile::finish(void)
// Finish writing an object
{
	if(!_calculateLengthOnly)
	{
		// Check that everything we said we would write is written.
		oFAssert(_ostr.size() == _toWrite);

		// Write out what is left.
		writeDataAt(_mark,_ostr.set(),_toWrite);
		_toWrite = 0;
	}
	return _count;
}

// OBuffer is used to buffer the output. It is probably unnecessary on most OS's as
// the OS buffers it.
OOStream::OBuffer::OBuffer(void)
{
	_bufSize  = 2048;
	_bufp = new char[_bufSize];
	_start = _bufp;
}

OOStream::OBuffer::~OBuffer(void)
{
	delete []_bufp;
}
void OOStream::OBuffer::reset(void)
{
	_start = _bufp;
}

void *OOStream::OBuffer::set(void)
{
	_start = _bufp;
	return (void *)_bufp;
}

oulong OOStream::OBuffer::write(const char *buf,oulong size)
// Return value: The number of  bytes actually written.
{
	oulong remaining = _bufSize - (_start -_bufp);

	oulong canWrite = (size > remaining) ? remaining : size;

	memcpy(_start,buf,canWrite);
	_start += size;
	return canWrite;
}
