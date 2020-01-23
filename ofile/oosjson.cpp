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
// OOStreamJSON is derived from OOStream. 
// It is used to output an XML representation of objects.
//
// XMLStyle
// The various styles affect the way objects and references appear as XML.
// By default, if a reference to an object that is owned by some other object 
// is written before the object is written, then the object will appear in
// the XML and not the reference to it. This may make the file appear
// unbalanced. Imagine writing a tree structure. The XML will have a hierarchy
// like the tree structure itself. To get around this you can specify "o:" as
// the first two characters of the label parameter in OOStream::writeObject().
// This will ensure that only a reference to the object is written.
// Further control can be achieved by specifying a style other than the default.
// The possible values of XMLStyle are:
//
// cBest  Take account of "o:" only.
//
// cSAX(default) SAX readable. Objects always appear in the output before references to
// them. A SAX parser does not store the parsed data, so it cannot resolve
// object references later. This is actually the most efficient way of reading
// XML, because it has very little memory overhead. To support this, objects
// must appear before their references in the file. This option ensures that
// objects get written the first time that they are referenced.
//
// cFlat   All references appear as references (even if "o:" is used in the label)
//
// Limitations:
// Wide string write functions encode data as UTF-8 regardless of the encoding string
// that is passed to beginDocument(). Multi-byte strings
// would usually be written using the single byte string functions, so this should not
// cause a problem. However if suitable conversion functions are available
// then pwriteString() can be modified to support other encodings.
//
#include "odefs.h"
#include <iostream>
#include <typeinfo>
#include <string.h>
#include <stdio.h>
#include "ofile.h"
#include "opersist.h"
#include "ox.h"
#include "oosjson.h"
#include "oiter.h"
#include "ConvertUTF.h"


// Used as the tag for data that does not have one of its own.
static const char *unnamed = "unnamed_object";

OOStreamJSON::OOStreamJSON(OFile *file, std::ostream &out,bool writeBlobsToFile):
							OOStream(0),
							_out(out),
							_sp(0),
							_unterminatedCount(0),
							_currentObj(0),
							_style(cSAX),
							_indent(true),
							_writeBlobsToFile(writeBlobsToFile),
							_fromFile(file)
// Constructor
// file - OFile from which to take the objects to be written.
// out - steam to which to write the XML formatted object.
// writeBlobsToFile - true if the BLOB data is to be written as binary to a seperate file.
// See comment of writeBlob for file naming convention.
// false if the data is to be written as XML data to the stream in BinHex format.
{
	_blobDirectory[0] = '\0';
}


OOStreamJSON::~OOStreamJSON(void)
{
}

void OOStreamJSON::reset(void)
// Reset the stream so that it will write objects that have already
// been written.
{
	_writtenObjects.erase(_writtenObjects.begin(),_writtenObjects.end());
	_blobLabels.erase(_blobLabels.begin(),_blobLabels.end());
}

void OOStreamJSON::setBlobDirectory(const char *blobDirectory)
// Set the directory to which blob data files are written.
{
	oFAssert(strlen(blobDirectory) < 256);

	strcpy(_blobDirectory,blobDirectory);
}

void OOStreamJSON::beginDocument(const char *appName,const char *encoding)
// Begin writing an XML document.
// If appName is none zero then produce a document of name appName.
// If encoding is non-zero(default 0) then use it as the character encoding of 
// the document,otherwise use "UTF-8". It is the users responsibility to ensure 
// that the encoding string is legal, and that all non-Unicode strings in the 
// objects, suit that encoding.
// The document should be terminated by calling endDocument().
// Some legal encodings.
//Japanese             "Shift_JIS"
//Korean               "KS_C_5601-1987"
//Chinese (simplified) "GB2312"
//Chinese (traditional)"Big5"
//                     "ISO-8859-1"
{
	_prevItem[_sp] = None;
	if(appName)
	{
		if (!encoding)
		{
			encoding = "UTF-8";
		}
		_out << "{\"" << appName << " root\":";
		_out << (_fromFile->getRoot()?_fromFile->getRoot()->oId() : 0)  << "}";
	}
	//_labelStack[_sp++] = appName;
}

void OOStreamJSON::endDocument(void)
// Terminate an XML document that was begun with beginDocument.
{
	_unterminatedCount++;

	// Terminate any that we have not terminated before ending.
	terminatePrev(None);

	--_sp;

}

