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

// ObjectFile recognises that file management is an application dependant
// activity. For this reason it implements one of many possible
// strategies in a subclass of OFile. Other strategies can be implemented
// by sub-classing OFile.
//
// OUFile implements the following strategy:
//
// A work file is used to allow intermittent commits, without effecting the application
// file. This allows you to commit say, after every function, but only do a save
// when the user requests one. By doing this the application can always recover
// any changes that were made between the user doing a save.
//
// (i)	When an unamed file is specified in the constructor an empty work file is created.
// (ii)	When an named file is specified in the constructor an empty work file is created and
// the filename is set.
// (iii)When a named file is opened for writing	a work file is created which is a copy
// of the named file.
// (iv)	When a named file is opened for reading only, that file is opened.
// (v)	Any save operation can be performed only if the file is not readOnly.
// (vi)	Only when a file name is set, can the the file be saved.
// (vii) If a work file name is specified in the constructor then it is used
// as the name of the workfile. If 0 is specified a temporary file name is generated.
// The purpose of this is to allow the application to restore itself after an
// unplanned termination. The application looks for the named workfile. If found
// it can be queried and the application can possible be restored.
//
// OUFile also makes up for a deficiency in the UNIX file system (its inability to
// truncate files) by copying only the useful bytes from the workfile.
//
// The _dirty flag means that the state of the last saved file differs
// from that of the last commit or from the objects in memory. The
// OUFile will be not dirty when 2 conditions are met:
// (i)  No objects are dirty. 
// (ii) The file has been saved.
//
// For save() a backup file can be specified. This will copy the previous file
// to a named backup file e.g. <filename>.bak.
//
// creatWorkFile initializes a static instance of WorkFile. The reason for 
// this is that if an exception is thrown by OFile the destructor of the local
// instance _workFile will not be called. When the new try/catch syntax
// arrives it will be possible to catch exceptions in the constructor of 
// OUFile and call _sWorkFile.cleanup(). You can leave it until the next
// workfile is created to clean automatically up or until termination of 
// the program. However in the meantime if you want to
// delete the workFile after an exception you must catch the exception
// and call OUFile::cleanup() yourself.Sorry.
//
//

#include "odefs.h"
#include "oufile.h"
#include "oiter.h"
#include "ox.h"
#include "opersist.h"

OUFile::WorkFile OUFile::_sWorkFile;

void OUFile::fileCopy(const char *file1,const char *file2,OFilePos_t length)
// Copy file1 to file2.Overwrite file2.	file2 is truncated to length bytes.
// Parameters: file1 - file to copy from.
//             file2 - file to copy to.
//             length - maximum length of copied file
// Exceptions: If for some reason there is a failure to open one of the two file
//             OFileIOErr is thrown.
{
O_fd fd1,fd2;

	// Remove the old ~.PD file (it may not exist so do not check if this worked)
	remove(file2);
	// copy file1 to file2

	fd1 = o_fopen(file1,OFILE_OPEN_READ_ONLY);

	try
	{
		fd2 = o_fopen(file2,OFILE_OPEN_FOR_WRITING|OFILE_CREATE);
	}
	catch(...)
	{
		o_fclose(fd1);
		throw;
	}

	const size_t cBUFS = 64000;
	char *buf = new char[cBUFS];
	size_t n;
	OFilePos_t bytesToWrite = length;

	// Copy a maximimum of length bytes from fd1 to fd2.
	while((n = o_fread(buf,1,cBUFS,fd1)) > 0)
	{
		unsigned long w = (unsigned long)min(bytesToWrite,n);
		if(o_fwrite(buf,1,w,fd2) != w)
		{

			OFileIOErr x(file2,"Failed to write file.");

			// Cleanup
			delete []buf;
			o_fclose(fd1);
			o_fclose(fd2);
			// The file is incomplete so remove it.
			remove(file2);
			throw x;
		}

		// Stop copying if we have already copied length bytes.
		bytesToWrite -= w;
		if(bytesToWrite <= 0)
			break;
	}

	// Cleanup
	delete []buf;
	o_fclose(fd1);
	o_fclose(fd2);
}


OUFile::OUFile(const char *fname,long operation,const char *workName,const char *magicNumber):
			OFile(createWorkFile(fname,operation,workName),operation,magicNumber),
			_workFile(_sWorkFile)
// Parameters: fname - file name.
//             operation - file open flags.
//             workName - Name of workfile. If 0 then generate one.
// 			   magicNumber - a four byte character string identifying the magic 
// number of the file.(default = 0)
// This is used in many systems to distinguish between different types of files. It
// is also a way of ensuring you are reading the right type of file. If it is 0
// then no magicNumber is written or checked. If it is used and there is no match
// an exception is thrown.
// Note: createWorkFile is the main initialization method.
{
	// Convert from older versions
	convert();
}

