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
///////////////////////////////////////////////////////////////////////////
// OIStreamXML is a concrete subclass of OIStream. 
// It is used to read an XML representation of objects.
// The overridden read functions are designed to be called from the constuctors
// of persistent objects (that are derived from OPersist). 
// A SAX parser is used to progressively parse a standard input stream in a way
// that is defined by the data function of this stream, that is called.
// Tag checking is performed only if a label is provided by the caller.
// When a parser error is encountered an OFileErr exception is thrown. The stream
// position of the error is obtained by the methods get getLine() and getColumn().
// Note that the column position includes tabs as a single character.
///////////////////////////////////////////////////////////////////////////
#include "odefs.h"
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "oisxml.h"
#include "ofile.h"
#include "ometa.h"
#include "ox.h"
#include "opersist.h"

#ifndef OF_MULTI_THREAD
// There can never be two readfunctions running simulultaneously,so
// save space by making all the streams share the same return buffer
OIStreamXML::StrBuffT OIStreamXML::_strBuffer;
#endif

OIStreamXML::OIStreamXML(OFile *file, std::istream &in):
						   _file(file),
						   _in(in),
						   _blobHandler(0),
						   _currentOId(0),
						   _uniqueId(0),
						   _returnString(0),
						   _wreturnString(0)
// Constructor
// file - OFile into which the objects will be written.
// in - Stream from which the objects in XML format are read.
{
}



char *asciiOf(const XMLCh* from,char *to)
// Return a pointer to the ascii of a wide char string.
{
	char *ret = to;
	while((*to++ = (char)*from++));
	return ret;
}

class BasicHandler: public OContentHandler
// Provides error checking and handling.
{
public:
	BasicHandler(const char *label):_tagLabel(label){}
	void startElement(const XMLCh *localName,OAttributes *_attributes);
	void endElement(const XMLCh *localName){OFILE_UNUSED(localName);};
	void characters(const XMLCh *characters,int len){OFILE_UNUSED(characters);OFILE_UNUSED(len);};
    void fatalError(const OXMLReader::OXMLErr& exc);
	const char *_tagLabel;
};

void BasicHandler::startElement(const XMLCh *localName,OAttributes * /*_attributes */)
{ 
	// Check for the correct tag.
	if(_tagLabel)
	{
		char buffer[256];
		if(strcmp(asciiOf(localName,buffer),_tagLabel) !=0)
		{
			char errorMsg[256];

			sprintf(errorMsg,"Invalid tag: %ls, found. Expecting %s.",localName,_tagLabel);
			throw OFileErr(errorMsg);
		}
	}
}

void BasicHandler::fatalError(const OXMLReader::OXMLErr& exc)
{
	throw OFileErr(exc.why());
}


class ObjectHandler: public BasicHandler
// Handles objects and references to objects.
{
public:
	ObjectHandler(const char *label):BasicHandler(label),
									_objOId(0),
									_objType(0),
    								_objVersion(0),
									_obj(false),
									_maybeNullRef(false),
									_open(false){}
	void startElement(const XMLCh *localName,OAttributes *_attributes);
	void endElement(const XMLCh *localName);
	OId _objOId;		// Object or reference identifier.
	OClassId_t _objType;// Object type
	long _objVersion;   // Object version.
	bool _obj;			// true if an object or reference was found.
	bool _maybeNullRef; // potentially a null reference. Will only know for sure when endElement is called.
	bool _open;         // true as soon as an element is opened.
};



void ObjectHandler::startElement(const XMLCh *localName,OAttributes *_attributes)
{
	char buffer[256];
	_maybeNullRef = false;
	_open = true;

	if(_attributes->getLength() >= 2 &&
		wcscmp(_attributes->getLocalName(0),L"type") == 0 && 
		wcscmp(_attributes->getLocalName(1),L"ID") == 0)
	{
		// Object header
		char buffer[256];
		_objType = (OClassId_t)atoi(asciiOf(_attributes->getValue(0),buffer));

		OMeta *meta;
		if(!_objType)
		{
			// From name
			meta = OMeta::meta(asciiOf(localName,buffer));
		}
		else
		{
			// From type
			meta = OMeta::meta(_objType);
		}
		// Check if we suceded in determing the object type.
		if(meta)
		{
			// Try to determine the class name. It is not always possible without an object.
			const char *className = meta->className(0);
			if(className)
			{
				_tagLabel = className;
			}
			_objOId = atoi(asciiOf(&_attributes->getValue(1)[2],buffer));
			_objVersion = atoi(asciiOf(_attributes->getValue(2),buffer));
			_obj = true;
		}
	}
	else if(_attributes->getLength() == 1 &&
		wcscmp(_attributes->getLocalName(0),L"IDREF") == 0)
	{
		// Object reference
		_objOId = atoi(asciiOf(&_attributes->getValue(0)[2],buffer));
		_obj = true;
	}
    else
    {
    	// Maybe a Null object reference, but we will only know if we hit an endtag.
	   	_maybeNullRef = true;
	}
	if (_tagLabel)
	{
		BasicHandler::startElement(localName, _attributes);
	}
}

