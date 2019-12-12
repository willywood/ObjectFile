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
#include "odefs.h"
#include "oxmlreader.h"
#include "ConvertUTF.h"

using namespace std;

static const XMLCh cData[] = {'!','[','C','D','A','T','A','['};



OXMLReader::OXMLErr::OXMLErr():_msg(0)
{}

OXMLReader::OXMLErr::~OXMLErr()
{
	delete []_msg;
}

OXMLReader::OXMLErr::OXMLErr(const char *msg,int line, int column): _line(line),_column(column)
{
	_msg = new char[strlen(msg) + 1];
	strcpy(_msg,msg);
}


OXMLReader::OXMLErr::OXMLErr(const OXMLErr &c)
{
	_msg = new char[strlen(c._msg) + 1];
	strcpy(_msg,c._msg);
}


void OXMLReader::addAttribute()
{
	int len = bufferLength();
	if(!isWhiteSpace(_charBuf,len))
	{
		_charBuf[len] = 0;
		_attributes.add(_charBuf,L"");
	}
}

void OXMLReader::pauseProgressive()
{
	if(_progressive)
	{
		_resumeState = _state;
		_state = &OXMLReader::pause;
	}
}

void OXMLReader::copyBufTo(XMLCh *name)const
{
	int len = bufferLength();
	wcsncpy(name,_charBuf,len);
	// Null terminate
	name[len] = 0;
}

// From convertUTF.cpp
extern const char trailingBytesForUTF8[];
bool OXMLReader::getChar(XMLCh *c)
// Get a character from the input stream. 
// If the end of the file or any other error occurs the false is returned and c is set to 0
// Return true is there are more characters to get.
{
	// Read character
	char cin;
	_in->get(cin);

	if(_in->eof())
	{
		*c = 0;
		return false;
	}
	else
	{
		char nTrailing = trailingBytesForUTF8[(unsigned char)cin];
		if(nTrailing == 0)
			// Simple character
			*c = (XMLCh)cin;
		else
		{
			// Complex character
			char source[6];
			source[0] = cin;
			XMLCh *pTarget = c;

			char *psource = &source[1];
			// Add the trailing characters.
			switch(nTrailing)
			{
			case 5: _in->get(*psource++);
			case 4: _in->get(*psource++);
			case 3: _in->get(*psource++);
			case 2: _in->get(*psource++);
			case 1: _in->get(*psource++);
			}
			if(_in->eof())
			{
				*c = 0;
				return false;
			}

			unsigned char *pend = (unsigned char *)psource;
			psource = &source[0];
			ConversionResult res = ConvertUTF8toUTF16 ((const unsigned char **)&psource, pend, &pTarget, c+1, lenientConversion);
		    OFILE_UNUSED(res);
		}
		if(*c == (XMLCh)'\n')
		{
			// Next line
			_column = -1;
			_line++;
		}else
			// Next column
			_column++;

		return true;
	}
}

bool OXMLReader::isWhiteSpace(XMLCh c)const
{
	static const XMLCh whiteSpaceChars[] = {' ','\n','\r','\t'};
	for(unsigned int j=0;j < sizeof(whiteSpaceChars)/sizeof(whiteSpaceChars[0]);j++)
	{
		if(c == whiteSpaceChars[j])
			return true;
	}
	return false;
}

bool OXMLReader::isWhiteSpace(XMLCh *str,int len)const
// Return true if str is totally white space.
{

	for(int i=0; i < len; i++)
	{
		if(!isWhiteSpace(str[i]))
			return false;
	}
	return true;
}

void OXMLReader::sendCharacters()
{
	int len = _pCharBuf - &_charBuf[0];
	if(!isWhiteSpace(_charBuf,len))
		_fDocHandler->characters(_charBuf,len);

	// Reset buffer
	_pCharBuf = &_charBuf[0];
}

void OXMLReader::appendCharacters(const XMLCh c)
{
	*_pCharBuf++ = c;
	if (_pCharBuf == &_charBuf[cCharBufLen])
		sendCharacters();
}

void OXMLReader::clearCharacters()
{
	_pCharBuf = &_charBuf[0];
}

void OXMLReader::comment()
{
	XMLCh c;
	while (getChar(&c))
	{
		if(c == (XMLCh)'-')
		{
			if(getChar(&c))
			{
				if(c == (XMLCh)'-')
				{
					if(getChar(&c) && c == (XMLCh)'>')
					{
						// Found
						_state = &OXMLReader::search;
						break;
					}
					// Error - Did not find closing >
					_fDocHandler->fatalError(OXMLErr("Did not find closing >",_line,_column));
					_state = &OXMLReader::stopped;
					break;
				}
			}
		}
	}
	if(c == 0)
		_state = &OXMLReader::stopped;
}

