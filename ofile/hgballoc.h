#ifndef HGBBLOB_H
#define HGBBLOB_H
//======================================================================
// 	This software may only be compiled in conjunction with a licence
// agreement with ObjectFile Limited.
// Copyright(c) 1996 ObjectFile Ltd. 
//======================================================================

//======================================================================
// Blob allocator that uses GlobalAlloc of MS Windows, to 
// allocate memory.
//======================================================================
// $Log:   C:/archive/ofile/ofile/hgballoc.h_v  $
 //
 //   Rev 1.1   Jul 27 1996 12:25:20   SAM
 //misspelling
 //
 //   Rev 1.0   Feb 17 1996 20:03:38   SAM
 //Initial revision.

#include "windows.h"


template <class T>
class HGLOBAL_allocator {
public:
	 typedef T handle;
	 typedef long size_type;
	 typedef ptrdiff_t difference_type;
	 T allocate(size_type n)
	 {
		return ::GlobalAlloc(GMEM_MOVEABLE,(difference_type)n);
	 }
	 void deallocate(T p)
	 {
		 ::GlobalUnlock(p);
		 ::GlobalFree(p);
	 }
	 void  * address(handle x)
	 {
		return ::GlobalLock(x);
	 }
	 long size(T p)
	 {
		::GlobalSize(p);
	 }
};

#endif