void ObjectHandler::endElement(const XMLCh * /*localName*/)
{
	if (_maybeNullRef)
	{
		// Now we know for sure.
		_obj = true;
	}
}

class PrimitiveHandler: public BasicHandler
// Handles all types of data.
{
public:
	PrimitiveHandler(const char *label,char *buf,int maxLen = 255,bool ownsBuf = false):BasicHandler(label),
	_bufp(buf),_len(0),_maxLen(maxLen),_ownsBuf(ownsBuf),_got(cNone){}
	~PrimitiveHandler(){if(_ownsBuf)delete []_bufp;}
	void startElement(const XMLCh *localName,OAttributes *_attributes);
	void endElement(const XMLCh *localName);
	void characters(const XMLCh *characters,int len);
	char *_bufp;  // Result buffer
	int _len;     // Result length
	int _maxLen;  // Max length in bytes of result (Not including null terminator)
	bool _ownsBuf;// True if buffer should be deleted on destruction.
	enum state{cNone,cStarted,cFinished};
	state _got;  // cStarted when we started reading; cFinished when we have finised reading the primitive.
};


void PrimitiveHandler::startElement(const XMLCh *localName,OAttributes *_attributes)
{ 
	BasicHandler::startElement(localName,_attributes);
	_got = cStarted;
	// Null terminate
	if (_bufp)
	{
		_bufp[0] = 0;
	}
}
void PrimitiveHandler::endElement(const XMLCh * /* localName */)
{ 
	if (_got == cStarted)
	{
		_got = cFinished;
	}
	else
	{
		_got = cNone;
	}
}
void PrimitiveHandler::characters(const XMLCh *characters,int len)
{ 
	len = min(len,_maxLen);
	for (int i = 0; i < len; i++)
	{
		_bufp[_len++] = (char)characters[i];
	}
	// Null terminate
	_bufp[_len++] = 0;
}

class BitHandler: public PrimitiveHandler
// Handles bit data.
{
public:
	BitHandler(const char *label,void *buf,int maxLen = 255):PrimitiveHandler(label,(char *)buf,maxLen){}
	void characters(const XMLCh *characters,int len);
};

void BitHandler::characters(const XMLCh *characters,int len)
// Handle a series of 1's and 0's e.g. 01110011
{
	int bit = 0;
	int byte,bitInbyte; 

	for(int i = 0;i<len; i++)
	{
		bitInbyte = bit % 8;
		byte = bit / 8;

		// Check that we have not exceded the expected number of bytes.
		if (byte >= _maxLen)
		{
			throw OFileErr("Too many bits.");
		}

		if(bitInbyte  == 0)
			_bufp[byte] = 0; 

		switch(characters[i])
		{
		case (XMLCh)'0': 
			bit++;
			break;
		case (XMLCh)'1': 
			_bufp[byte] |= (XMLCh)( 1 << bitInbyte);
			bit++;
			break;
		default: ;// Ignore anything else.
		}
	}
}

class ByteHandler: public PrimitiveHandler
// Handles byte data.
{
public:
	ByteHandler(const char *label,void *buf,int maxLen = 255):PrimitiveHandler(label,(char *)buf,maxLen),
		_first(true),_hexFound(false){}
	void characters(const XMLCh *characters,int len);
	bool _first;  // true if the next hex character is the first part of the byte.
	bool _hexFound;
};

