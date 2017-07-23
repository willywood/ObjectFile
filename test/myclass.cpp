#include "odefs.h"
#include <iostream>
#include "myclass.h"
#include "ostrm.h"

using namespace std;

OMeta MyClass::_metaClass(cMyClass,(Func)MyClass::New,cOPersist,0);
OMeta MyClass1::_metaClass(cMyClass1,(Func)MyClass1::New,cOPersist,0);
OMeta MyClass2::_metaClass(cMyClass2,(Func)MyClass2::New,cOPersist,0);
OMeta MyClass3::_metaClass(cMyClass3,(Func)MyClass3::New,cOPersist,0);
OMeta MyClass4::_metaClass(cMyClass4,(Func)MyClass4::New,cOPersist,0);


MyClass::MyClass(OIStream *in):OPersist(in)
// Object file read constructor
{
	in->readCString(dum,sizeof(dum),"dum");
	for(int i = 0; i < 4;i++)
//		m[i] = (MyClass *)in->readObject();
		in->readObject((OPersist **)&m[i],"m");

}
MyClass::MyClass(int n)
{
int i;
	int nchars = min(99,n);
	for(i = 0; i < nchars;i++)
		dum[i] = i % 10 + 48;

	dum[i] = '\0';
	for(int j = 0;j < 4;j++)
	m[j] = new MyClass2();

}

void MyClass::oWrite(OOStream *out)const
{
	inherited::oWrite(out);

	out->writeCString(dum,"dum");
	for(int i = 0;i < 4;i++)
		out->writeObject(m[i],"m");

}

void MyClass::oAttach(OFile *file,bool deep)
{
	inherited::oAttach(file,deep);

	if(deep){
		for(int i = 0;i < 4;i++)
			m[i]->oAttach(file,deep);
	}

}

void MyClass::oDetach(OFile *file,bool deep)
{
	if(deep){
		for(int i = 0;i < 4;i++)
			m[i]->oDetach(file,deep);
	}
	inherited::oDetach(file,deep);
}
void MyClass::print(void)
{
	cout << "MyClass "<< oId() <<"-";
	for(int i = 0;i < 4;i++)
		cout << m[i]->oId() << " ";
	cout <<'\n';
}

void MyClass1::oAttach(OFile *file,bool deep)
{
	inherited::oAttach(file,deep);

	_blob.oAttach(file);

}

void MyClass1::oDetach(OFile *file,bool deep)
{
	inherited::oDetach(file,deep);

	_blob.oDetach(file);

}


MyClass2::MyClass2(OIStream *in):OPersist(in)
// Object file read constructor
{
	_ddata = in->readDouble("_ddata");
	_fdata = in->readFloat("_fdata");
	_unicodeStr1 = in->readWCString256("_unicodeStr1");
	_unicodeStr2 = in->readWCString("_unicodeStr2");
}

void MyClass1::oWrite(OOStream *out)const
{
	inherited::oWrite(out);

	_blob.oWrite(out,"_blob");

}


void MyClass2::oWrite(OOStream *out)const
{
	inherited::oWrite(out);

	out->writeDouble(_ddata,"_ddata");
	out->writeFloat(_fdata,"_fdata");
	out->writeWCString256(_unicodeStr1.wc_str(),"_unicodeStr1");
	out->writeWCString(_unicodeStr2.wc_str(),"_unicodeStr2");
}


// This demonstrates the use of a collection of owned objects(_properties)
// and a collection of pointed to objects (_children).
// By attaching or erasing with deep true, a whole tree of objects can
// be attached or detachd from the file. _properties are always attached
// or deleted with the owner.


MyClass3::MyClass3(OIStream *in):OPersist(in),_children(in,"_children"),_properties(in,"_properties")
// Object file read constructor
{
}

MyClass3::~MyClass3(void)
{
	// Only destroy the properties since children are not owned by this class
	if(!oAttached())
		_properties.destroy();
}

void MyClass3::oWrite(OOStream *out)const
{
	inherited::oWrite(out);

	_children.oWrite(out,"_children");
	_properties.oWrite(out,"_properties");
}

void MyClass3::oAttach(OFile *file,bool deep)
{
	inherited::oAttach(file,deep);

	if(deep)
		_children.oAttach(file,deep);

	_properties.oAttach(file,deep);
}

void MyClass3::oDetach(OFile *file,bool deep)
{
	if(deep)
		_children.oDetach(file,deep);

	_properties.oDetach(file,deep);

	inherited::oDetach(file,deep);
}

void MyClass3::addChild(MyClass3 *child)
{
	_children.insert(child);
	oSetDirty();
}
void MyClass3::addProperty(MyClass3 *property)
{
	_properties.insert(property);
    oSetDirty();
}
