#ifndef OUFILE_H
#define OUFILE_H
//======================================================================
// 	This software may only be compiled in conjunction with a licence
// agreement with ObjectFile Limited.
// Copyright(c) 1996-99 ObjectFile Ltd. 
//======================================================================

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

	static void fileCopy(const char *file1,const char* file2,unsigned long length);
	
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