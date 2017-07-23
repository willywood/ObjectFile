#ifndef MYCLASS_H
#define MYCLASS_H

#include "opersist.h"
#include "myblob.h"
#include "oset.h"
#include "ofstring.h"

const OClassId_t cMyClass =  10;
const OClassId_t cMyClass1 = 11;
const OClassId_t cMyClass2 = 12;
const OClassId_t cMyClass3 = 13;
const OClassId_t cMyClass4 = 14;


class MyClass1 : public OPersist
{
typedef OPersist inherited;
public:
	MyClass1(OIStream * c):OPersist(c),_blob(c){}
	MyClass1(long size):_blob(size){}

	void oAttach(OFile *file,bool);
	void oDetach(OFile *file,bool);

	OMeta *meta(void)const{return &_metaClass;}
protected:
	void oWrite(OOStream *)const;
private:
	MyBlob _blob;
private:
	// Instantiation function
	static OPersist *New(OIStream *s){return new MyClass1(s);}
	static OMeta _metaClass;
};

const O_WCHAR_T u[]={0x5668,0x56DF,0xFB33,0xFB2C};
class MyClass2 : public OPersist
{
typedef OPersist inherited;
public:
	MyClass2(OIStream * c);
	MyClass2():_ddata(100.0),_fdata(200.0),
				_unicodeStr1(u),
				_unicodeStr2(L"UNICODE2"){}
	OMeta *meta(void)const{return &_metaClass;}
protected:
	void oWrite(OOStream *)const;

private:
	double _ddata;
	float _fdata;
	ofile_wstring _unicodeStr1;
	ofile_wstring _unicodeStr2;
	// Instantiation function
	static OPersist *New(OIStream *s){return new MyClass2(s);}
	static OMeta _metaClass;
};

class MyClass3 : public OPersist
{
typedef OPersist inherited;
public:
	MyClass3(void):_children(),_properties(){}
	MyClass3(OIStream * c);
	~MyClass3(void);

	void oAttach(OFile *file,bool);
	void oDetach(OFile *file,bool);

	OMeta *meta(void)const{return &_metaClass;}

	void addChild(MyClass3 *);
	void addProperty(MyClass3 *);

    const OSet &children()const{return _children;}

protected:
	void oWrite(OOStream *)const;

private:
	// Instantiation function
	static OPersist *New(OIStream *s){return new MyClass3(s);}
	static OMeta _metaClass;

	OSet _children;
	OSet _properties;
};



class MyClass4 : public OPersist
{
typedef OPersist inherited;
public:
	MyClass4(OIStream * c):OPersist(c){}
	MyClass4(){}
	OMeta *meta(void)const{return &_metaClass;}

private:
	// Instantiation function
	static OPersist *New(OIStream *s){return new MyClass4(s);}
	static OMeta _metaClass;
};


class MyClass : public OPersist
{
typedef OPersist inherited;
public:
	MyClass(OIStream *c);
	MyClass(int n);
	void print(void);

	void oAttach(OFile *file,bool);
	void oDetach(OFile *file,bool);

	OMeta *meta(void)const{return &_metaClass;}

protected:
	void oWrite(OOStream *)const;
private:
	// Instantiation function
	static OPersist *New(OIStream *s){return new MyClass(s);}

	static OMeta _metaClass;

	// Persistant Object data
	char dum[100];
	OPersist *m[4];

};
#endif