void OXMLReader::pause()
{
}

void OXMLReader::tag()
{
	XMLCh c;
	while (getChar(&c))
	{
		if(c == '>')
		{
			// Output the start tag
			copyBufTo(_localName);
			_fDocHandler->startElement(_localName,&_attributes);
			clearCharacters();
			_state = &OXMLReader::search;
			pauseProgressive();
			break;
		}
		else if(c == (XMLCh)'/')
		{
			// Start and end tag together (i.e no attributes)
			copyBufTo(_localName);
			clearCharacters();
			_state = &OXMLReader::endTag;
			break;
		}
		else if(isWhiteSpace(c))
		{
			// Start and end tag together (i.e no attributes)
			copyBufTo(_localName);
			clearCharacters();
			_state = &OXMLReader::attribute;
			break;
		}
		else
			appendCharacters(c);
	}
	if(c == 0)
		_state = &OXMLReader::stopped;

}

void OXMLReader::attribute()
{
	XMLCh c;
	while (getChar(&c))
	{
		if(c == (XMLCh)'=')
		{
			// Start of attribute value
			addAttribute();
			clearCharacters();
			_state = &OXMLReader::attributeValue;
			break;
		}
		else if(c == (XMLCh)'>')
		{
			// End of attribute and tag
			addAttribute();
			_fDocHandler->startElement(_localName,&_attributes);
			_attributes.reset();
			_state = &OXMLReader::search;
			pauseProgressive();
			clearCharacters();
			break;
		}
		else if(c == (XMLCh)'/')
		{
			clearCharacters();
			_state = &OXMLReader::endTag;
			break;
		}
		else if(isWhiteSpace(c))
		{
			// Skip whitespace
		}
		else
		{
			appendCharacters(c);
		}
	}
	if(c == 0)
		_state = &OXMLReader::stopped;

}

void OXMLReader::stopped()
{
}

void OXMLReader::attributeValue()
{
	XMLCh c;
	while (getChar(&c))
	{
		if(c == (XMLCh)'"')
		{
			// Start of string
			_state = &OXMLReader::stringValue;
			clearCharacters();
			break;
		}
		else
		{
			appendCharacters(c);
		}
	}
	if(c == 0)
		_state = &OXMLReader::stopped;
}

void OXMLReader::value()
{
	XMLCh c;
	while (getChar(&c))
	{
		if(c == (XMLCh)'<')
		{
			sendCharacters();
			if(getChar(&c))
			{
				if(c == (XMLCh)'/')
				{
					// Found
					_state = &OXMLReader::endTag;
					break;
				}
			}
			// Error - faulty end tag.
			_fDocHandler->fatalError(OXMLErr("faulty end tag",_line,_column));
			_state = &OXMLReader::stopped;
			break;
		}
		else
		{
			appendCharacters(c);
		}
	}
	if(c == 0)
		_state = &OXMLReader::stopped;

}

void OXMLReader::stringValue()
{
	XMLCh c;
	while (getChar(&c))
	{
		if(c == (XMLCh)'"')
		{
			// End of string
			_charBuf[bufferLength()] = 0;
			_attributes.setValue(_charBuf);
			clearCharacters();
			_state = &OXMLReader::attribute;
			break;
		}
		else
		{
			appendCharacters(c);
		}
	}
	if(c == 0)
		_state = &OXMLReader::stopped;

}

void OXMLReader::cdata()
{
	XMLCh c;
	while (getChar(&c))
	{
		if(c == ']')
		{
			// Check for end of CDATA
			if(getChar(&c))
			{
				if(c == ']')
				{
					sendCharacters();
					if(getChar(&c) && c == (XMLCh)'>')
					{
						// Found
						_state = &OXMLReader::search;
						break;
					}
					// Error - Did not find closing >
					_fDocHandler->fatalError(OXMLErr("Did not find closing >",_line,_column));
					_state = &OXMLReader::stopped;
					break;
				}
				else
				{
					// Not found, so add the ']' that we thought indicated the end of the CDATA.
					appendCharacters(']');
				}
			}
		}
		appendCharacters(c);
	}
	if(c == 0)
		_state = &OXMLReader::stopped;

}

