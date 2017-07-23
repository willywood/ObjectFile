#ifndef OIO_H
#define OIO_H
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


#if defined(__WIN32__) || defined(_WIN32) && !defined(OFILE_TEST_STD)

#include <windows.h>

typedef HANDLE Oi_fd;

#elif defined(__BORLANDC__) && !defined(__TLS__) && !defined(OFILE_TEST_STD)

//==================== B O R L A N D ============================

// This implementation is probly suitable for other windows 16-bit
// compilers. On Windows 32 bit you might as well use the stddio version.
//
// The only reason for not using stdio on 16-bit is the definition of
// size_t as unsigned int.

#include <fcntl.h>
#include <io.h>
#include <sys\stat.h>

typedef int Oi_fd;

#else

//======================== S T A N D A R D ====================

// This implementation uses stdio and should be suitable for all
// 32-bit compilers.
//
#include <stdio.h>

typedef FILE* Oi_fd;
typedef const char* O_flags;


#endif

// Basic io method prototypes. Implemented in oio.cpp

Oi_fd oi_fopen(const char *fname,long flags);

long oi_fread(void *ptr,long size,long nobj,Oi_fd &fd);

int oi_fclose(Oi_fd &fd);

int oi_fseek(Oi_fd &fd,OFilePos_t offset,int origin);

OFilePos_t oi_fileLength(Oi_fd &fd);

long oi_fwrite(const void *ptr,long size,long nobj,Oi_fd &fd);

bool oi_setLength(Oi_fd &fd,OFilePos_t size);

int oi_fflush(Oi_fd &fd);

void oi_lastError(char *messageBuffer,int maxSize);

#ifndef OF_OLE

// OLE is not implemented so just make inline calls to the basic
// io methods.

typedef Oi_fd O_fd;


inline Oi_fd o_fopen(const char *fname,long operation)
{
	return oi_fopen(fname,operation);
}

inline long o_fread(void *ptr,long size,long nobj,Oi_fd &fd)
{
	return oi_fread(ptr,size,nobj,fd);
}

inline int o_fclose(Oi_fd &fd)
{
	return oi_fclose(fd);
}

inline int o_fseek(Oi_fd &fd,OFilePos_t offset,int origin)
{
	return oi_fseek(fd,offset,origin);
}

inline OFilePos_t o_fileLength(Oi_fd &fd)
{
	return oi_fileLength(fd);
}

inline long o_fwrite(const void *ptr,long size,long nobj,Oi_fd &fd)
{
	return oi_fwrite(ptr,size,nobj,fd);
}

inline bool o_setLength(Oi_fd &fd,OFilePos_t size)
// Set the file length
// Return true on succes
{
	return oi_setLength(fd,size);
}

inline int o_fflush(Oi_fd &fd)
{
	return oi_fflush(fd);
}

inline void o_lastError(char *messageBuffer,int maxSize)
{
	oi_lastError(messageBuffer,maxSize);
}


#else
// OF_OLE

#include <windows.h>
#include <ole2.h>
#include <stdio.h>

struct O_fd{
public:
	O_fd(void):fd(0),strm(0){}
	bool ole;  // True if OLE storage is open.
	Oi_fd fd;  // Standard io stream
	LPSTREAM  strm;		// OLE stream
};

// Prototypes for io functions. Implemeted in oio.cpp

O_fd o_fopen(IStorage *istorage,const char *fname,unsigned long istorage_mode);
O_fd o_fopen(const char *fname,long flags);

long o_fread(void *ptr,long size,long nobj,O_fd &fd);

int o_fclose(O_fd &fd);

int o_fseek(O_fd &fd,OFilePos_t offset,int origin);

OFilePos_t o_fileLength(O_fd &fd);

long o_fwrite(const void *ptr,long size,long nobj,O_fd &fd);

bool o_setLength(O_fd &fd,OFilePos_t size);

int o_fflush(O_fd &fd);

void o_lastError(char *messageBuffer,int maxSize);

#endif // OF_OLE



#endif // OIO_H
