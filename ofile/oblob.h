#ifndef OBLOB_H
#define OBLOB_H
//======================================================================
// 	This software may only be compiled in conjunction with a licence
// agreement with ObjectFile Limited.
// Copyright(c) 1996-2000 ObjectFile Ltd. 
//======================================================================

#include "oblobt.h"
// Generates a blob that allocates regular memory.
#include "lballoc.h"
// Define a type for the blob that uses char * to access the data.
typedef OBlobT<char,long_blob_allocator<char> > OBlob;



#endif