void OOStreamJSON::writeObjectAsJSON(OPersist *ob)
// Write the given object in XML format.
// Does not write the object if it has already been written.
// Exceptions: Throws OFileError if there is an output stream error.
{
	// If the object has not been written
	if(!_writtenObjects.count(ob->oId()))
	{
		start(ob);
		ob->oWrite(this);
		finish();

		// If there was an io error then abort.
		if(_out.fail())
			throw OFileIOErr("Output stream error.");
	}
}

void OOStreamJSON::writeObjects(const char *appName,
								OClassId_t classId,
								bool deep,
								const char *encoding)
		// Write all the objects in file of class classId. Also their sub-classes
// if deep is true(default = true).
// If appName is none zero then produce a document of name appName.
// If encoding is non-zero(default 0) then use it as the character encoding of 
// the document,otherwise use "UTF-8". It is the users responsibility to ensure 
// that the encoding string is legal, and that all non-Unicode strings in the 
// objects, suit that encoding.
// Exceptions: Throws OFileError if there is an output stream error.
{
	oFAssert(_fromFile);

	try
	{
		beginDocument(appName,encoding);

		// Just in case we are calling the method a second time.
		reset();

		OIterator it(_fromFile,classId,deep);
		OPersist *ob;

		while((ob = it++))
		{
			writeObjectAsJSON(ob);
		}
		endDocument();
	}catch(...)
	{
		// Cleanup and rethrow
		reset();
		throw;
	}
	reset();
}

void OOStreamJSON::terminatePrev(ItemType newItemType)
{
	// Terminate all unterminated objects above the stack pointer.
	for (int item = _sp + _unterminatedCount; item >= _sp; --item)
	{
		switch (_prevItem[item]) {
		case None:
			_out << "\n";
			break;
		case Primitive:
			// Only terminate a primitive if there are no objects to terminate because in that case we
			// must terminate without the comma, and that will be done when the object closure is written.
			if(_unterminatedCount == 0)
				_out << ",\n";
			break;
		case Object:
			_out << "\n";
			indent(item);
			if (item == 0)
			{
				_out << "}";
			}
			_out << "}";
			// We have terminated an object so we can reduce the count.
			--_unterminatedCount;
			// Only terminate with a comma if there are no objects to terminate because in that case we
			// must terminate without the comma, and that will be done when the next object closure is written.
			if (_unterminatedCount == 0)
			{
				_out << ",\n";
			}
			break;
		case ContainingObject:
			_out << "\n";
			indent(item);
			_out << "}";
			// We have terminated an object so we can reduce the count.
			--_unterminatedCount;
			// Only terminate with a comma if there are no objects to terminate because in that case we
			// must terminate without the comma, and that will be done when the next object closure is written.
			if (_unterminatedCount == 0)
			{
				_out << ",\n";
			}
			break;
		}
	}
	_prevItem[_sp] = newItemType;
}

void OOStreamJSON::start(OPersist *ob)
// Start writing an object
// Parameters:mark - Position in file to write object.
//			  size - size of data in object; -1 if unknown.
//            calcLength - calculate length only. Does not actually write
//                         anything. mark is ignored in this case.
{
	terminatePrev(Object);

	_currentObj  = ob;

	// virtual base has not been written
	_VBWritten = false;

	indent(_sp);
	// Wrap a first level object in {}/
	if (_sp == 0)
	{
		_out << "{";
	}
	// Object metadata
	_out << "\"" << ob->meta()->className(ob) << "\": {"
		<< "\"type\": " << ob->meta()->id()
		<< ", \"ID\":" << ob->oId()
		<< ", \"ver\":" << OFile::userSourceVersion()
		<<",";

	// Check that we have not already written such an object
	oFAssert(!_writtenObjects.count(ob->oId()));

	// Mark the object as written
	_writtenObjects.insert(ob->oId());

	// Push stack ready for object content.
	_sp++;
	_prevItem[_sp] = None;
}

void OOStreamJSON::finish(void)
// Finish writing an object
{
	// Keep a count of objects that we must terminate.
	_unterminatedCount++;

	--_sp;
}