const char *OUFile::createWorkFile(const char *fname,long operation,const char *workName)
// Generate a name for the file, and initialize object.
// Parameters: fname - file name.
//             operation - file open flags:
// OFILE_CREATE    -  Create a new file. This will overwrite an existing file of the same name.
// If the file cannot be created an exception will be thrown.
// OFILE_OPEN_FOR_WRITING - Open an existing file for writing. If the file does not
// exist an exception will be thrown.
// If or'ed with OFILE_CREATE and the file exists, the existing file will be opened.
// OFILE_OPEN_READ_ONLY - Open an existing file readonly. If the file does not exist
// an exception will be thrown.
//
// The following flags may be ore'ed together with one of the previous flags.
// OFILE_FAST_FIND - An extra index will be built, that will allow objects, of unspecified classes, to be found much more quickly than otherwise. This extra speed is particularly useful when many objects have to be connected in a file that is being loaded. It can improve file loading speed enourmously. The index requires extra memory, but this can be released by calling fastFindOff() after the objects have been connected, or at any other time.
// OFILE_AS_COMPOUND -  Force an OLE Compound Document to be created.
//
//             workName - Name of workfile. If 0 then generate one.
// Return a pointer to the work file name.
{
	// Assume an existing file. Unless reset later.
	_dirty = false;
	// Needed to ensure _workFile is initialized even if exception is thrown here.
    _workFile.init();

	switch(operation & 0x00000007L){

	case OFILE_OPEN_READ_ONLY:
		// A readonly file has no work file. It directly opens the application file.
		oFAssert(fname);
 		strcpy(_fileName,fname);
		return _fileName;

	case OFILE_OPEN_FOR_WRITING:
		// A file must be given when opening an existing file
		oFAssert(fname);

		// Create a workfile.
		_sWorkFile.create(workName);

		fileCopy(fname,_sWorkFile.name(),ULONG_MAX);
		strcpy(_fileName,fname);
		break;

	case OFILE_CREATE:

		// A new file is created dirty.
		_dirty = true;

		// Create a workfile.
		_sWorkFile.create(workName);

		if (fname)
		{
			strcpy(_fileName, fname);
		}
		else
		{
			_fileName[0] = '\0';
		}
		break;
	case OFILE_OPEN_FOR_WRITING | OFILE_CREATE:
		{
			// A file must be given when opening an existing file
			oFAssert(fname);

			// Create a workfile.
			_sWorkFile.create(workName);


			FILE *fd = fopen(fname,"rb");
			if(fd)
			// File exists
			{
				fclose(fd);
				fileCopy(fname,_sWorkFile.name(),ULONG_MAX);
			}
			else
			{
				// A new file is created dirty.
				_dirty = true;
			}

			strcpy(_fileName,fname);
		}
		break;

	default: oFAssert(0);   // Multiple open flags specified

	} // end switch

	return _sWorkFile.name();
}


OUFile::~OUFile()
{
	// close the file
	close();
}

void OUFile::makeWritable(const char *workName)
// Make a file that is open for reading, open for writing.
// Parameters: workName - Name of workfile. If 0 then generate one.
{
	oFAssert(isReadOnly());

	// Create a workfile.
	_workFile.create(workName);

	// Physically close the file.
	close();

	fileCopy(_fileName,_workFile.name(),ULONG_MAX);

	// Change to writable
	_operation &= ~OFILE_OPEN_READ_ONLY;
	_operation |= OFILE_OPEN_FOR_WRITING;

	// Physically open the newly created work file.
	reopen(_workFile.name());

}


void OUFile::saveAs(const char *name)
// Save the file with a new name.
// Parameter: New name of file.
// Exceptions: OFileIOErr will be thrown if the save failed.
{
	// Physically close the file.
	close();

	char oldFile[256];
	// If the file is read only we copy from it. If not we
	// copy from the workfile.
	strcpy(oldFile,isReadOnly() ? _fileName : _workFile.name());

	try{
		
		// copy the work file to name, truncating to the useful length.
		fileCopy(oldFile,name,getLength());

		// Physically open the file.
		reopen(oldFile);

		strcpy(_fileName,name);

		// file is now clean
		_dirty = false;

	}catch(OFileErr x){
		// Did not succeed so recover.
		reopen(oldFile);
		throw;
	}
}


