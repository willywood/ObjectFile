//======================================================================
// 	This software may only be compiled in conjunction with a licence
// agreement with ObjectFile Limited.
// Copyright(c) 1996 ObjectFile Ltd. 
//======================================================================

#include "odefs.h"
#include "oufile.h"
#include "oiter.h"
#include "ox.h"

void OUFile::convert(void)
// Convert objects from older versions. 
// This is the place to put object evolution code.
{
   // Check that we are not trying to convert a file that is more up to date
   // than the source code.
	if(userSourceVersion() < userVersion())
		throw OFileIOErr(_fileName,"The application file has a newer version than this"
									"program.");
	// This is the most universal case.	If the affected classes are known
	// then it is only necassary to read objects of those classes.
	if(userSourceVersion() > userVersion())
	{
		// Make sure every object is read so that it can be converted.
		OIterator it(this,cOPersist);
		while(it++);

	   if(!isReadOnly())
   	{
			// Might like to commit and purge to release memory
			commit();
			purge();
	   }
	}
}