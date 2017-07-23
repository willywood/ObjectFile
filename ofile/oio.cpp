//======================================================================
// 	This software may only be compiled in conjunction with a licence
// agreement with ObjectFile Limited.
// Copyright(c) 2000 ObjectFile Ltd. 
//======================================================================

// oio.h defines an interface for io in ObjectFile. It is not suprisingly
// similar to the interface defined by the standard io library of 'C', and
// stdio documentation can be used as the interface definition.
// The main difference is that in places that stdio uses a size_t parameter
// oio uses long. This is to allow ObjectFile to read and write blocks greater
// than 64k even on a 16-bit architecture.
// All functions are inline to avoid the overhead of an extra function call.
// The only function having no stdio equivalent is o_setLength().
//
// File sizes.
// On WIN32 we can write files up to to 4GB.
// Using stdio on a 32-bit architecture files are limited to 2GB, because
// the stdio functions use signed values to return error conditions.
// stdio on 64-bit architecture (as with the rest of ObjectFile on 64-bit
// architecture) has not been tested.
//
// o_ methods are high level
// oi_ methods are low level.
//
// Compilation flags:
// OFILE_TEST_STD - use stdio even on Borland.
// OFILE          - Compile with OLE Compound document support.(MS-Windows only)
//
// 
// 

#include "odefs.h"
#include "oio.h"
#include "ox.h"


#if defined(__WIN32__) || defined(_WIN32) && !defined(OFILE_TEST_STD)

//======================== W I N 3 2 ====================

// This implementation uses stdio and should be suitable for all
// 32-bit compilers.
//

Oi_fd oi_fopen(const char *fname,long operation)
{
		DWORD readFlags;
		DWORD shareMode = 0;
		DWORD creationDistribution = OPEN_EXISTING;
		if((operation & 0x00000007) == OFILE_CREATE)
		{
			readFlags = GENERIC_READ | GENERIC_WRITE;
			creationDistribution = CREATE_ALWAYS;
		}					
		else if(OFILE_OPEN_READ_ONLY & operation)
		{
			readFlags = GENERIC_READ;
			shareMode = FILE_SHARE_READ;
		}
		else if(OFILE_OPEN_FOR_WRITING & operation)
		{
			readFlags = GENERIC_READ | GENERIC_WRITE; // Will not create file.
			if(OFILE_CREATE & operation)
			creationDistribution = (OFILE_CREATE & operation)? OPEN_ALWAYS:OPEN_EXISTING;
		}


		HANDLE fd = CreateFile(fname,
							   readFlags,
							   shareMode,
							   0,
							   creationDistribution,
							   FILE_FLAG_RANDOM_ACCESS,
							   //|FILE_FLAG_WRITE_THROUGH, // This helps very large BLOBS, but makes writes with
							                               // lots of objects take ~40 times longer!
							   0);
		if(fd == INVALID_HANDLE_VALUE)
			throw OFileIOErr("Failed to open.");

		return fd;
}

long oi_fread(void *ptr,long size,long nobj,Oi_fd &fd)
{
	unsigned long bytesRead;
	if(ReadFile(fd,ptr,size*nobj,&bytesRead,0))
		return bytesRead/size;
	else
		return 0;
}

int oi_fclose(Oi_fd &fd)
{
	return CloseHandle(fd) ? 0 : 1;
}

int oi_fseek(Oi_fd &fd,OFilePos_t offset,int /*origin*/)
// Return 0 on success
{
#ifdef OFILE_64BIT_FILE_ADDRESSES
	LARGE_INTEGER loffset,newFilePointer;
	loffset.QuadPart = offset;
	return TRUE == SetFilePointerEx(fd,loffset,&newFilePointer,FILE_BEGIN)?0:1;
#else
	long high = 0;
	return ((0xFFFFFFFF == SetFilePointer(fd,offset,&high,FILE_BEGIN) &&
			GetLastError() != NO_ERROR) ? 1 : 0);
#endif
}

