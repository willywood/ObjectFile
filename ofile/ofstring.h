#ifndef CSTRING_H
#define CSTRING_H
// This is a simplistic string class just to get the test
// programs compiled and working.
// If you have Borland C++ you can delete this file, because
// it uses the exact same interface as Borlands string.
// Do not use it as your string class


#include <string.h>
#include <wchar.h>

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
	bool operator<(const ofile_string& lhs)const
    {
   		return strcmp(lhs.c_str(),_data) < 0 ? true:false ;
	}


private:
	char *_data;
};


class ofile_wstring
{
public:
	ofile_wstring(void):_data(0){}
	ofile_wstring(const O_WCHAR_T *str)
	{
		_data = new O_WCHAR_T[wcslen(str) + 1];
		wcscpy(_data,str);
	}
	ofile_wstring(const ofile_wstring &from)
	{
		_data = new O_WCHAR_T[wcslen(from._data) + 1];
		wcscpy(_data,from._data);
	}
	~ofile_wstring(){delete []_data;}
	const O_WCHAR_T *wc_str(void)const{return _data;}
	ofile_wstring &operator=(const O_WCHAR_T *str)
	{
    	delete []_data;
		_data = new O_WCHAR_T[wcslen(str) + 1];
		wcscpy(_data,str);
		return *this;
	}
	bool operator<(const ofile_wstring& lhs)const
    {
   		return wcscmp(lhs.wc_str(),_data) < 0 ? true:false ;
	}


private:
	O_WCHAR_T *_data;
};
#endif