void ByteHandler::characters(const XMLCh *characters,int len)
// Handle a series of hexadecimal bytes of the form ff ff ff
{ 
	for(int i = 0;i<len; i++)
	{
		// Check that we have not exceded the expected number of bytes.
		if (_len > _maxLen)
		{
			throw OFileErr("Too many bytes.");
		}

		// Initialize the byte
		if (_first)
		{
			_bufp[_len] = 0;
		}
        // Ignore case
		XMLCh hex = (unsigned char)tolower(characters[i]);
		if(hex >= '0' && hex <= '9')
		{
			hex = hex - '0';
			_hexFound = true;
		}
		else if (hex >= 'a' && hex <= 'f')
		{
			hex = hex - 'a' + 10;
			_hexFound = true;
		}
		else 
		{ 
			// Whitespace
			if(_hexFound)
			{
				// Prepare for the next byte.
				_len++;
				// Set this so that we do not advance the output until another hex character 
				// has been found.
				_hexFound = false;
			}
			// If there is some whitespace then reset the next hex to be the
			// first hex of the next byte.
			_first = true;
		}
		if(_hexFound)
		{
			if (!_first)
			{
				// Shift the lower nibble up.
				_bufp[_len] <<= 4;
			}
			// Set the nible into the buffer.
			_bufp[_len] |= hex;
			_first = !_first;
		}

	}
}
class WPrimitiveHandler: public BasicHandler
// Handles wide character strings.
{
public:
	WPrimitiveHandler(const char *label,O_WCHAR_T *buf,int maxLen = 255,bool ownsBuf = false):BasicHandler(label),
	_bufp(buf),_len(0),_maxLen(maxLen),_ownsBuf(ownsBuf),_got(cNone){}
	~WPrimitiveHandler(){if(_ownsBuf)delete []_bufp;}
	void startElement(const XMLCh *localName,OAttributes *_attributes);
	void endElement(const XMLCh *localName);
	void characters(const XMLCh *characters,int len);
	O_WCHAR_T *_bufp;// Pointer to result buffer
	int _len;        // Length in characters of the result
	int _maxLen;     // Maximum length in characters of the result (Not including null terminator)
	bool _ownsBuf;   // True if buffer should be deleted on destruction.
	enum state{cNone,cStarted,cFinished};
	state _got;		 // cStarted when we started reading; cFinished when we have finised reading the primitive.
};


void WPrimitiveHandler::startElement(const XMLCh *localName,OAttributes *_attributes)
{ 
	BasicHandler::startElement(localName,_attributes);
	_got = cStarted;
	// Null terminate
	if (_bufp)
	{
		_bufp[0] = 0;
	}
}

void WPrimitiveHandler::endElement(const XMLCh * /*localName */)
{ 
	if (_got == cStarted)
	{
		_got = cFinished;
	}
	else
	{
		_got = cNone;
	}
}

void WPrimitiveHandler::characters(const XMLCh *characters,int len)
{ 
	len = min(len,_maxLen);
	for (int i = 0; i < len; i++)
	{
		_bufp[_len++] = characters[i];
	}
	// Null terminate
	_bufp[_len++] = 0;
}

class DocumentHandler: public BasicHandler
{
public:
	DocumentHandler():BasicHandler(0),_start(false){}
	void startElement(const XMLCh *localName,OAttributes *_attributes);
	void endElement(const XMLCh *localName){OFILE_UNUSED(localName);}
	void characters(const XMLCh *characters,int len){OFILE_UNUSED(characters);OFILE_UNUSED(len);}
	bool _start;
	OId _rootId;
};


void DocumentHandler::startElement(const XMLCh * /*localName */,OAttributes *_attributes)
{
	char buffer[256];
	if(_attributes->getLength() == 1 &&
		wcscmp(_attributes->getLocalName(0),L"root") == 0)
	{
		_rootId = atoi(asciiOf(&_attributes->getValue(0)[2],buffer));
		_start= true;
	}
}

class BLOBHandler: public PrimitiveHandler
// Handles BLOB's
{
public:
	BLOBHandler(const char *label,char *buf):PrimitiveHandler(label,buf),_type(0),_size(0){}
	void startElement(const XMLCh *localName,OAttributes *_attributes);
	void characters(const XMLCh *characters,int len);
	size_t base64_decode(const XMLCh *source, size_t sourcelen,unsigned char *target, size_t targetlen);
	int _type;
	int _size;
};


