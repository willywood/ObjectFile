#ifndef OMETA_H
#define OMETA_H
//======================================================================
// 	This software may only be compiled in conjunction with a licence
// agreement with ObjectFile Limited.
// Copyright(c) 1996-99 ObjectFile Ltd. 
//======================================================================



#include <vector>
#include <set>

#ifdef OFILE_STD_IN_NAMESPACE
using std::vector;
using std::set;
using std::less;
using std::max;
#endif


class OPersist;
class OMeta;

// These are needed to overide STL's destroy function, which does not compile
// for pointers.
inline void destroy(OMeta *) {}
inline void destroy(OMeta **) {}


class  OMeta{
public:
	typedef set<OClassId_t,less<OClassId_t> > Classes;

	OMeta(OClassId_t id,Func f,...);
	~OMeta();

	static void initialize(void);
    OClassId_t id(void)const{return _cId;}
	OPersist *construct(OIStream &in)const{return _create(in);}
	static OMeta *meta(OClassId_t id){return _metaList[id - 1];}
	static OMeta *meta(const char *className);
	const Classes &classes(bool deep=true)const{
		return deep?_subclassesSet:_classSet;}
	bool isA(OClassId_t id)const;
	const char *className(OPersist *ob);

private:
	typedef vector<OMeta *> Subclasses;
	enum {cMaxSupers = 4};    // Maximum number of super classes allowed.
							 
	void getClassesDeep(Classes &)const;
	void setSubclasses(void);
	void setSubclass(OMeta *);

	OClassId_t _cId;
	OClassId_t _super[cMaxSupers + 1];        // List of super classes
	Func _create;			  // Function to create objects of this class
	Subclasses _subclasses;   // Set of pointers to the sub-classes of this class
	Classes _subclassesSet;   // Set of class ids of sub-classes of this class
	Classes _classSet;        // Set of the single class id of this class
	const char *_className;	  // Pointer to class name.

	static OMeta *_metaList[cOMaxClasses];  // List of all meta classes
	static OClassId_t _nmeta;    // Last used element in meta class array.
	static bool _initialized; // true if initialized.
};



#endif