const char *OOStreamJSON::setLabel(const char *label)
// label - a descriptive label for the attribute or 0.
// return either a new label (with/without index) or 0
{

	if (!label)
	{
		label = unnamed;
	}
	else if (label[0] == '*') // label containes "*" as first character
	{
		label++;
	}

	oFAssert(strlen(label) < cLabelLength);	

	strcpy(_basicLabel,label);

	return label;
}

void OOStreamJSON::indent(int level)const
// Output indentation according to level.
{
	if(_indent)
	{
		for (int i = 0; i < level; i++)
		{
			_out << '\t';
		}
	}
}

void OOStreamJSON::startData(const char *label,const char *attributes)
// Write an attribute label.
{
	terminatePrev(Primitive);

	indent(_sp);
	_out << '\"' << setLabel(label ? label : unnamed);
	if (attributes)
	{
		_out << attributes;
	}
	_out << "\":";
}

void OOStreamJSON::endData(void)
// Write an attribute label.
{
}

void OOStreamJSON::beginObject(const char *label)
// Indicate the start of a containing object
{
	label = setLabel(label);

	if (label && 0 == strncmp("o:", label, 2))
	{
		// Ignore the "o:"
		label += 2;
	}

	// save label on stack
	_labelStack[_sp] = label ? label  : unnamed;

	terminatePrev(ContainingObject);

	indent(_sp);
	_out << '\"' << _labelStack[_sp] << "\":{";

	// Push stack ready for object content.
	_sp++;
	_prevItem[_sp] = None;
}

void OOStreamJSON::endObject(void)
// End of a containing object
{
	// stack not empty
	oFAssert(_sp);

	// Keep a count of objects that we must terminate.
	_unterminatedCount++;

	--_sp;
}

void OOStreamJSON::comment(const char *text)
// Write a text.
// text - a pointer to a text.
{
	_out << "<!--" << text << "-->" << '\t';
}

void OOStreamJSON::writeCDATA(const char *text)
// Write CDATA.
// text - a pointer to a text.
{
	_out << "\"";
	while (*text)
	{
		writeJSONEscapedChar(*text);
		text++;
	}
	_out <<"\"";
}

void OOStreamJSON::writeLong(O_LONG data, const char *label)
// Write a long word (4 bytes)
// label - a pointer to a descriptive label for the attribute or 0.
{
	startData(label);
	_out << data;
	endData();
}

void OOStreamJSON::writeLong64(O_LONG64 data, const char *label)
// Write a 64 bit long word (8 bytes)
// label - a pointer to a descriptive label for the attribute or 0.
{
	startData(label);
// Visual C++ 6.0 does not have an operator for __int64
#ifndef _MSC_VER
	_out << data;
#endif
	endData();
}

void OOStreamJSON::writeFloat(float data, const char *label)
// Write a float (4 bytes)
// label - a pointer to a descriptive label for the attribute or 0.
{
	startData(label);
	_out << data;
	endData();
}

void OOStreamJSON::writeDouble(double data, const char *label)
// Write a double (8 bytes)
// label - a pointer to a descriptive label for the attribute or 0.
{
	startData(label);
	_out << data;
	endData();
}

void OOStreamJSON::writeShort(O_SHORT data, const char *label)
// Write a two byte word.
// label - a pointer to a descriptive label for the attribute or 0.
{
	startData(label);
	_out << data;
	endData();
}

void OOStreamJSON::writeChar(char data, const char *label)
// Write a single byte character.
// label - a pointer to a descriptive label for the attribute or 0.
{
	startData(label);
	_out << "\"";
	writeJSONEscapedChar(data);
	_out << "\"";
	endData();
}

void OOStreamJSON::writeWChar(O_WCHAR_T data, const char *label)
// Write a single wide character.
// label - a pointer to a descriptive label for the attribute or 0.
{
	// O_WCHAR_T may be more than two bytes on some systems.
	unsigned short sdata = (unsigned short)data;
	writeBytes(&sdata,2,label);
}

void OOStreamJSON::writeBool(bool data, const char *label)
// Write a bool.
// label - a pointer to a descriptive label for the attribute or 0.
{
	startData(label);
	_out << (data ? "true" : "false");
	endData();
}

