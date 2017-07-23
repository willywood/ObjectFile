#ifndef ODEFS_H
#define ODEFS_H
//======================================================================
// 	This software may only be compiled in conjunction with a licence
// agreement with ObjectFile Limited.
// Copyright(c) 1996-2001 ObjectFile Ltd. 
//======================================================================
///////////////////////////////////////////////////////////////
// Developer definable macros. 
// These can also be defined on the compiler command line.

#include <stdint.h>

// Define this for ole2 support
//#define OF_OLE
// Define this for Borland OLE support
//#define INC_OLE2

// Define this for multiple threads. It also causes OF_REF_COUNT 
// to be defined. 
//#define OF_MULTI_THREAD

// Define this if you are using multiple processes.
// Otherwise critical sections are much faster.
// used in ofthread.h
//#define OF_THREADS_MUTEX

typedef int64_t OSYS_LONG64; // Eight bytes
//typedef __int64 OSYS_LONG64; // Eight bytes

// Uncomment this if you are using the standard library in its namespace - std
// The standard specifies this, but by default it is off, for backward compatability
// for ObjectFile users
#define OFILE_STD_IN_NAMESPACE 1

// Maximum number of classes that can be defined. Keep this low to
// reduce the memory footprint of each OFile instance. Your ClassId
// must not exceed this number less one.
// Below 10 are reserved for no good reason other than possible future
// ObjectFile expansion.
typedef long OClassId_t;
const OClassId_t cOMaxClasses = 250;

// If you want to store 64 bit longs then define this.
#define OFILE_64BIT_LONGS 1

// On Windows 2000/XP you can write files up to 2**44 - 64Kb
// On 64 bit platforms you can also write large files.
// The IO in oio.cpp is implemented for Windows 2000/XP so you can
// use it right away. However it will not work on other
// Windows platforms.
// Note that defining this will make the file format only compatible with
// files written with the same define.
// All file addresses become 8 byte, so objects require an extra 4 bytes
// of storage.
//#define OFILE_64BIT_FILE_ADDRESSES 1
#ifdef OFILE_64BIT_FILE_ADDRESSES
typedef OSYS_LONG64 OFilePos_t;
#else
typedef unsigned long OFilePos_t;
#endif

// Maximum size of the file in bytes.
#ifdef OFILE_64BIT_FILE_ADDRESSES
inline OFilePos_t cOFileMaxLength(){return 100000000000;} // 100 Giga
#else
inline OFilePos_t cOFileMaxLength(){return 2147483647UL;} //LONG_MAX
// 4GB can be supported by WIN32. However be careful because
// files that grow beyond 2GB will not be portable to all Windows
// platforms.
//inline OFilePos_t cOFileMaxLength(){return 4294967295UL;} //ULONG_MAX
#endif

// Not supported yet #define OFILE_64BIT_OBJECT_IDENTITIES 1
#ifdef OFILE_64BIT_OBJECT_IDENTITIES
typedef OSYS_LONG64 OId;
#else
typedef unsigned long OId;
#endif

///////////////////////////////////////////////////////////////

#ifdef OF_MULTI_THREAD
// Causes OPersist to be reference counted
#define OF_REF_COUNT
#endif

// Borland does not like compiling this after STL e.g <set.h>. So if you want to
// include it do it here.
//#include <cstring.h>
#include <assert.h>

#if defined(__BORLANDC__) && __BORLANDC__ < 0x500
// Borland 5.0 does not have bool.h
#include <bool.h>
#endif
#if defined(__BORLANDC__) && __BORLANDC__ <= 0x500
#include <windows.h>
// Borland 5.0 does not have these functions, so use equivalent Windows functions.
#define wcscmp lstrcmpW
#define wcsncpy lstrcpynW
#endif

#if defined(__BORLANDC__)  && __BORLANDC__ <= 0x460
// Until Borland solve their VRDIFF problem
// typedef unsigned long oulong;
typedef long oulong;
#else
typedef unsigned long oulong;
#endif
// For rogue wave(Borland 5.0x) (until ObjectFile uses namespace.
#ifndef OFILE_STD_IN_NAMESPACE
#define RWSTD_NO_NAMESPACE 1
#endif

#if defined(_MSC_VER) && _MSC_VER >= 1400
// This gets rid of the unsecure strcpy warning in VC
#pragma warning( disable : 4996 )
// 'this' : used in base member initializer list
#pragma warning( disable : 4355 )
#endif


// For comparing pointers. This should be a type that is the size of
// a memory address.
typedef unsigned long OAddress_type;
#if defined(_MSC_VER) && _MSC_VER < 1400
//The MSVC compiler does not know wchar_t is a type. Including wchar.h
// causes compile problems. So we just define it ourselves.
typedef unsigned short O_WCHAR_T; // Unicode characters(2 or 4 bytes)
#else
typedef wchar_t O_WCHAR_T;
#endif

class OPersist;
class OIStream;

typedef OPersist*  (*Func)(OIStream &);

// ClassId is depracated because it pollutes the namespace
//typedef OClassId_t ClassId;

typedef long O_LONG;      // Four bytes
typedef short O_SHORT;    // Two bytes
#ifdef OFILE_64BIT_LONGS
typedef OSYS_LONG64 O_LONG64; // Eight bytes
#else
typedef long O_LONG64;    // Eight bytes
#endif

#define OFILE_CREATE             0x00000001L
#define OFILE_OPEN_FOR_WRITING	 0x00000002L
#define OFILE_OPEN_READ_ONLY	 0x00000004L
// Fast object resolution
#define OFILE_FAST_FIND			 0x00000008L

// For eliminating compiler warnings
#define OFILE_UNUSED(x) (void)(x)

// This may be redefined to an application specific assert method, in order
// to improve the debugging process. 
#ifndef	oFAssert
#define oFAssert(x) {OFILE_UNUSED(x); assert(x);}
#endif

const OClassId_t cOPersist = 1;

// These are needed to overide STL's destroy function, which does not compile
// for pointers.
inline void destroy(OPersist *) {}
inline void destroy(OPersist **) {}


//#define OF_HASH

/* Precompiled headers
#include "ofile.h"
#include "opersist.h"
#include "oufile.h"
#include "oiter.h"
#include "oistrm.h"
#include "ostrm.h"
#include "ox.h"
#pragma hdrstop
*/
#endif
