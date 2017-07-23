#ifndef OX_H
#define OX_H
//======================================================================
// 	This software may only be compiled in conjunction with a licence
// agreement with ObjectFile Limited.
// Copyright(c) 1996,97,98 ObjectFile Ltd. 
//======================================================================


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