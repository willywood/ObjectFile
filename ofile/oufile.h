#ifndef OUFILE_H
#define OUFILE_H
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


#include "ofile.h"

class OUFile : public OFile
{
typedef OFile inherited;

class WorkFile
// WorkFile manages the existence of the workfile.
{
public:
	WorkFile(void):_name(0){}
	~WorkFile(void);
	WorkFile(WorkFile &w);
    void init(void){_name = 0;}
	const char *name(void)const{return(_name);}
	void create(const char *name);
 	void cleanup(void);
private:
	char *_name;
};

public:
	OUFile(const char *fname,long operation,const char *workName = 0,
											const char *magicNumber = 0);
	~OUFile(void);
	
	// Overridden virtual function.
	void commit(bool compact = false,bool wipeFreeSpace = false);
	bool isDirty(void);

	// Overridable functions
	virtual void saveAs(const char *name);
	virtual void save(const char *backupFile = 0);

	const char *fileName(void)const{return _fileName;}
	const char *workFileName(void)const{return _workFile.name();}

	static void fileCopy(const char *file1,const char* file2,OFilePos_t length);
	
	static OUFile *getOUFile(const char *fileName);

	static void cleanup(void){_sWorkFile.cleanup();}
	void makeWritable(const char *workFile = 0);

protected:
	void convert(void);

private:
	const char *createWorkFile(const char *fname,long operation,const char *workName);
	void checkSpaceFor(const char *fname,OFilePos_t length)const;

private:
	// Replace with standard string class.
	char _fileName[256];       // File name
	WorkFile _workFile;		   // Workfie name
	static WorkFile _sWorkFile;// Temporary
	bool _dirty;			   // true if file has been changed
};

#endif