#ifndef OFLIST_H
#define OFLIST_H
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
	long size(void)const{return (long)(sizeof(long)+ _fList.size()*(sizeof(OFilePos_t)+ sizeof(long)));}
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
