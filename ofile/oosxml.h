#ifndef oosxml_h
#define oosxml_h
//======================================================================
// 	This software may only be compiled in conjunction with a licence
// agreement with ObjectFile Limited.
// Copyright(c) 1996-2001 ObjectFile Ltd. 
//======================================================================



#include "ostrm.h"
#include "ox.h"
#include <map>
#include <set>
#include "ofstring.h"

#ifdef OFILE_STD_IN_NAMESPACE
using std::map;
using std::set;
using std::less;
#endif

class OOStreamXML: public OOStream
{
typedef OOStream inherited;
friend class OPersist;
friend class OFile;
friend class FreeList;
public:

	enum XMLStyle {cBest,  // Take account of "o:" only
				   cSAX,   // SAX readable. Objects appear before references
				   cFlat   // All references appear as references (even if "o:"
				   };
	enum {cLabelLength = 256}; // maximum number of characters in a label.

	OOStreamXML(OFile *f,std::ostream &out,bool writeBlobsToFile=false);
	~OOStreamXML(void);
protected:
	void writeLong(O_LONG data,const char *label = 0);
	void writeLong64(O_LONG64 data,const char *label = 0);
	void writeFloat(float data,const char *label = 0);
	void writeDouble(double data,const char *label = 0);
	void writeShort(O_SHORT data,const char *label = 0);
	void writeChar(char data,const char *label = 0);
	void writeBool(bool data,const char *label = 0);
	void writeCString(const char * str,const char *label = 0);
	void writeCString256(const char * str,const char *label = 0);
	// wchar support  can be removed if not used.
	void writeWChar(O_WCHAR_T data,const char *label = 0);
	void writeWCString(const O_WCHAR_T * str,const char *label = 0);
	void writeWCString256(const O_WCHAR_T * str,const char *label = 0);
	//
	void writeBytes(const void *buf,size_t nBytes,const char *label = 0);
	void writeBits(const void *buf,size_t nBytes,const char *label = 0);
	void writeObjectId(OId,const char *label = 0);
	void writeObject(OPersist *,const char *label = 0);
	bool writeBlob(void *buf,OFilePos_t mark,unsigned long size,const char *label = 0);
    void writeBlobHeader(OFilePos_t mark,oulong blobLength);
	void writeFile(const char *fname,OFilePos_t mark,oulong from,oulong size){OFILE_UNUSED(fname);OFILE_UNUSED(mark);OFILE_UNUSED(from);OFILE_UNUSED(size);}
	// Stream is actually writing to the file.
	bool writing(void)const{return true;}

public:
	void setXMLStyle(XMLStyle style){_style = style;}
	void setBlobDirectory(const char *blobDirectory);
	void setIndent(bool indent = true){_indent = indent;}
	void writeObjects(const char *appName,
				  OClassId_t classId = cOPersist,
				  bool deep = true,
				  const char *encoding = 0);
	void writeObjectAsXML(OPersist *ob);
	void beginDocument(const char *appName,const char *encoding = 0);
	void endDocument(void);

	void comment(const char *text = 0);
	void beginObject(const char *label);
	void endObject(void);
	void reset(void);

protected:
	void start(OPersist *ob);
	void finish(void);

	OPersist *currentObject(void)const{return _currentObj;}
	OFile *fromFile(void)const{return _fromFile;}
	void writeEscapedString(const char *str);
	void writeCDATA(const char *text);

private:
	const char *setLabel(const char *label);

	void startData(const char *label,const char *attributes=0);
	void endData(void);
	void indent(int level)const;
	void writeObjectReference(OId id,const char *label);
	void writeObjectInstance(OPersist *,const char *label);
	const O_WCHAR_T *pwriteWCString(const O_WCHAR_T * str);

private:
	std::ostream &_out;			  // Output stream
	char _basicLabel[cLabelLength];	  // Store the label until the attribute is terminated
	const char *_labelStack[500]; // Label stack
	int _sp;				  // Label stack pointer
	OPersist *_currentObj;    // pointer to the object currently being written
	set<OId,less<OId> > _writtenObjects; // Set of objects that have been written already.
	XMLStyle _style;          // XML object reference style
	bool _indent;             // true if XML is indented for easy reading
	bool _writeBlobsToFile;	  // true if blobs are to be written to a file
	OFile * _fromFile;		  // The OFile from which the objects are written
	char _blobDirectory[256]; // The directory to which the blob data files are written

	typedef  map<ofile_string,int,less<ofile_string> > Dictionary;
    Dictionary _blobLabels;   // Map of blob labels and indexes used to generate unique
                              // file names
};

#endif  // oosxml_h