void BLOBHandler::startElement(const XMLCh *localName,OAttributes *_attributes)
{ 
	PrimitiveHandler::startElement(localName,_attributes);
	if(_attributes->getLength() == 3 && 
		wcscmp(_attributes->getLocalName(1),L"type") == 0 && 
		wcscmp(_attributes->getLocalName(2),L"size") == 0)
	{
		// Object header
		char buffer[20];
		_size = atoi(asciiOf(_attributes->getValue(2),buffer));
	}
}

void BLOBHandler::characters(const XMLCh *characters,int len)
{ 
	int res = (int)base64_decode(characters, len, (unsigned char *)&_bufp[_len], _maxLen+ 3);
	oFAssert(res > 0);
	_len += res;
}

void OIStreamXML::readObjects(OClassId_t classId,bool deep)
// Read all the objects in file of class classId. Also their sub-classes
// Also their sub-classes if deep is true(default = true).
// Exceptions: Throws OFileError if there is an input stream error.In this case
// no objects are added to the OFile.
// When a parser error is encountered an OFileErr exception is thrown. The stream
// position of the error is obtained by the methods get getLine() and getColumn().
// The column position treats tabs as a single character.
{
	oFAssert(_file);

	DocumentHandler h;
	_reader.setContentHandler(&h);

	try
	{
		// Find the start of the document.
		while (!h._start)
		{
			_reader.parseFirst(&_in);
		}

		// Read objects untill none is found.
		while(readObject());

	}catch(...){
		// Delete all objects that were read so far.
		for(ObjectList::const_iterator it = _readObjects.begin(); it != _readObjects.end();++it)
		{
			delete (*it).second;
		}
		// Clear list.
    	_readObjects.erase(_readObjects.begin(),_readObjects.end());
		throw;
	}

		// Attach objects to file
	for(ObjectList::const_iterator it = _readObjects.begin(); it != _readObjects.end();++it)
	{
		// If deep then check if the class is in the hierarchy otherwise check
		// for an exact match.
		if ((deep && (*it).second->meta()->isA(classId)) || (!deep && (*it).second->meta()->id() == classId))
		{
			_file->attach((*it).second, false);
		}
		else
		{
			delete (*it).second;
		}
	}

	// Find and set the root if there is one.
	ObjectList::iterator oret = _readObjects.find(h._rootId);
	if (_readObjects.end() != oret)
	{
		_file->setRoot((*oret).second);
	}
}

// Return the version of this file.
long OIStreamXML::userVersion(void)const
{
	return _userVersion;
}

// Return the version of the source code.
long OIStreamXML::userSourceVersion(void)const
{
	return _file->userSourceVersion();
}

void OIStreamXML::setCurrentObject(OPersist *ob)
// Set the currently being read object to ob. This allows any references
// to the object to be resolved even though it is still being read.
// Called before any of the object is read.
{
	// If there is no object if then invent one;
	if(!_currentOId)
	{
		while(!_readObjects.count(++_uniqueId));

		_currentOId = _uniqueId;
	}	
	_readObjects.insert(ObjectList::value_type(_currentOId,ob));
}

O_LONG OIStreamXML::readLong(const char *label)
// Read a long word (4 bytes)
// Parameters: label - label describing the element.
{
	PrimitiveHandler h(label,_strBuffer.str);
	_reader.setContentHandler(&h);

	while (h._got != h.cFinished)
	{
		parseNext();
	}

	O_LONG data = atoi(_strBuffer.str);
	return data;
}

O_LONG64 OIStreamXML::readLong64(const char *label)
// Read a long word (8 bytes)
// Parameters: label - label describing the element.
{
	PrimitiveHandler h(label,_strBuffer.str);
	_reader.setContentHandler(&h);

	while (h._got != h.cFinished)
	{
		parseNext();
	}
	O_LONG64 data = atoi(_strBuffer.str);
	return data;
}

OFilePos_t OIStreamXML::readFilePos(const char * /*label*/)
// Read a long word (4 bytes)
// Parameters: label - label describing the element.
{
	// Should not be called
	oFAssert(0);
	return 0;
}

float OIStreamXML::readFloat(const char *label)
// Read a float (4 bytes)
// Parameters: label - label describing the element.
{
	PrimitiveHandler h(label,_strBuffer.str);
	_reader.setContentHandler(&h);

	while (h._got != h.cFinished)
	{
		parseNext();
	}

	float data = (float)atof(_strBuffer.str);
	return data;
}

