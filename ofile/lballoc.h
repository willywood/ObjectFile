#ifndef LBALLOC_H
#define LBALLOC_H
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
// Blob allocator that uses STL default allocate functions, to 
// allocate memory.
//======================================================================

//#include <memory>

template <class T>
class long_blob_allocator {
public:
	typedef T* handle;
	typedef unsigned long size_type;
	typedef long difference_type;
#ifdef WIN16
	handle allocate(size_type n) {
	return ::allocate((difference_type)n, (handle)0);
	}
	void deallocate(handle p) { ::deallocate(p);
    } 
#else
// This has nothing to do with WIN16. It is just that the above
// code does not compile with SGI's STL and may not with other
// STL's because the standard did not define these functions.
	handle allocate(size_type n) {
	return new T[n]; // ::allocate((difference_type)n, (handle)0);
	}
	void deallocate(handle p) { delete[] p; //::deallocate(p);
    } 
#endif
	handle address(handle x) { return x; }
};

// For C++ Builder 4.0
#if __BORLANDC__ > 0x520 ||  _MSC_VER >= 1400 || __GNUC__
template<>
#endif
class long_blob_allocator<void> {
public:
	typedef void* handle;
};

#endif