OFilePos_t oi_fileLength(Oi_fd &fd)
// Return the length of the file
{
#ifdef OFILE_64BIT_FILE_ADDRESSES
	LARGE_INTEGER fileSize;
	BOOL ret = GetFileSizeEx(fd,&fileSize);
	oFAssert(ret != FALSE);
	return fileSize.QuadPart;
#else
	DWORD high;
	DWORD size = GetFileSize(fd,&high);
	oFAssert(size != 0xFFFFFFFF || GetLastError() == NO_ERROR);
	return size;
#endif
}

long oi_fwrite(const void *ptr,long size,long nobj,Oi_fd &fd)
{
	unsigned long bytesWritten;
	if(WriteFile(fd,ptr,size*nobj,&bytesWritten,0))
		return bytesWritten/size;
	else
		return 0;
}



bool oi_setLength(Oi_fd &fd,OFilePos_t size)
// Set the file length
// Return true on succes
{
#ifdef OFILE_64BIT_FILE_ADDRESSES
	LARGE_INTEGER lsize,newFilePointer; 
	lsize.QuadPart = size;
	if(TRUE != SetFilePointerEx(fd,lsize,&newFilePointer,FILE_BEGIN))
        return false;
#else
	long high = 0;
	if(0xFFFFFFFF == SetFilePointer(fd,size,&high,FILE_BEGIN) && 
									GetLastError() != NO_ERROR)
		return false;
#endif
	return SetEndOfFile(fd) != 0;
}

int oi_fflush(Oi_fd &fd)
// Flush the file
{
	return(FlushFileBuffers(fd) ? 0 : 1);
}

void oi_lastError(char *messageBuffer,int maxSize)
// Return in the messageBuffer an error message if there is one.
// The buffer is at least maxSize characters long.
{
	*messageBuffer = '\0';
	DWORD error = GetLastError();
	if(error)
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
				  0,
				  error,  // messageid
				  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), //The user default language,      // Language id
				  messageBuffer, // Buffer
				  maxSize,       // Max buffer size
				  0);            // Format buffer
}	

// End of WIN32


#elif defined(__BORLANDC__) && !defined(__TLS__) && !defined(OFILE_TEST_STD)
#include <windows.h>

//==================== B O R L A N D ============================

// This implementation is probly suitable for other windows 16-bit
// compilers. On Windows 32 bit you might as well use the stddio version.
//
// The only reason for not using stdio on 16-bit is the definition of
// size_t as unsigned int.


Oi_fd oi_fopen(const char *fname,long operation)
{
		int flags;
		if((operation & 0x00000007) == OFILE_CREATE)
			flags = O_RDWR|O_CREAT|O_TRUNC|O_BINARY;
		else if(OFILE_OPEN_READ_ONLY & operation)
			flags = O_RDONLY|O_BINARY;
		else if(OFILE_OPEN_FOR_WRITING & operation)
		{
			flags = O_RDWR|O_BINARY;
			if(OFILE_CREATE & operation)
				flags |= O_CREAT;
		}


		Oi_fd fd = open(fname,flags,S_IREAD|S_IWRITE);
		if(fd == -1)
			throw OFileIOErr("Failed to open.");

		return fd;
}

long oi_fread(void *ptr,long size,long nobj,Oi_fd &fd)
{
	return _hread(fd,ptr,size*nobj)/size;
}

int oi_fclose(Oi_fd &fd)
{
	return close(fd);
}

int oi_fseek(Oi_fd &fd,OFilePos_t offset,int origin)
{
	long ret =  lseek(fd,offset,origin);
	return ret == -1 ? ret: 0;
}

long oi_fileLength(Oi_fd &fd)
// Return the length of the file
{
	long ret;
	if((lseek(fd,0,SEEK_END) != -1) && ((ret = tell(fd)) != -1))
		return ret;
	else
		return 0;
}

long oi_fwrite(const void *ptr,long size,long nobj,Oi_fd &fd)
{
	  return _hwrite(fd,ptr,size*nobj)/size;
}

bool oi_setLength(Oi_fd &fd,OFilePos_t size)
// Set the file length
// Return true on succes
{
	return !chsize(fd,size);
}

int oi_fflush(Oi_fd &fd)
{
	return 0;
}

void oi_lastError(char *messageBuffer,int maxSize)
// Return in the messageBuffer an error message if there is one.
// The buffer is at least maxSize characters long.
{
	*messageBuffer = '\0';
	char *error = strerror(errno);
	if(error)
		strncpy(messageBuffer,error,maxSize -1);
}