double OIStreamXML::readDouble(const char *label)
// Read a double (8 bytes)
// Parameters: label - label describing the element.
{
	PrimitiveHandler h(label,_strBuffer.str);
	_reader.setContentHandler(&h);

	while (h._got != h.cFinished)
	{
		parseNext();
	}
	double data = atof(_strBuffer.str);
	return data;
}

O_SHORT OIStreamXML::readShort(const char *label)
// Read a two byte word.
// Parameters: label - label describing the element.
{
	PrimitiveHandler h(label,_strBuffer.str);
	_reader.setContentHandler(&h);

	while(h._got != h.cFinished)
		parseNext();
	O_SHORT data = (O_SHORT)atoi(_strBuffer.str);
	return data;
}

char OIStreamXML::readChar(const char *label)
// Read a single byte character.
// Parameters: label - label describing the element.
{
	PrimitiveHandler h(label,_strBuffer.str);
	_reader.setContentHandler(&h);

	while(h._got != h.cFinished)
		parseNext();
	return _strBuffer.str[0];
}

O_WCHAR_T OIStreamXML::readWChar(const char *label)
// Read a single byte character.
// Parameters: label - label describing the element.
{
	WPrimitiveHandler h(label,_strBuffer.wstr);
	_reader.setContentHandler(&h);

	while (h._got != h.cFinished)
	{
		parseNext();
	}

	return _strBuffer.wstr[0];
}

bool OIStreamXML::readBool(const char *label)
// Read a single byte character.
// Parameters: label - label describing the element.
{
	PrimitiveHandler h(label,_strBuffer.str);
	_reader.setContentHandler(&h);

	while (h._got != h.cFinished)
	{
		parseNext();
	}

	return strcmp(_strBuffer.str,"true") == 0;
}

void OIStreamXML::readCString(char * str,unsigned int maxlen,const char *label)
// Read a null terminated string.
// String is limited to 64k.
// Parameters: str - buffer in which to put string. (Must be long enough)
//             maxlen - maximum string length.
//			   label - label describing the element.
{
	PrimitiveHandler h(label,str,maxlen);
	_reader.setContentHandler(&h);

	while (h._got != h.cFinished)
	{
		parseNext();
	}
}

void OIStreamXML::readWCString(O_WCHAR_T * str,unsigned int maxlen,const char *label)
// Read a null terminated string.
// String is limited to 64k.
// Parameters: str - buffer in which to put string. (Must be long enough)
//             maxlen - maximum string length.
//			   label - label describing the element.
{
	WPrimitiveHandler h(label,str,maxlen);
	_reader.setContentHandler(&h);

	while (h._got != h.cFinished)
	{
		parseNext();
	}
}

char * OIStreamXML::readCString256(const char *label)
// Read a null terminated string. 
// String is limited to 256 chars including null terminator.
// Parameters: label - label describing the element.
{
	PrimitiveHandler h(label,_strBuffer.str);
	_reader.setContentHandler(&h);

	while (h._got != h.cFinished)
	{
		parseNext();
	}

	return _strBuffer.str;
}

O_WCHAR_T * OIStreamXML::readWCString256(const char *label)
// Read a null terminated string. 
// String is limited to 256 chars including null terminator.
// Parameters: label - label describing the element.

{
	WPrimitiveHandler h(label,_strBuffer.wstr);
	_reader.setContentHandler(&h);

	while (h._got != h.cFinished)
	{
		parseNext();
	}

	return _strBuffer.wstr;
}

char *OIStreamXML::readCString(const char *label)
// Read a null terminated string.
// Parameters: label - label describing the element.

// Return value: char buffer containing string. User must delete it.
{
	// Handler is responsible for deleting str.
	char *str = new char[15 << 1];
	PrimitiveHandler h(label,str,15 << 1,true);
	_reader.setContentHandler(&h);

	while(h._got != h.cFinished)
		parseNext();

	// Reduce the buffer by creating and copying to one of the correct length.
	char *ret = new char[h._len + 1];
	strcpy(ret,str);

	return ret;
}