void OOStreamJSON::writeCString(const char * str, const char *label)
// Write a null terminated string.
// Two bytes are used for size, giving a maximum length of 64k.
// label - a pointer to a descriptive label for the attribute or 0.
{
	startData(label);
	writeCDATA(str);
	endData();
}

void OOStreamJSON::writeWCString(const O_WCHAR_T * str, const char *label)
// Write a null terminated string.
// Two bytes are used for size, giving a maximum length of 64k.
// label - a pointer to a descriptive label for the attribute or 0.
{
 	startData(label);
	_out << "\""; 
	const O_WCHAR_T *endOfStr = str + wcslen(str);
	while (endOfStr != pwriteWCString(str))
	{}
	_out << "\"";
	endData();
}

void OOStreamJSON::writeEscapedString(const char *str)
{
	while(*str)
	{
		if ('<' == *str)
		{
			_out << "&#38;#60;";
		}
		else if ('&' == *str)
		{
			_out << "&#38;#38;";
		}
		else
		{
			_out << *str;
		}
		str++;
	}
}

void OOStreamJSON::writeCString256(const char * str, const char *label)
// Write a null terminated string.
// One byte is used for the length giving a maximum length of 255 characters.
// label - a pointer to a descriptive label for the attribute or 0.
{
	startData(label);
	writeCDATA(str);
	endData();
}



void OOStreamJSON::writeWCString256(const O_WCHAR_T * str, const char *label)
// Write a null terminated string.
// One byte is used for the length giving a maximum length of 255 characters.
// label - a pointer to a descriptive label for the attribute or 0.
{
	writeWCString(str,label);
}

void OOStreamJSON::writeBytes(const void *buf, size_t nBytes, const char *label)
// Write an array of bytes.
// buf is a pointer to the array. nBytes is the number of bytes to be written.
// label - a pointer to a descriptive label for the attribute or 0.
{
	startData(label);

	_out << "\"";
	char *bufp = (char *)buf;
	for(size_t i = 0; i < nBytes; i++)
	{
		char hexStr[8];
		sprintf(hexStr,"%02hhX",(int)*bufp++);
		if (i > 0)
		{
			_out << ' ';
		}
		_out << hexStr;
	}
	_out << "\"";
	endData();
}

void OOStreamJSON::writeBits(const void *buf, size_t nBytes, const char *label)
// Write an array of bits.
// buf is a pointer to the array. nBytes is the number of bytes to be written.
// label - a pointer to a descriptive label for the attribute or 0.
{
	const unsigned char masks[8] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};

	startData(label);
	_out << '\"';
	unsigned char *bufp = (unsigned char *)buf;
	for(size_t i = 0; i < nBytes; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			if ((*bufp & masks[j]))
				_out << '1';
			else
				_out << '0';
		}
		bufp++;
	}
	_out << '\"';
	endData();
}

void OOStreamJSON::writeObjectReference(OId id,const char *label)
// Private .
// Write a reference to a persistent object.
{
	// Remove the "o:"
	if (label && 0 == strncmp("o:", label, 2))
	{
		label += 2;
	}

	// Just write a reference.
	startData(label);

	// Only write a reference if there is one (If not it is illegal XML)
	if (id)
	{
		_out <<  id;
	}

	endData();
}

void OOStreamJSON::writeObjectInstance(OPersist *ob, const char *label)
// Private.
// Write an object instance.
// Parameter ob: Object in file or 0 for no object.
// label - a pointer to a descriptive label for the attribute or 0. If
//         the label starts with "o:", indicating ownership, then the object
//         is written embedded in the current object.
{
	// Save class variables on stack
	OPersist *saveCurrentObject = _currentObj;
	bool saveVBWritten = _VBWritten;

	beginObject(label);

	// Write object
	start(ob);
	ob->oWrite(this);
	finish();

	endObject();

	// Restore class variables
	_VBWritten = saveVBWritten;
	_currentObj = saveCurrentObject;
}

void OOStreamJSON::writeObjectId(OId id,const char *label)
// Write an object identity.
// label - a pointer to a descriptive label for the attribute or 0.
{
	label = setLabel(label);
	// Write the full object if it is non-zero and is owned(o:) or _sax is set,
	// but not if it has already been written.
	if( id
		&& (_style != cFlat)
		&& ((label && 0 == strncmp( "o:",label,2)) || _style == cSAX)
		&& !_writtenObjects.count(id))
	{
		oFAssert(_fromFile);
		writeObjectInstance(_fromFile->getObject(id),label);
	}
	else
		writeObjectReference(id,label);
}

