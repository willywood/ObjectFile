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

// The iterator can be beyond the last object. In this case 0 is returned.

#include "odefs.h"
#include "oiter.h"

OIterator::OIterator(OFile *oFile,OClassId_t classId,bool deep):
									_classes(OMeta::meta(classId)->classes(deep)),
									_oFile(oFile)
// Constructor sets iterator to first object of class id classId,
// or its subclasses.
{
	reset();
}


void OIterator::reset(void)
// Set to first object
{
	_cSetIt = _classes.begin();
	while(_cSetIt != _classes.end())
	{
		_cListIt = _oFile->_cList.classList(*_cSetIt).begin();
		if(_cListIt != _oFile->_cList.classList(*_cSetIt).end())
			break;
		else
			_cSetIt++;
	}
}


OPersist* OIterator::begin(void)
{
	OMeta::Classes::const_iterator cSetIt = _classes.begin();
	OFile::ClassList::iterator cListIt;

	while(cSetIt != _classes.end()){
		if((cListIt = _oFile->_cList.classList(*_cSetIt).begin()) !=
			_oFile->_cList.classList(*_cSetIt).end())
			return _oFile->getObject(cListIt,*cSetIt);
		cSetIt++;
	}
	return 0;
}


OPersist *OIterator::object(OId id)
// Return pointer to the object with identity id. If it does not exist
// return 0.
// Adds a reference to the object.
{
	OMeta::Classes::const_iterator cSetIt = _classes.begin();
	while(cSetIt != _classes.end())
	{
		OFile::ClassList::iterator it = _oFile->_cList.classList(*cSetIt).find(id);
		if(it != _oFile->_cList.classList(*cSetIt).end() )
			// found
			return _oFile->getObject(it,*cSetIt);
	}
	return 0;
}
	

OPersist* OIterator::operator*()
// Return the object at the current position.
// Adds a reference to the object.
{
	if(_cSetIt != _classes.end() &&
		  _cListIt != _oFile->_cList.classList(*_cSetIt).end()){
		return  _oFile->getObject(_cListIt,*_cSetIt);
	}
	else
		return 0;

}

OPersist* OIterator::operator++()     // prefix ++a
// Increment the iterator and return the object at the new position.
// Adds a reference to the object.
{
	if(_cSetIt != _classes.end() )
	{
		// Advance to next object
		if(++_cListIt != _oFile->_cList.classList(*_cSetIt).end()){
			return operator*();
		}
		else
		{
			// Advance to next class
			while(++_cSetIt != _classes.end())
			{
				// Reset class list iterator to start of next class
				_cListIt = _oFile->_cList.classList(*_cSetIt).begin();
				if(_cListIt != _oFile->_cList.classList(*_cSetIt).end()){
					return operator*();
				}
			}
		}
	}
	return 0;
}

OPersist* OIterator::operator++(int)  // postfix a++
// Return the object at the current position and increment the iterator.
// Adds a reference to the object.
{
	OPersist *p = operator*();

	// Increment the iterator without returning the object.
	if(_cSetIt != _classes.end() )
	{
		// Advance to next object
		if(!(++_cListIt != _oFile->_cList.classList(*_cSetIt).end()))
		{
			// Advance to next class
			while(++_cSetIt != _classes.end())
			{
				// Reset class list iterator to start of next class
				_cListIt = _oFile->_cList.classList(*_cSetIt).begin();
				if(_cListIt != _oFile->_cList.classList(*_cSetIt).end())
					// found a class list with an object
					break;
			}
		}
	}

	return p;
}
