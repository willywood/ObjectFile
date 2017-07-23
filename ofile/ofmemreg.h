#ifndef OFMEMREG_H
#define OFMEMREG_H
//======================================================================
// 	This software may only be compiled in conjunction with a licence
// agreement with ObjectFile Limited.
// Copyright(c) 1996,97 ObjectFile Ltd. 
//======================================================================


class OFMemoryRegister
{
public:
	void add(long nBytes);
	void subtract(long nBytes);
	unsigned long size(void)const{return _nBytes;}

private:
	unsigned long _nBytes;
};

#endif