void OUFile::checkSpaceFor(const char *fname,OFilePos_t length)const
// Private.
// Check there is enough space to create a file of size length
// with name, fname. 
// This is done by actually attempting to create a file of that length.
{
	// Throws OFileIOError on failure
	O_fd fp = o_fopen(fname,OFILE_OPEN_FOR_WRITING|OFILE_CREATE);

	// Try to create a file of size size
	if(o_setLength(fp,length))
	{
		// Success so delete it
		o_fclose(fp);
		remove(fname);
		return;
	}

	// Initialize before error code is wiped out
	OFileIOErr err(fname,"Not enough space.");

	// Clean up
	o_fclose(fp);
	remove(fname);

	throw err;
}

void OUFile::save(const char *backupFile)
// Save the current file.
// Parameter:
//			   backupFile - if non 0, create a backupfile with this name.
// Exceptions: OFileIOErr will be thrown if the save failed.
{
	// Must have a file name to save.
	oFAssert(strlen(_fileName) && !isReadOnly());

	// Check that we can write to the file. Throws OFileIOErr if not
	O_fd fd = o_fopen(_fileName,OFILE_OPEN_FOR_WRITING|OFILE_CREATE);
	o_fclose(fd);


	// Physically close the file.
	close();

	try{

		
		if(backupFile)
		{
			try
			{
        		// if we have a file to make a backup of
				fd = o_fopen(_fileName,OFILE_OPEN_READ_ONLY);
				o_fclose(fd);
				// make a copy of the original file. This can fail if there
				// is not enough space for the backup file.

				// Remove the backup file (there may not be one)
				remove(backupFile);
				// Check there is enough space to create a backup file
				checkSpaceFor(backupFile,getLength());
				// Rename the original file as the backup file.
				int res = rename(_fileName,backupFile);
				// This may fail if the backup file is on a different
				// device, so copy it.
				if(res != 0)
				{
					fileCopy(_fileName,backupFile,ULONG_MAX);
					remove(_fileName);
				}
			}
			catch(...)
			{ // Do nothing
			}
		}
		else
			remove(_fileName);
			

		// Try to rename the workfile to the file. This can fail if the
		// work file is on a different device. In that case copy it.
		if (rename(_workFile.name(), _fileName))
		{
			// copy the work file to _fileName, truncating to the useful length.
			fileCopy(_workFile.name(), _fileName, ULONG_MAX);
		}

		// copy the _fileName to work file, truncating to the useful length.
		fileCopy(_fileName,_workFile.name(),getLength());

		// Physically open the file.
		reopen(_workFile.name());

		// file is now clean
		_dirty = false;

	}catch(OFileErr x){
		// Did not succeed so recover.
		reopen(_workFile.name());
		throw;
	}
}


// Overridden virtual function.

void OUFile::commit(bool compact,bool wipeFreeSpace)
// Commit the file to the disk.
// Parameters: compact - obselete
// wipeFreeSpace - writes a character('\xFE) over the free space. 
//				   default is false. This should
//                 improve the compression ratio when zipped.
// Exceptions: OFileIOErr is thrown if the file cannot be written.
{
	inherited::commit(compact,wipeFreeSpace);
	
	// temporary file is now out of sync with saved file.	
	_dirty = true;
}

bool OUFile::isDirty(void)
// Return true if the state of the last saved file differs
// from that of the last commit or from the objects in memory. The
// OUFile will be not dirty when 2 conditions are met:
// (i)  No objects are dirty. 
// (ii) The file has been saved.
{
	if(!_dirty)
	{
		// Check if objects are in sync with temporary file.
		if (inherited::isDirty())
		{
			_dirty = true;
		}
	}

	return _dirty;
}

OUFile *OUFile::getOUFile(const char *fName)
// Static
// Return a pointer to the OUFile with file name fileName. Return
// 0 if it does not exist.
{
	OUFile *file = (OUFile *)OFile::getFirstOFile();

	while(file)
	{
		if (!strcmp(file->fileName(), fName))
		{
			return file;
		}

		file = (OUFile *)file->getNextOFile();
	}
	return 0;
}



// The purpose of this little class is to manage the destruction of the workfile.
OUFile::WorkFile::~WorkFile(void)
{
	cleanup();
}

void OUFile::WorkFile::cleanup(void)
{
	// Remove the work file if there is one.
	if(_name)
	{
 		remove(_name);
		delete []_name;
		_name = 0;
	}
}

void OUFile::WorkFile::create(const char *name)
// Create a Workfile.
// Parameter: name - Workfile name . if 0 invent a name.
// Throws an exception if it fails to create the file.
{
	cleanup();
	if(!name)
	{
		// Invent a name.
		_name = o_tmpnam(NULL,0);
	}
	else
	{
		// Use the given name
	   _name = new char[strlen(name) + 1];
		strcpy(_name,name);
	}
}

OUFile::WorkFile::WorkFile(WorkFile &w)
// Copy constructor. Actually transfers ownership. 
{
	_name = w._name;
	w._name = 0;
}
