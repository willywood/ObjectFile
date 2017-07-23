#ifndef MYCOLCT_H
#define MYCOLCT_H

#include "opersist.h"
#include <list>

#ifdef OFILE_STD_IN_NAMESPACE
using std::list;
#endif

typedef list<OPersist *>  MyObjectList;

const OClassId_t cMyCollect =  20;

class MyCollect : public OPersist,public MyObjectList
{
typedef OPersist inherited;
public:
	// Read constructor
	MyCollect(OIStream *);
	// Default Constructor
	MyCollect(void):OPersist(),MyObjectList(){}
	~MyCollect(void);

	void oAttach(OFile *file,bool);
	void oDetach(OFile *file,bool);

	OMeta *meta(void)const{return &_metaClass;}

protected:

	void oWrite(OOStream *)const;

private:

	// Instantiation function
	static OPersist *New(OIStream *s){return new MyCollect(s);}
	// Meta class
	static OMeta _metaClass;

};


//===================== Streamable collection ==================


class MySCollect : public MyObjectList
{
public:
	// Read constructor
	MySCollect(OIStream *);
	// Default Constructor
	MySCollect(void){}
	void destroy(void);

	void oAttach(OFile *file,bool);
	void oDetach(OFile *file,bool);

	void oWrite(OOStream *)const;

};

#endif
