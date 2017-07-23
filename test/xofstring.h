#ifndef CSTRING_H
#define CSTRING_H
// This is a simplistic string class just to get the test
// programs compiled and working.
// If you have Borland C++ you can delete this file, because
// it uses the exact same interface as Borlands string.
// Do not use it as your string class


#include <string.h>

class ofile_string
{
public:
	ofile_string(void):_data(0){}
	ofile_string(const char *str)
	{
		_data = new char[strlen(str) + 1];
		strcpy(_data,str);
	}
	ofile_string(const ofile_string &from)
	{
		_data = new char[strlen(from._data) + 1];
		strcpy(_data,from._data);
	}
	~ofile_string(){delete []_data;}
	const char *c_str(void)const{return _data;}
	ofile_string &operator=(const char *str)
	{
    	delete []_data;
		_data = new char[strlen(str) + 1];
		strcpy(_data,str);
		return *this;
	}


private:
	char *_data;
};

#endif