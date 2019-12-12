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