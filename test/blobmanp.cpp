//===================================================================
// Simple demonstration program for BlobFile.
// All the files of a directory with the .ccp or .h extensions
// are saved to a file. This is then opened and copied to another file
// while removing from the original.
//===================================================================

#include "odefs.h"
#include "ofstring.h"
#include <dirent.h>
#include <iostream>
//#include <fstream.h>
#include <string.h>
#include <stdio.h>
#include "oufile.h"
#include "blobfilp.h"

using namespace std;

#define D_MAX_PATH 1024
#define ARRSIZE(a) sizeof(a)/sizeof(a[0])

const char *extensions[] = {".cpp",".h",".CPP",".H"};

OUFile *file = 0;


void createBlob(const char *fname)
// Create a BlobFile object and attach it to the file.
{
	for(int i = 0; i < ARRSIZE(extensions);i++)
	{
		if(strstr(fname,extensions[i]))
		{
			// Create a BlobFile object and attach it to the database.
			file->attach(new BlobFile(fname));
		}
	}
}


void dirwalk(char *dir,void (*fcn)(const char *))
// Traverse files in directory dir. Call function fcn for each one.
{
char name[D_MAX_PATH];
dirent *dp;
DIR *dfd;

	if((dfd = opendir(dir)) == NULL){
		cout << "cannot open " << dir << '\n';
		return;
	}

	while((dp = readdir(dfd)) != NULL) {
		if(strcmp(dp->d_name,".") == 0
		  || strcmp(dp->d_name,"..") == 0)
		  continue;  // skip self and parent

		if(strlen(dir) + strlen(dp->d_name) + 2 > sizeof(name))
			cout << name << dir << dp->d_name << " too long\n";

		else
		{
			sprintf(name,"%s/%s", dir , dp->d_name);
			(*fcn)(name);
		}

	}
	closedir(dfd);

}

int main()
{
	// Create a new file
	file = new OUFile("blobfile.db",OFILE_CREATE, "~blobfile.db");

	// Traverse files calling createBlob fro each one.
	dirwalk(".",createBlob);
	
	// Commit the file.
	file->commit();

	cout << "Total size of blob data = " << OBlobP::currentTotalMemoryUsage() << " bytes\n";
	// Save the file
	file->save();
	cout << "Total size of blob data = " << OBlobP::currentTotalMemoryUsage() << " bytes\n";

	
	delete file;
	cout << "Total size of blob data = " << OBlobP::currentTotalMemoryUsage() << " bytes\n";

	// Reopen the file
	file = new OUFile("blobfile.db",OFILE_OPEN_FOR_WRITING);
	// Open a new file
	OUFile *file2 = new OUFile("blobfile.cp",OFILE_CREATE, "~blobfile.cp");

	BlobFile::It it(file);
	BlobFile *p;
	long total = 0;

	// Iterate over all BlobFile objects, printing the name and size and transfering
	// them to file2.
	while(p = it++)
	{
		cout << p->name() << " = " << p->size() << '\n';
		// Calculate total size
		total += p->size();

		// Test the assignment operator (no other purpose)
		BlobFile bf = *p;

		// Make a copy and put it in the copy file
		file2->attach(new BlobFile(*p));

		// Now remove it from the first file.
		file->detach(p);
		delete p;

	}
	cout << "\n\nTotal size = " << total << " bytes\n";
	cout << "Total size of blob data = " << OBlobP::currentTotalMemoryUsage() << " bytes\n";

	// Commit and save the depleted file
	file->commit();
	cout << "Total size of blob data = " << OBlobP::currentTotalMemoryUsage() << " bytes\n";
	file->save();
	cout << "Total size of blob data = " << OBlobP::currentTotalMemoryUsage() << " bytes\n";
	delete file;
	cout << "Total size of blob data = " << OBlobP::currentTotalMemoryUsage() << " bytes\n";

	// Commit and save the copied file.
	file2->commit(true); // compress
	file2->save();
	delete file2;

	cout << "Total size of blob data = " << OBlobP::currentTotalMemoryUsage() << " bytes\n";

	// Both files are closed so the blobs should not be using any memory.
	oFAssert(OBlobP::currentTotalMemoryUsage() == 0);

	// Check validity of files
	file = new OUFile("blobfile.db",OFILE_OPEN_READ_ONLY, "~blobfile.db");
	file2 = new OUFile("blobfile.cp",OFILE_OPEN_READ_ONLY, "~blobfile.cp");
	cout << "Total size of blob data = " << OBlobP::currentTotalMemoryUsage() << " bytes\n";
	delete file;
	cout << "Total size of blob data = " << OBlobP::currentTotalMemoryUsage() << " bytes\n";
	delete file2;
	cout << "Total size of blob data = " << OBlobP::currentTotalMemoryUsage() << " bytes\n";

	file2 = new OUFile("blobfile.cp",OFILE_OPEN_FOR_WRITING, "~blobfile.cp");

	BlobFile::It it2(file2);

	// Iterate over all BlobFile objects, printing the name and size and transfering
	int i = 0;
	while(p = it2++)
	{
		// remove every other object
		if(i++ % 2 == 1)
		{
			file2->detach(p);
			delete p;
		}
	}
	file2->commit(false,true);
	file2->save();

	cout << "Total size of blob data = " << OBlobP::currentTotalMemoryUsage() << " bytes\n";

	delete file2;

	cout << "Total size of blob data = " << OBlobP::currentTotalMemoryUsage() << " bytes\n";

	return 0;
}

