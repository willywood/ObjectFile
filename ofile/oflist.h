#ifndef OFLIST_H
#define OFLIST_H
//======================================================================
// 	This software may only be compiled in conjunction with a licence
// agreement with ObjectFile Limited.
// Copyright(c) 1996-2000 ObjectFile Ltd. 
//======================================================================

#ifdef WIN16
#define Allocator allocator
#include <lngalloc.h>
#endif

#include <map>

#ifdef OFILE_STD_IN_NAMESPACE
using std::map;
using std::less;
#endif

class OFile;
class OOStreamFile;
class OIStreamFile;


class FreeList{

typedef map<OFilePos_t,oulong,less<OFilePos_t> > FList;

public:
	FreeList(OFile *o):_oFile(o){}
	OFilePos_t getSpace(oulong length);
	void freeSpace(OFilePos_t mark,oulong length);
    void write(OOStreamFile *out,bool wipeFreeSpace)const;
	void read(OIStreamFile *in);
	long size(void)const{return sizeof(long)+ _fList.size()*(sizeof(OFilePos_t)+ sizeof(long));}
	void clear(void)
//    	{_fList.clear();}
    	{_fList.erase(_fList.begin(),_fList.end());}

private:
	FList _fList;
	OFile *_oFile;
// Test code
public:
	void print(void);
};


#endif
