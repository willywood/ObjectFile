#ifndef OX_H
#define OX_H
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



// Base class for all ObjectFile exceptions.
class OFileErr
{
public:
	OFileErr(const OFileErr &c);
	OFileErr(const char *msg);
    OFileErr(void);
	~OFileErr();
	const char *why(void)const{return _msg;}
protected:
	char *_msg;
};


class OFileIOErr : public OFileErr
// This is used where we do not have the file name available when throwing.
{
public:
	OFileIOErr(const char *msg);
	OFileIOErr(const char *fname,const char *msg);
	OFileIOErr(const OFileIOErr &c):OFileErr(c){};
private:
	void addSystemMessage(const char *msg,const char *systemMessage);
};


class OFileThresholdErr : public OFileIOErr
{
public:
	OFileThresholdErr(const char *msg):OFileIOErr(msg){}
	OFileThresholdErr(const OFileThresholdErr &c):OFileIOErr(c){};
};


#endif