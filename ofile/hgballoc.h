#ifndef HGBBLOB_H
#define HGBBLOB_H
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