#else  // End of Borland io

//======================== S T A N D A R D ====================

// This implementation uses stdio and should be suitable for all
// 32-bit compilers.
//
#include <string.h>
#include <errno.h>

Oi_fd oi_fopen(const char *fname,long operation)
{
		const char *flags;
		if((operation & 0x00000007) == OFILE_CREATE)
			flags = "w+b";  // force create
		else if(OFILE_OPEN_READ_ONLY & operation)
			flags = "rb";
		else if(OFILE_OPEN_FOR_WRITING & operation)
			flags = "r+b"; // Will not create file.
		else
		    throw OFileIOErr(fname,"Failed to open.");


		FILE *fd = fopen(fname,flags);
		if(!fd)
		{
			if(OFILE_CREATE & operation)
				fd = fopen(fname,"w+b"); // create file
			if(!fd)
				throw OFileIOErr(fname,"Failed to open.");
		}
		return fd;
}

long oi_fread(void *ptr,long size,long nobj,Oi_fd &fd)
{
		return fread(ptr,size,nobj,fd);
}

int oi_fclose(Oi_fd &fd)
{
	return fclose(fd);
}

int oi_fseek(Oi_fd &fd,OFilePos_t offset,int origin)
{
	return fseek(fd,offset,origin);
}

OFilePos_t oi_fileLength(Oi_fd &fd)
// Return the length of the file
{
	fseek(fd,0,SEEK_END);
    return ftell(fd);
}

long oi_fwrite(const void *ptr,long size,long nobj,Oi_fd &fd)
{
	  return fwrite(ptr,size,nobj,fd);
}


bool oi_setLength(Oi_fd &fd,OFilePos_t size)
// Set the file length
// Return true on succes
{
	// This cannot be done by stdio.
	int err = fseek(fd,size+1,SEEK_SET);
	if(err)
		return false;

	if(4 != fwrite("eofm",1,4,fd))
		return false;
	err = fseek(fd,size+1,SEEK_SET);
	if(err)
		return false;

	return true;
}

int oi_fflush(Oi_fd &fd)
{
	return fflush(fd);
}

void oi_lastError(char *messageBuffer,int maxSize)
// Return in the messageBuffer an error message if there is one.
// The buffer is at least maxSize characters long.
{
	*messageBuffer = '\0';
	char *error = strerror(errno);
	if(error)
		strncpy(messageBuffer,error,maxSize -1);
}

#endif  // End of standard io


#ifdef OF_OLE

// ========================= O L E support. ================================

// ObjectFile can maintain OLE and non-OLE documents at the same time.
// When a file or storage is opened, the file descriptor is set according to
// whether it is a compound document or a regular file. This is subsequently
// used to determine what io operations to perform.
//
// Note: Only OFile and not OUFile support OLE operation.
//
// If you are writing a test program you must call:
//	OleInitialize(NULL) and OleUnInitialize()


#ifdef WIN16
// OLE 16-bit takes ascii charater parameters. OLE 32-bit takes unicode
// character parameters.
// In order to compile OLE code for both 16 and 32 bit we must virtualize
// these character parameters.
typedef  char of_wchar;

static of_wchar* A2U(const char *in,of_wchar *out)
// Ascii to unicode for 16-bit. Acts as a dummy.
{
   strcpy(out,in);
   return out;
}
inline static void setLarge(LARGE_INTEGER &li,long l)
{
   LISet32(li,l);
}
inline static void setLarge(ULARGE_INTEGER &li,long l)
{
   LISet32(li,l);
}
inline static long getLong(ULARGE_INTEGER &li)
// Return the low part of a long integer.
{
	union{    // Just for moving a 64-bit integer to a 32-bit integer
		ULARGE_INTEGER together;
		struct{
			long low;
			long high;
		}separate;
	}size;
	size.together = li;
	return size.separate.low;
}

#else  // End WIN16

//WIN32
typedef  wchar_t of_wchar;

static of_wchar* A2U(const char *in,of_wchar *out)
// Ascii to unicode.
{
   int len = strlen(in) + 1;
   MultiByteToWideChar(CP_ACP,0,in,len,out,len*2);
   return out;
}