void OOStreamJSON::writeObject(OPersist *ob, const char *label)
// Write an object identity.
// Parameter ob: Object in file or 0 for no object.
// label - a pointer to a descriptive label for the attribute or 0. If
//         the label starts with "o:", indicating ownership, then the object
//         is written embedded in the current object.
{
	label = setLabel(label);
	// Write the full object if it is non-zero and is owned(o:) or _sax is set,
	// but not if it has already been written.
	if( ob
		&& (_style != cFlat)
		&& ((label && 0 == strncmp( "o:",label,2)) || _style == cSAX)
		&& !_writtenObjects.count(ob->oId()))
	{
		writeObjectInstance(ob,label);
	}
	else
	{
		writeObjectReference((ob ? ob->oId() : 0), label);
	}
}

static int base64_encode(unsigned char *source, size_t sourcelen,std::ostream &out);

bool OOStreamJSON::writeBlob(void * buf , OFilePos_t /* mark */, unsigned long blobLength, const char *label)
// Blob gets written straight to the file.
// Returns true if data was actually written. false otherwise.
// label - a pointer to a descriptive label for the attribute or 0.
// label may take the form of a file name <tag.xxx.yyy> result:
//  <tag SRC='xxxn.yyy' TYPE='yyy' SIZE=<blobLength> \>
// label may take the form of a file name <tag.yyy> result:
//  <tag SRC='unnamedn' TYPE='yyy' SIZE=<blobLength> \>
// label may take the form of a file name <tag> result:
//  <tag SRC='tagn' TYPE='' SIZE=<blobLength> \>
// Exceptions: Throws OFileError if there is problem creating the blob file.
{
	char attributes[512];
	char blobFileName[256];
	char tokenLabel[cLabelLength];

	blobFileName[0] = attributes[0] = '\0';

	// If no label is specified invent a label with a unique file name.
	label = setLabel(label);

	// Decompose the label into its components.
	strcpy(tokenLabel,label);
	const char *tag = strtok(tokenLabel,".");
	const char *prefix = strtok(NULL,".");
	const char *suffix=0;
	if(prefix)
	{
		suffix = strtok(NULL,".");
		// If there is no suffix then set it to prefix and set prefix to unnamed.
		if(!suffix)
		{
			suffix = prefix;
			prefix = unnamed;
		}
	}
	else
	{
		prefix = tag;
	}

	if(_writeBlobsToFile)
	{
		// Use the label as a file name attribute if the blob is not empty.
		strcpy(attributes,"src=\"");
		if(blobLength)
		{
			ofile_string slabel(prefix);
			if (!_blobLabels.count(slabel))
			{
				_blobLabels.insert(Dictionary::value_type(slabel, 0));
			}
			else
			{
				_blobLabels[slabel]++;
			}

			strcpy(blobFileName,prefix);

			// Add index to file name
			char str[12];
			sprintf(str,"%d",_blobLabels[slabel]);
			strcat(blobFileName,str);
			// Add extension
			if(suffix)
			{
				strcat(blobFileName,".");
				strcat(blobFileName,suffix);
			}
			strcat(attributes,blobFileName);
		}
		// Terminate SRC attribute.
		strcat(attributes,"\"");
	}
	else
	{
		// Write to XML file
		strcpy(attributes,"src=\"\",");
	}

	// Use the label extension as a TYPE name attribute if it exists, otherwise
	// empty.
	strcat(attributes," type=\"");
	strcat(attributes,suffix? suffix:"");
	strcat(attributes,"\",");

	strcat(attributes," size=");
	// Add index to file name
	char str[32];
	sprintf(str,"\"%lu\",",blobLength);
	strcat(attributes,str);

	beginObject(label);
	terminatePrev(Primitive);
	indent(_sp);

	_out << attributes;
	
	_out << "\"data\":";


	// Use label prefix as tag name.
	//startData(tag,attributes);

	// Write file if there is data
	if(blobLength)
	{
		if(_writeBlobsToFile)
		{
			// Write to external file
			char fileName[512];
			
			// Build file name. _blobDirectory must have a path seperator at the end. 
			strcpy(fileName,_blobDirectory);
			strcat(fileName,blobFileName);

			// Open file for writing, create it if it does not exist.
			O_fd fd1;
			try
			{
				fd1 = o_fopen(fileName,OFILE_OPEN_FOR_WRITING|OFILE_CREATE);
			}catch(OFileErr &err)
			{
				// Format error message to include filename.
				strcat(fileName,":");
				strcat(fileName,err.why());
				OFileErr x(fileName);
				throw x;
			}

			// Write the data
			bool success = o_fwrite(buf,1,blobLength,fd1) == (long)blobLength;
			// Record the error so that fclose does not change it.
			OFileIOErr x(fileName,"Failed to write BLOB file.");

			o_fclose(fd1);

			if (!success)
			{
				throw x;
			}
		}
		else
		{
			// Write to XML file
			_out << '\"';
			base64_encode((unsigned char *)buf, blobLength,_out);
			_out << '\"';
		}
	}
	endData();
	endObject();

	return true;
}

