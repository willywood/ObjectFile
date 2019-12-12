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
//
//  The free list consists of a map between file mark(position of free space in the file)
// and length (length in bytes of the free space at the mark).
//
#include "odefs.h"
#include "ofile.h"
#include "oflist.h"
#include <string.h>

OFilePos_t FreeList::getSpace(oulong length)
// Get space in the file of length - length.
// Return start position of space.
{
OFilePos_t mark;

	if(!length)
		// No space required
		return 0;

	FList::iterator it = _fList.begin();
	// Traverse free list
	while(it != _fList.end()){

		// Look for first fit
		if(length <= (*it).second){
			if((*it).second == length)
			{
				//exact fit

				mark = (*it).first;
				// remove from free list
				_fList.erase(it);
				return mark;
			}
			else
			{
				// inexact fit

				mark = (*it).first;
				// adjust entry	by adding a new one and removing the old one.
				_fList.insert(FList::value_type(mark + length,(*it).second - length));
				_fList.erase(it);

				return mark;
			}
		}
		it++;
	} // while

	// No fit, so extend file
	mark = _oFile->getLength();
	_oFile->increaseLengthBy(length);
	return mark;
}

void FreeList::freeSpace(OFilePos_t mark,oulong length)
// Release the space at file position mark and of length length.
// Free space is immediatly combined with its buddy if it has one, so
// there is never any contiguous free space.
{
	oFAssert(length);

	pair<FList::iterator,bool> p = _fList.insert(FList::value_type(mark,length));

	// Check that we are not freeing free space
	oFAssert(p.second);

	FList::iterator it = p.first;

	// Try to concatanate free space entries.

	FList::iterator prev = it;

	// Backward
	if(prev-- != _fList.begin()){

		// Check for overlapping space
		oFAssert((*prev).first + (*prev).second <= (*it).first);

		if((*prev).first + (*prev).second == (*it).first){
			// concatanate

			OFilePos_t nmark = (*prev).first;
			oulong nlength =  (*prev).second + (*it).second;
			// remove two...
			_fList.erase(it);
			_fList.erase(prev);
			// ...and replace it with one
			pair<FList::iterator,bool>ret =
				_fList.insert(FList::value_type(nmark,nlength));
			it = ret.first;
		}
	}

	FList::iterator next = it;

	// Forward
	if(++next != _fList.end()){


		// Check for overlapping space
		oFAssert((*it).first + (*it).second <= (*next).first);

		if((*it).first + (*it).second == (*next).first){
			// concatanate

			OFilePos_t cmark = (*it).first;
			oulong nlength = (*it).second + (*next).second;
			// remove two...
			_fList.erase(it);
			_fList.erase(next);
			// ...and replace it with one
			pair<FList::iterator,bool>ret =
			_fList.insert(FList::value_type(cmark,nlength));

			it = ret.first;
		}
	}

	// Clip any space at the end of the file
	FList::iterator end = _fList.end();
	end--;
	if((*end).first + (*end).second == _oFile->getLength())
	{
		_oFile->setLength((*end).first);
		_fList.erase(end);
	}

}

/* debugging only
void FreeList::print(void)
{
	cout << "Free list\n";
	for(FList::iterator it = _fList.begin(); it != _fList.end(); ++it)
		cout << (*it).first << ' ' << (*it).second << '\n';
}
*/

void FreeList::write(OOStreamFile *out,bool wipeFreeSpace)const
// Write the free list to the stream.
// wipeFreeSpace - writes a character over the free space. This should
//                 improve the compression ratio when zipped.
{
	long entryCount = (long)_fList.size();
	out->writeLong(entryCount);

	for(FList::const_iterator it = _fList.begin(); it != _fList.end();++it)
	{
		// mark
		out->writeFilePos((*it).first);
		// length
		out->writeLong((*it).second);
	}

#ifndef WIN16
	// This can be changed.
	const char cOF_FreeFillChar = '\xFE';

	if(wipeFreeSpace)
	{
		// For every free list element write a block of fill characters.
		for(FList::const_iterator it = _fList.begin(); it != _fList.end();++it)
		{
			// This will not work in 16 bit because we are possibly allocating 
			// arrays larger than 64K.
			char *bp = new char[(*it).second];
			memset(bp,cOF_FreeFillChar,(*it).second);

			// Write as a blob
			out->writeBlob(bp,(*it).first,(*it).second);

			delete []bp;
		}
	}
#endif
}

void FreeList::read(OIStreamFile *in)
// Read the free list from the stream.
{
	long entryCount = in->readLong();

	for(long i = 0; i < entryCount;i++){
		OFilePos_t mark = in->readFilePos();
		oulong length = in->readLong();
		_fList.insert(FList::value_type(mark,length));
	}
}
	