inline static void setLarge(LARGE_INTEGER &li,long l)
{
   li.u.LowPart =  l;
   li.u.HighPart = 0;
}
inline static void setLarge(ULARGE_INTEGER &li,long l)
{
   li.u.LowPart =  l;
   li.u.HighPart = 0;
}
inline static long getLong(ULARGE_INTEGER &li)
// Return the low part of a long integer.
{
   return li.u.LowPart;
}

#endif  // End WIN32



O_fd o_fopen(IStorage *istorage,const char *fname,unsigned long istorage_mode)
// Open a file or compound document.
// Parameters - fname: pointer to file name to open.
//            - operation: additional OFILE_AS_COMPOUND flag forces the
//              creation of a compound file if no file exists, otherwise
//              a regular file will be opened.
{
O_fd res;
of_wchar wbuf[256];

	oFAssert(istorage);

    HRESULT hr;
	if(istorage_mode & STGM_CREATE)
		hr = istorage->CreateStream(A2U(fname,wbuf),
			                             istorage_mode,
										 0,
								         0,
								         &res.strm);
	else
		hr = istorage->OpenStream(A2U(fname,wbuf),
			                             0,
			                             istorage_mode,
								         0,
								         &res.strm);
			
	if(FAILED(hr))
		throw OFileIOErr("Failed to open stream in compound document.");

 	res.ole = true;
	return res;		
}

O_fd o_fopen(const char *fname,long operation)
// Open a file or compound document.
// Parameters - fname: pointer to file name to open.
//            - operation: additional OFILE_AS_COMPOUND flag forces the
//              creation of a compound file if no file exists, otherwise
//              a regular file will be opened.
{
O_fd res;

	// Not a compound document - use basic io methods
	res.fd =  oi_fopen(fname,operation);
	res.ole = false;
	return res;		
}


long o_fread(void *ptr,long size,long nobj,O_fd &fd)
{
	if(!fd.ole)
		return oi_fread(ptr,size,nobj,fd.fd);
	else
	{
		ULONG cb = (ULONG)-1;
		HRESULT hr = fd.strm->Read(ptr,size*nobj,&cb);

		if(FAILED(hr))
			return 0;
		else
			return cb / size;
	}
}

int o_fclose(O_fd &fd)
{
	if(!fd.ole)
		return oi_fclose(fd.fd);
	else
	{
		fd.strm->Release();
		return 0;
	}
}

int o_fseek(O_fd &fd,OFilePos_t offset,int origin)
{
	if(!fd.ole)
		return oi_fseek(fd.fd,offset,origin);
	else
	{
		LARGE_INTEGER li;
		setLarge(li,offset);
		fd.strm->Seek(li,STREAM_SEEK_SET,NULL);
		return 0;
	}
}

OFilePos_t o_fileLength(O_fd &fd)
{
	if(!fd.ole)
		return oi_fileLength(fd.fd);
	else
	{
		STATSTG stats;
		HRESULT hr = fd.strm->Stat(&stats,STATFLAG_DEFAULT);
		oFAssert(hr == S_OK);
		return getLong(stats.cbSize);
	}
}

long o_fwrite(const void *ptr,long size,long nobj,O_fd &fd)
{
	if(!fd.ole)
		return oi_fwrite(ptr,size,nobj,fd.fd);
	else
	{
		ULONG cb = (ULONG)-1;
		HRESULT hr = fd.strm->Write(ptr,size*nobj,&cb);

		if(FAILED(hr))
			return 0;
		else
			return cb / size;
	}
}

bool o_setLength(O_fd &fd,OFilePos_t size)
{
	if(!fd.ole)
		return oi_setLength(fd.fd,size);
	else
	{
		ULARGE_INTEGER li;
		setLarge(li,size);
		return(S_OK == fd.strm->SetSize(li));
	}
}

int o_fflush(O_fd &fd)
{
	if(!fd.ole)
		return oi_fflush(fd.fd);
	else
		return 0;
}

void o_lastError(char *messageBuffer,int maxSize)
// Return in the messageBuffer an error message if there is one.
// The buffer is at least maxSize characters long.
{
	oi_lastError(messageBuffer,maxSize);
}	

#endif // OF_OLE