void OOStreamJSON::writeBlobHeader(OFilePos_t /*mark*/,oulong /*blobLength */)
// Write a blob header
// mark - Te file position
// blobLength - Length of memory used by blob.
{
}

void OOStreamJSON::writeJSONEscapedChar(unsigned char u)
{
	const unsigned char JSONEscapedCharMap[256] = {
	0,0,0,0,0,0,0,0,4,8,6,0,5,7,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,3,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	};
	const char JSONEscapedChars[] = {'"','\\','/','b','f','n','r','t'};

	unsigned char JSONEscapedCharIndex = JSONEscapedCharMap[u];
	if (JSONEscapedCharIndex == 0)
	{
		_out << u;
	}
	else
	{
		_out << '\\' << JSONEscapedChars[JSONEscapedCharIndex - 1];
	}
}

const O_WCHAR_T *OOStreamJSON::pwriteWCString(const O_WCHAR_T * str)
// Private
// Write a null terminated string.
// label - a pointer to a descriptive label for the attribute or 0.
// There may be 5 characters in UTF-8 for each wide character, so this function
// may have to called several times in order to write 256 characters.
{
	const int bufferSize = 256;
	UTF8 target[bufferSize];
	UTF8 *ptargetStart = &target[0];
	UTF8 *ptarget = ptargetStart;
	ConversionResult res = ::ConvertUTF16toUTF8 (&str, str + wcslen(str),&ptarget, &target[bufferSize - 1], lenientConversion);
	OFILE_UNUSED(res);
	while (ptargetStart < ptarget)
	{
		writeJSONEscapedChar(*ptargetStart++);
	}
	return str;
}

static const char *BASE64_CHARS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/**
 * encode three bytes using base64 (RFC 3548)
 *
 * @param triple three bytes that should be encoded
 * @param result buffer of four characters where the result is stored
 */
static void _base64_encode_triple(unsigned char triple[3], char result[4])
{
	int tripleValue, i;

	tripleValue = triple[0];
	tripleValue *= 256;
	tripleValue += triple[1];
	tripleValue *= 256;
	tripleValue += triple[2];

	for (i=0; i<4; i++)
	{
		result[3-i] = BASE64_CHARS[tripleValue%64];
		tripleValue /= 64;
	}
}

/**
 * encode an array of bytes using Base64 (RFC 3548)
 *
 * @param source the source buffer
 * @param sourcelen the length of the source buffer
 * @param target the target buffer
 * @param targetlen the length of the target buffer
 * @return 1 on success, 0 otherwise
 */
static int base64_encode(unsigned char *source, size_t sourcelen,std::ostream &out)
{
	char target[5];

	/* encode all full triples */
	while (sourcelen >= 3)
	{
		_base64_encode_triple(source, &target[0]);
		sourcelen -= 3;
		source += 3;
		target[4] = 0;
		out << target;

	}

	/* encode the last one or two characters */
	if (sourcelen > 0)
	{
		unsigned char temp[3];
		memset(temp, 0, sizeof(temp));
		memcpy(temp, source, sourcelen);
		_base64_encode_triple(temp, target);
		target[3] = '=';
		if (sourcelen == 1)
		{
			target[2] = '=';
		}

		/* terminate the string */
		target[4] = 0;
		out << target;
	}
	return 1;
}