void OXMLReader::search()
{
	XMLCh c;
	int n = 0;
	while (getChar(&c))
	{
		if(c == (XMLCh)'<')
		{
			// Check for CDATA
			for(unsigned int i = 0; i < sizeof(cData)/sizeof(cData[0]); i++)
			{
				if(getChar(&c))
				{
					n++;
					appendCharacters(c);
					if(c != cData[i])
						break;
				}
			}
			if(n == (sizeof(cData)/sizeof(cData[0])))
			{
				// CDATA found
				_state = &OXMLReader::cdata;
				clearCharacters();
				break;
			}
			if(n == 2 && c == (XMLCh)'-')
			{
				// check for comment <!-- i.e just one more '-'
				if(getChar(&c))
				{
					appendCharacters(c);
					if(c == (XMLCh)'-')
					{
						// Comment
						_state = &OXMLReader::comment;
						clearCharacters();
						break;
					}
				}
			}
			if(c == '/')
			{
				// End tage found
				clearCharacters();
				_state = &OXMLReader::endTag;
				break;
			}
			else
			{
				// Must be a start tag
				_state = &OXMLReader::tag;
				break;
			}
		}
		else if (isWhiteSpace(c))
		{
			// Skip white space
			clearCharacters();
		}
		else 
		{
			// Must be a value
			appendCharacters(c);
			_state = &OXMLReader::value;
			break;
		}
	}

	if(c == 0)
		_state = &OXMLReader::stopped;

}

void OXMLReader::endTag()
{
	XMLCh c;
	while (getChar(&c))
	{
		if(c == '>')
		{
			// A broken tag will have its name in Element_Str by the time '>' is detected wheras a non broken tag will not.
			_state = &OXMLReader::search;
			if(bufferIsEmpty())
			{
				//End of non broken tag. Output the start tag.
				_fDocHandler->startElement(_localName,&_attributes);
			}
			else
			{
				// End of broken tag
				copyBufTo(_localName);
			}
			// Output the endElement
			_fDocHandler->endElement(_localName);
			_attributes.reset();
			clearCharacters();
			pauseProgressive();
			break;
		}
		else
			appendCharacters(c);
	}
	if(c == 0)
		_state = &OXMLReader::stopped;

}

void OXMLReader::parseSetup()
// Private.
{
	_resumeState = _state = &OXMLReader::search;
	clearCharacters();
	_attributes.reset();
}

////////////////////////////////////////////////////////////////////////
// Public methods
////////////////////////////////////////////////////////////////////////
OXMLReader::OXMLReader(): _fDocHandler(0),
		                  _state(&OXMLReader::stopped),
                          _in(0),
						  _pCharBuf(&_charBuf[0]),
						  _line(1),
						  _column(-1)
{
}

void OXMLReader::setContentHandler(OContentHandler* const handler)
{
	_fDocHandler = handler;
}

bool OXMLReader::parseNext(void)
// Parse progressively.
// Return true if there is more to parse otherwise false.
{
	// If progressively parsing then restore the state we saved before pausing.
	if(_progressive)
		_state = _resumeState;

	while(_state != &OXMLReader::pause && _state != &OXMLReader::stopped)
		// Parsing action
		(this->*_state)();

	return (_state != &OXMLReader::stopped);
		
}


bool OXMLReader::parseFirst(istream *in)
// Start a progressive parse.
// Return true if there is more to parse otherwise false.
{
	_progressive = true;
	_in = in;
	parseSetup();

	return parseNext();
		
}

void OXMLReader::parse(istream *in)
// Parse until there is nothing more to parse.
{
	_progressive = false;
	_in = in;
	parseSetup();
	while(parseNext());
		
}

// ---------------------------------------------------------------------------
//  OAttributes
// ---------------------------------------------------------------------------

bool OAttributes::add(const XMLCh *name,const XMLCh *value)
// Add a new attribute
{
	if(_count < cMaxAttributes)
	{
		wcscpy(_names[_count],name);
		wcscpy(_values[_count],value);
		_count++;
		return true;
	}else
		return false;
}

void OAttributes::setValue(const XMLCh *value)
// Set value of previously added attribute
{
	wcscpy(_values[_count - 1],value);
}

const XMLCh* OAttributes::getLocalName(const unsigned int index) const
{
    if (index >= _count) {
        return 0;
     }
    return _names[index];
}

const XMLCh* OAttributes::getValue(const unsigned int index) const
{
    if (index >= _count) {
        return 0;
     }
    return _values[index];
}

int OAttributes::getIndex(const XMLCh* const qName ) const
{
    //
    //  Search the vector for the attribute with the given name and return
    //  its type.
    //
    for (unsigned int index = 0; index < _count; index++)
    {
		if(wcscmp(qName,_names[index]) == 0)
            return index ;
    }
    return -1;
}