O_WCHAR_T *OIStreamXML::readWCString(const char *label)
// Read a null terminated string.
// Parameters: label - label describing the element.
// Return value: char buffer containing string. User must delete it.
{
	// Handler is responsible for deleting str.
	O_WCHAR_T *str = new O_WCHAR_T[15 << 1];
	WPrimitiveHandler h(label,str,15 << 1,true);
	_reader.setContentHandler(&h);

	while (h._got != h.cFinished)
	{
		parseNext();
	}

	// Reduce the buffer by creating and copying to one of the correct length.
	O_WCHAR_T *ret = new O_WCHAR_T[h._len + 1];
	memcpy(ret,str,h._len*sizeof(O_WCHAR_T));
	ret[h._len] = 0;

	return ret;
}


char *OIStreamXML::readCStringD(const char *label)
// Read a null terminated string.
// Parameters: label - label describing the element.
// Return value: char buffer containing string. User must NOT delete it. It
// is deleted on the next call to this method or when finish is called. i.e.
// use it immediatly.
{
	delete []_returnString;
	_returnString =  readCString(label);

	return _returnString;
}

O_WCHAR_T *OIStreamXML::readWCStringD(const char *label)
// Read a null terminated string.
// Parameters: label - label describing the element.
// Return value: char buffer containing string. User must NOT delete it. It
// is deleted on the next call to this method or whne finish is called. i.e.
// use it immediatly.
{
	delete []_wreturnString;
	_wreturnString =  readWCString(label);

	return _wreturnString;
}

void OIStreamXML::readBytes(void *buf,int len,const char *label)
// Read a number of bytes.
// Parameters: buf - buffer in which to put bytes
//             len - number of bytes to read.
//			   label - label describing the element.
{
	ByteHandler h(label,buf,len);
	_reader.setContentHandler(&h);
	
	while (h._got != h.cFinished)
	{
		parseNext();
	}
}

void OIStreamXML::readBits(void *buf,int len,const char *label)
// Read a number of bits.
// Parameters: buf - buffer in which to put bytes
//             len - number of bytes to read.
//			   label - label describing the element.
{
	BitHandler h(label,buf,len);
	_reader.setContentHandler(&h);

	while (h._got != h.cFinished)
	{
		parseNext();
	}
}

void OIStreamXML::beginObject(const char *label)
// Indicate the start of a containing object
{
	ObjectHandler h(label);
	_reader.setContentHandler(&h);

	while (!h._open)
	{
		parseNext();
	}
}

void OIStreamXML::endObject(void)
// End of a containing object
{
	ObjectHandler h(0);
	_reader.setContentHandler(&h);
    parseNext();
}

void OIStreamXML::readObject(OPersist **obp,const char *label)
// Read an object deferred until finish()
// Parameter: Address of a pointer in which to place the reference to the
//            object. Reference is only written when finish() is called.
{
	// No reason to defer as it is next in the stream.
	*obp = readObject(label);
}

OPersist *OIStreamXML::readObject(const char *label)
// Read an object or object reference. 
// Return value: Pointer to object or 0 if null reference.
{
	OPersist *ob = 0;
	ObjectHandler h(label);
	_reader.setContentHandler(&h);

	do
	{
		if (!_reader.parseNext())
		{
			// End of file
			return 0;
		}
	}while(!h._obj); // While an object is not found.

	if(h._obj && h._objType)
	{
		// Instance

		OMeta *meta = OMeta::meta(h._objType);
		_currentOId = h._objOId;
		_userVersion = h._objVersion;
		try
		{
			ob =  meta->construct(*this);
		}catch(...){
			// Abort reading of this object.
			_readObjects.erase(h._objOId);
			// clean the index, because the object was not constructed.
			throw;
		}
        // Parse the end tag.  Must set a new handler because contructing the object
        // caused other handlers to be set.
		ObjectHandler h1(label);
		_reader.setContentHandler(&h1);
        parseNext();
	}
	else
	{
    	if(h._objOId)
        {
			// Reference
			ObjectList::iterator ret= _readObjects.find(h._objOId);
			ob = (*ret).second;
        }
		else
		{
			// Null Reference
			ob = 0;
		}
	}
	return ob;
}

OId OIStreamXML::readObjectId(const char * /*label*/)
// Read an object identity.
// Note: Assumes OId is defined as long.
{
	// Should not be called because there is no Ofile to resolve Id's against.
	oFAssert(0);
	return 0;
}

