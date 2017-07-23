#ifndef LBALLOC_H
#define LBALLOC_H
//======================================================================
// 	This software may only be compiled in conjunction with a licence
// agreement with ObjectFile Limited.
// Copyright(c) 1996,97 ObjectFile Ltd. 
//======================================================================


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
