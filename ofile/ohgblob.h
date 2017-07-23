#ifndef OHGBLOB_H
#define OHGBLOB_H
//======================================================================
// 	This software may only be compiled in conjunction with a licence
// agreement with ObjectFile Limited.
// Copyright(c) 1996-2000 ObjectFile Ltd. 
//======================================================================

#include "oblobt.h"
// Generates a blob that allocates Global memory.
#include "hgballoc.h"
// Define a type for the blob that uses HGLOBAL to access the data.
typedef OBlobT<HGLOBAL,HGLOBAL_allocator<HGLOBAL> > OHGBlob;



#endif