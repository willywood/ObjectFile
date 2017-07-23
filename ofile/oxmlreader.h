/*
 * Copyright (c) 2005 ObjectFile.  All rights
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * The source code may be used or modified for any purpose whatsoever,
 * include incorporation in commercial products.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE APACHE SOFTWARE FOUNDATION OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
*/
#ifndef OXMLREADER_H
#define OXMLREADER_H

///////////////////////////////////////////////////////////////////////////////////////////
// A Small and Fast SAX parser.                                   Version 0.1
// Takes data from a standard C++ stream.
//
// Usage:
// 1. Derive a handler from OContentHandler.
// 2. Create an instance of OXMLReader 
// 3. Either:
//	   Call parse() passing an open input stream.
//   Or parse progressively:
//	   Call parseFirst passing an open input stream.
//     Call parseNext() until it returns false.
// 4. If you need to abort parsing you can throw an exception from your handler.
//
// Incomplete list of syntax restrictions:
//  UTF-8 only supported. (UTF-16 or double-byte encodings can be supported with modifications).
//	Cannot use ' to enclose strings, only ".
//	String Data must either not contain special characters or it must be CDATA. This is because 
//		escape sequences are not supported.
//	Attribute names are limited as follows:
//  	enum {cNameSize = 40,cMaxAttributes = 10};
//  Tag names are limited as follows.
//		enum {cLocalNameLen = 100};
//  You can increase or reduce the above according to your requirements.
//
//
// Implementation:
// The parser is implemented as a state machine. There is a function to handle each state.
// Transition to a new state is achieved by assigning a pointer to the function of that state
// to the _state member.
//
// ToDo
// Handling UTF-16 + detection of encoding.
// More error checking.
// 
////////////////////////////////////////////////////////////////////////////////////////////
//#include <istream.h>
#include <fstream>
#include <istream>
#include <string.h>

//typedef unsigned short XMLCh ;
#define XMLCh wchar_t

class OXMLReader;
class OContentHandler;

class  OAttributes
{
public :
	OAttributes() : _count(0){}
	~OAttributes(){}

	unsigned int getLength() const{return _count;}

    const XMLCh* getLocalName(const unsigned int index) const ;
    const XMLCh* getValue(const unsigned int index) const ;

	int getIndex(const XMLCh* const qName ) const  ;

	friend class OXMLReader;

private :
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    OAttributes(const OAttributes&);
    OAttributes& operator=(const OAttributes&);


	// Add a new attribute
	bool add(const XMLCh *name,const XMLCh *value);
	// Set value of previously added attribute
	void setValue(const XMLCh *value);
	void reset(){_count = 0;}

private:
	enum {cNameSize = 40,cMaxAttributes = 10};
    unsigned int _count;    // The count of elements in the vector
	XMLCh _names[cNameSize][cMaxAttributes];
	XMLCh _values[cNameSize][cMaxAttributes];
};

class OXMLReader
{
public:
	class OXMLErr
	{
	public:
		OXMLErr(const OXMLErr &c);
		OXMLErr(const char *msg,int line, int col);
		OXMLErr(void);
		~OXMLErr();
		const char *why(void)const{return _msg;}
	protected:
		char *_msg;
		int _line;
		int _column;
	};

	OXMLReader();
	~OXMLReader(){}
    void setContentHandler(OContentHandler* const handler);
	OContentHandler* getContentHandler(void)const{return _fDocHandler;}
	void parse(std::istream *in);
	bool parseFirst(std::istream *in);
	bool parseNext(void);
	// Location info
	int getLine(void)const{return _line;}
	int getColumn(void)const{return _column;}
private:
	typedef  void (OXMLReader::*PFF)();

	// State functions.
	void comment();
	void pause();
	void tag();
	void attribute();
	void stopped();
	void attributeValue();
	void value();
	void stringValue();
	void cdata();
	void endTag();
	void search();

	// Input function
	bool getChar(XMLCh *);

	void appendCharacters(const XMLCh );
	void sendCharacters();
	void clearCharacters();
	bool isWhiteSpace(XMLCh *,int len)const;
	bool isWhiteSpace(XMLCh c)const;

	bool bufferIsEmpty()const{return &_charBuf[0] == _pCharBuf;}
	int  bufferLength()const{return _pCharBuf - &_charBuf[0];}
	void pauseProgressive();
	void parseSetup();
	void copyBufTo(XMLCh *name)const;
	void addAttribute();

private:
	OContentHandler *_fDocHandler;	// Current content handler.
	PFF _state,_resumeState;		// State with which to resume when calling paiseNext.
	std::istream *_in;					// Pointer to input stream.
	// Buffer length should be divisible by 3 and 4 in order to enable strings
	// consisting of BinHex and space separated hex (e.g. AE )to be send without
	// breaking in the middle of a character.
	enum {cCharBufLen = 204,cLocalNameLen = 100};
	OAttributes _attributes;		// Attributes vector.
	XMLCh _localName[cLocalNameLen];// Tag name.
	XMLCh _charBuf[cCharBufLen];	// Buffer of characters to send to content handler.
	XMLCh *_pCharBuf;				// Pointer to first free chracter in _charBuf.
	bool _progressive;				// True if parsing stops after every element.
	int _line;						// Last line number read.
	int _column;					// Last column read.
};

class OContentHandler
{
public:
	virtual void startElement(const XMLCh *localName,OAttributes *_attributes) = 0;
	virtual void endElement(const XMLCh *localName) = 0;
	virtual void characters(const XMLCh *,int len) = 0;
    virtual void fatalError(const OXMLReader::OXMLErr& exc) = 0;
    virtual ~OContentHandler(){}
};

#endif //OXMLREADER_H
