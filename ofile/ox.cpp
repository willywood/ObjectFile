//======================================================================
// 	This software may only be compiled in conjunction with a licence
// agreement with ObjectFile Limited.
// Copyright(c) 1996-2000 ObjectFile Ltd. 
//======================================================================


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
	if(strlen(systemMessage))
		sprintf(p,"%s\n%s",msg,systemMessage);
	else
		strcpy(p,msg);
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
