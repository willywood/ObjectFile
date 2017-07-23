//======================================================================
// 	This software may only be compiled in conjunction with a licence
// agreement with ObjectFile Limited.
// Copyright(c) 1996,97 ObjectFile Ltd.
//======================================================================

// OMeta represents information about a persistent class.
// It knows the following:
// (i)  A classes super classes.
// (ii) A classes sub-classes.
// (iii)How to construct an object of the class.



#include "odefs.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <typeinfo>
#include "ometa.h"
#include "opersist.h"

OClassId_t OMeta::_nmeta = 0;

// Some compilers require the array size to be defined here as well
// as in ometa.h
//OMeta *OMeta::_metaList[];
OMeta *OMeta::_metaList[cOMaxClasses];

bool OMeta::_initialized = false;

OMeta::OMeta(OClassId_t id,Func create,...):_cId(id),_create(create),_className(0)
// Constructor for OMeta.
// Parameters: id     - class identity of class to be represented.
//             create - read constructor for class.
//             ...    - class identities of superclasses of class.
{
	// Check that id is not already used (i.e there are two classes 
	// with the same id)
	// Also that we have not exceeded the maximum number of classes.
	// This can be increased in odefs.h
	oFAssert(_cId < cOMaxClasses && !_metaList[_cId - 1]);

	_metaList[_cId - 1] = this;
	_nmeta = max(_nmeta,_cId);

	va_list ap;
	va_start(ap,create);

	// Initialise _super (0 terminated super class list)
	for (int i = 0; i <= cMaxSupers ; i++)
   {
	   OClassId_t id = va_arg(ap,OClassId_t);
      if(id == 0)
      	break;
		// Maximum of cMaxSupers superclasses. Increase it here and in header file.
      oFAssert( i < cMaxSupers || (i == cMaxSupers && id == 0));
      _super[i] = id;
   }

	va_end(ap);

	// So that first allocation does not allocate full page
	_subclasses.reserve(3);

	_classSet.insert(id);

	// initialize again if already initialized.
	if(_initialized)
	{
		_initialized = false;
		initialize();
	}
}

void OMeta::initialize(void)
// Initialize meta classes static data structures.
{
	if(!_initialized){
		int i;
		// Empty the subclasses lists
		for(i = 0;i < _nmeta; i++){
			if(_metaList[i])
				_metaList[i]->_subclasses.erase(_metaList[i]->_subclasses.begin(),
												_metaList[i]->_subclasses.end());
		}

		for(i = 0;i < _nmeta; i++){
			if(_metaList[i])
				_metaList[i]->setSubclasses();
		}
		for(i = 0;i < _nmeta; i++){
			if(_metaList[i])
				_metaList[i]->getClassesDeep(_metaList[i]->_subclassesSet);
		 }
		_initialized = true;
	}
}

OMeta::~OMeta()
// Destructor. Must not be inline otherwise it crashes for some reason.
{}

void OMeta::setSubclasses(void)
// Set the sub-classes of this class.
{
	int i = 0;
	while(_super[i]){
		_metaList[_super[i] - 1]->setSubclass(this);
		i++;
	}
}
void OMeta::setSubclass(OMeta *subclass)
// Set a meta class as a sub-class of this meta class.
{
	_subclasses.push_back(subclass);
}

void OMeta::getClassesDeep(Classes &subclassesSet)const
// Private.
// Add to the subClassesSet all subclasses of this class.
{
	// Insert this class
	subclassesSet.insert(_cId);

	// Recurse on the subclasses
	for(Subclasses::const_iterator i = _subclasses.begin(); i != _subclasses.end(); ++i)
	{
		(*i)->getClassesDeep(subclassesSet);
	}
}

bool OMeta::isA(OClassId_t id)const
// Return true if this meta class represents a sub-class of the class
// with identity id.
{
	// If this is the meta class	
	if(id == _cId)
		return true;

	// Recurse up the super classes
	for(int i = 0; _super[i] != 0; i++)
	{
		if(_metaList[_super[i] - 1]->isA(id))
			return true;
	}
	return false;
}

const char *OMeta::className(OPersist *ob)
// Return a pointer to the class name of ob.
// Fixme: This function is dependant on the compiler because RTTI does not have to produce consistent names
// on different compilers. It may need some #ifdefs in order to produce an
// identical name on all compilers. 
// If this is not possible then we will have to consider having the developer pass a class name in the 
// constructor of OMeta.
{
	// Initialize _className the first time that it is needed.
	if(!_className && ob)
	{
		// RTTI - for this to work compiler must know that OPersist is polymorhic, so we need to
		// include opersist.h
		const char * typeName = typeid(*ob).name();

		// Find last blank separated string.
		int classNameStart;
#ifdef __GNUG__
		// On g++ compiler names take the form 6Person instead of Person
		for(classNameStart = 0;!isalpha(typeName[classNameStart]);classNameStart++);
#else
		// Find last blank separated string.
		for(classNameStart = strlen(typeName);classNameStart > 0 && typeName[classNameStart-1] != ' ';--classNameStart);
#endif
		_className = &typeName[classNameStart];
	}
	return _className;
}

OMeta *OMeta::meta(const char *className)
// Return a pointer to the meta class of the class names className or 0 if not found.
{
	for(int i = 0;i < _nmeta; i++){
		if(_metaList[i])
			if(_metaList[i]->_className && strcmp(_metaList[i]->_className,className) == 0)
				return _metaList[i]; 
	}
	return 0;
}
