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
#include "ox.h"
#include "oio.h"
#include <string.h>
#include <stdio.h>


OFileErr::OFileErr(void):_msg(0)
{}

OFileErr::~OFileErr()
{
	delete []_msg;
}

OFileErr::OFileErr(const char *msg)
{
	_msg = new char[strlen(msg) + 1];
	strcpy(_msg,msg);
}


OFileErr::OFileErr(const OFileErr &c)
{
	_msg = new char[strlen(c._msg) + 1];
	strcpy(_msg,c._msg);
}

void OFileIOErr::addSystemMessage(const char *msg,const char *systemMessage)
// Add the system message to the message.
{
	char *p = new char[strlen(msg) + strlen(systemMessage) + 4];
	if (strlen(systemMessage))
	{
		sprintf(p, "%s\n%s", msg, systemMessage);
	}
	else
	{
		strcpy(p, msg);
	}
	_msg = p;
}

OFileIOErr::OFileIOErr(const char *msg)
{
	char systemMessage[512];
	o_lastError(systemMessage,512);

	addSystemMessage(msg,systemMessage);
}

OFileIOErr::OFileIOErr(const char *fname,const char *msg)
{
	char systemMessage[512];
	o_lastError(systemMessage,512);

	char *p = new char[strlen(fname) + strlen(msg) + 4];
	sprintf(p,"%s : %s",fname,msg);
	addSystemMessage(p,systemMessage);
	delete []p;
}