void OIStreamXML::readBlob(void *buf,OFilePos_t /*mark*/,unsigned long size)
// Read blob data.
// Parameters: buf - buffer in which to put blob data
//             size - number of bytes to read.
{
	oFAssert(_blobHandler);

	// buf has been allocated by caller.
	_blobHandler->_bufp = (char *)buf;
	_blobHandler->_maxLen = size;

	while(_blobHandler->_got != _blobHandler->cFinished)
		parseNext();

	delete _blobHandler;
	_blobHandler = 0;
}

OFile *OIStreamXML::readBlobHeader(OFilePos_t *mark,oulong *blobLength,
								oulong *fileLength,const char *label)
// Read a blob header
// Return a pointer to the OFile in which the blob is located or 0 if
// it is not located in an OFile. In this case it is read by the next
// call to readBlob().
// Parameters:
// *mark - return 0 because the file is not attached to a OFile.
// *blobLength - return length of memory required for blob. 
//  return 0 because blob has not yet been read. It will be set by the caller to the
// fileLength that is returned.
// *fileLength - return the length in bytes of the blob in the file.
// label - label describing the element.
{
	_blobHandler =  new BLOBHandler(label,0);
	_reader.setContentHandler(_blobHandler);

	while (_blobHandler->_got != _blobHandler->cStarted)
	{
		parseNext();
	}

	*mark = 0;
	*fileLength = _blobHandler->_size;
	*blobLength = 0;
	// Blob not attached
	return 0;
}

OIStreamXML::~OIStreamXML()
{

	delete []_returnString;
	delete []_wreturnString;
	// Just in case it was not deleted.
	delete _blobHandler;
}

void OIStreamXML::finish(void)
// Finish reading an object
{

	// Make a copy on the stack of the stream information, because it can be changed
	// by getObject.
	// Restore the stream state from the stack so that we can carry on reading the
	// object that was interrupted by another start.
	// Remove the return string.
	delete []_returnString;
	_returnString = 0;
}


void OIStreamXML::parseNext()
// Parse. Throw an exception in case the end of file is reached.
{
	if (!_reader.parseNext())
	{
		throw OFileErr("XML Parser reached end of file.");
	}
}


/**
 * @file base64.c
 * @brief Functions to handle Base64 encoding and decoding
 */

/**
 * characters used for Base64 encoding
 */


/**
 * decode base64 encoded data
 *
 * @param source the encoded data (zero terminated)
 * @param target pointer to the target buffer
 * @param targetlen length of the target buffer
 * @return length of converted data on success, -1 otherwise
 */
size_t BLOBHandler::base64_decode(const XMLCh *source, size_t sourcelen,unsigned char *target, size_t targetlen) 
{
    const XMLCh *cur;
    unsigned char *dest, *max_dest;
    int d, dlast, phase;
    unsigned char c;
    static int table[256] = {
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 00-0F */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 10-1F */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,-1,-1,63,  /* 20-2F */
        52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-1,-1,-1,  /* 30-3F */
        -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,  /* 40-4F */
        15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,  /* 50-5F */
        -1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,  /* 60-6F */
        41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1,  /* 70-7F */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 80-8F */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 90-9F */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* A0-AF */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* B0-BF */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* C0-CF */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* D0-DF */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* E0-EF */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1   /* F0-FF */
    };

    d = dlast = phase = 0;
    dest = target;
    max_dest = dest+targetlen;
	size_t i = 0;
    for (cur = source; i<sourcelen && dest<max_dest; ++cur,++i ) {
        d = table[(int)*cur];
        if(d != -1) {
            switch(phase) {
                case 0:
                    ++phase;
                    break;
                case 1:
                    c =  ((dlast << 2) | ((d & 0x30) >> 4));
                    *dest++ = c;
                    ++phase;
                    break;
                case 2:
                    c = (((dlast & 0xf) << 4) | ((d & 0x3c) >> 2));
                    *dest++ = c;
                    ++phase;
                    break;
                case 3:
                    c = (((dlast & 0x03 ) << 6) | d);
                    *dest++ = c;
                    phase = 0;
                    break;
            }
            dlast = d;
        }
    }

    /* we decoded the whole buffer */
    if (i == sourcelen)
	{
        return dest-target;
    }

    /* we did not convert the whole data, buffer was to small */
    return -1;
}

