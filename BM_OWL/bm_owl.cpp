/*  Project bm_owl

	SUBSYSTEM:    bm_owl.exe Application
	FILE:         bm_owl.cpp
	AUTHOR:


	OVERVIEW
	========
	Benchmark for ObjectFile.
*/

#include "odefs.h"
#include "bm_owl.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <owl\eventhan.h>

#include "ofile.h"
#include "oiter.h"
#include "ox.h"
#include "mmyclass.h"



// Build a response table for all messages/commands handled
// by the application.
//
DEFINE_RESPONSE_TABLE1(ofilegApp, TApplication)
	EV_COMMAND(CM_HELPABOUT, CmHelpAbout),
END_RESPONSE_TABLE;


//////////////////////////////////////////////////////////
// ofilegApp
// =====
//
ofilegApp::ofilegApp () : TApplication("ObjectFile Benchmark")
{
}


ofilegApp::~ofilegApp ()
{
}


// Application intialization.
//
void ofilegApp::InitMainWindow ()
{
    if (nCmdShow != SW_HIDE)
        nCmdShow = (nCmdShow != SW_SHOWMINNOACTIVE) ? SW_SHOWNORMAL : nCmdShow;

    SDIDecFrame *frame = new SDIDecFrame(0, GetName(), 0, false);
    frame->SetFlag(wfShrinkToClient);

	frame->SetIcon(this, IDI_SDIAPPLICATION);

  
    SetMainWindow(frame);

}


SDIDecFrame::SDIDecFrame (TWindow *parent, const char far *title, TWindow *clientWnd, bool trackMenuSelection, TModule *module)
    : TDecoratedFrame(parent, title, clientWnd == 0 ? new ofilegTDLGClient(0) : clientWnd, trackMenuSelection, module)
{
}


SDIDecFrame::~SDIDecFrame ()
{
}


// Menu Help About ofileg.exe command
void ofilegApp::CmHelpAbout ()
{
}
int OwlMain (int , char* [])
{
    try {
        ofilegApp    app;
        return app.Run();
    }
    catch (xmsg& x) {
        ::MessageBox(0, x.why().c_str(), "Exception", MB_OK);
    }

    return -1;
}



class Timer{
public:
	void start(void){
		_start = clock();
	}
	float read(void){
		return((float)(clock() - _start)/CLOCKS_PER_SEC);
	}
private:
	clock_t _start;
};

DEFINE_RESPONSE_TABLE1(ofilegTDLGClient, TDialog)
  EV_CHILD_NOTIFY(IDSTART, BN_CLICKED, CmStart),
END_RESPONSE_TABLE;

ofilegTDLGClient::ofilegTDLGClient (TWindow *parent, TResId resId, TModule *module)
	 : TDialog(parent, resId, module)
{
  _nClasses = 10;
  _nObjects = 1000;
  _avSize = 100;

  _enClasses = new TEdit(this, IDC_NCLASSES);
  _enObjects = new TEdit(this, IDC_NOBJECTS);
  _eavSize = new TEdit(this,IDC_AVSIZE);

	_sinsert = new TStatic(this,IDINSERT);
	_saccess = new TStatic(this,IDACCESS );
	_ssave = new TStatic(this,IDSAVE);
	_sload = new TStatic(this,IDLOAD);
	_serase = new TStatic(this,IDERASE);
	_sall = new TStatic(this,IDALL);

	_sinsertt = new TStatic(this,IDINSERTT);
	_saccesst = new TStatic(this,IDACCESST);
	_ssavet = new TStatic(this,IDSAVET);
	_sloadt = new TStatic(this,IDLOADT);
	_seraset = new TStatic(this,IDERASET);
	_sallt = new TStatic(this,IDALLT);
}


ofilegTDLGClient::~ofilegTDLGClient ()
{
	 Destroy();
}

//
// SetupWindow(). Initialize dialog fields with given fields from
// TAppProperties record.
//
void
ofilegTDLGClient::SetupWindow()
{
  char str[20];
  TDialog::SetupWindow();

  // Initialize dialog fields with data from AppProperties record.
  //
  sprintf(str,"%ld",_nClasses);
  _enClasses->Insert(str);
  sprintf(str,"%ld",_nObjects);
  _enObjects->Insert(str);
  sprintf(str,"%ld",_avSize);
  _eavSize->Insert(str);


}


#define CREATE_OBJECT(n)   \
	p[obCount++] = new MyClass##n(rand() % _avSize*2); \
	if(++classCount == _nClasses) continue;


void
ofilegTDLGClient::CmStart()
{
char str[20];
	_enClasses->GetLine(str,20,0);
	_nClasses = atol(str);
	_enObjects->GetLine(str,20,0);
	_nObjects = atol(str);
	_eavSize->GetLine(str,20,0);
	_avSize = atol(str);


long obCount = 0;
OPersist **p = new OPersist*[_nObjects];
long i;

	long loops = (_nClasses != 0) ? _nObjects/_nClasses : 0;
	for(i = 0; i < loops ; i++){
		long classCount = 0;

		// Create objects of different classes.
		CREATE_OBJECT(10);
		CREATE_OBJECT(11);
		CREATE_OBJECT(12);
		CREATE_OBJECT(13);
		CREATE_OBJECT(14);
		CREATE_OBJECT(15);
		CREATE_OBJECT(16);
		CREATE_OBJECT(17);
		CREATE_OBJECT(18);
		CREATE_OBJECT(19);
		CREATE_OBJECT(20);
		CREATE_OBJECT(21);
		CREATE_OBJECT(22);
		CREATE_OBJECT(23);
		CREATE_OBJECT(24);
		CREATE_OBJECT(25);
		CREATE_OBJECT(26);
		CREATE_OBJECT(27);
		CREATE_OBJECT(28);
		CREATE_OBJECT(29);
		CREATE_OBJECT(30);
		CREATE_OBJECT(31);
		CREATE_OBJECT(32);
		CREATE_OBJECT(33);
		CREATE_OBJECT(34);
		CREATE_OBJECT(35);
		CREATE_OBJECT(36);
		CREATE_OBJECT(37);
		CREATE_OBJECT(38);
		CREATE_OBJECT(39);
		CREATE_OBJECT(40);
		CREATE_OBJECT(41);
		CREATE_OBJECT(42);
		CREATE_OBJECT(43);
		CREATE_OBJECT(44);
		CREATE_OBJECT(45);
		CREATE_OBJECT(46);
		CREATE_OBJECT(47);
		CREATE_OBJECT(48);
		CREATE_OBJECT(49);
		CREATE_OBJECT(50);
		CREATE_OBJECT(51);
		CREATE_OBJECT(52);
		CREATE_OBJECT(53);
		CREATE_OBJECT(54);
		CREATE_OBJECT(55);
		CREATE_OBJECT(56);
		CREATE_OBJECT(57);
		CREATE_OBJECT(58);
		CREATE_OBJECT(59);
		CREATE_OBJECT(60);
		CREATE_OBJECT(61);
		CREATE_OBJECT(62);
		CREATE_OBJECT(63);
		CREATE_OBJECT(64);
		CREATE_OBJECT(65);
/*		CREATE_OBJECT(66);   // My compiler breaks down here
		CREATE_OBJECT(67);
		CREATE_OBJECT(68);
		CREATE_OBJECT(69);
		CREATE_OBJECT(70);
		CREATE_OBJECT(71);
		CREATE_OBJECT(72);
		CREATE_OBJECT(73);
		CREATE_OBJECT(74);
		CREATE_OBJECT(75);
		CREATE_OBJECT(76);
		CREATE_OBJECT(77);
		CREATE_OBJECT(78);
		CREATE_OBJECT(79);
		CREATE_OBJECT(80);
		CREATE_OBJECT(81);
		CREATE_OBJECT(82); */
	}

char *fmt = "%.3f";
Timer timer;
float time;
float totTime  = 0.0;

	// Create a temporary file
	OFile *f = new OFile("ofile.tst",OFILE_CREATE|OFILE_FAST_FIND);

	// Insertion
	timer.start();
	for(i = 0; i < obCount ; i++){
//*******************************************************************
		f->attach(p[i]);
//*******************************************************************
	}
	time = timer.read();
	totTime += time;
	sprintf(str,fmt,time);
	_sinsertt->SetText(str);
	sprintf(str,fmt,time*1000/obCount);
	_sinsert->SetText(str);

	// Access
	timer.start();
	for(i = 0; i < obCount ; i++){
	// Worst case scenario, by specifying lowest possible resolution.
//*******************************************************************
		OPersist *ob = f->getObject(p[i]->oId(),cOPersist);
//*******************************************************************
	}
	time = timer.read();
	totTime += time;
	sprintf(str,fmt,time);
	_saccesst->SetText(str);
	sprintf(str,fmt,time*1000/obCount);
	_saccess->SetText(str);

	// Save
	timer.start();
//*******************************************************************
	try{
		f->commit();
	}catch(OFileErr &x){
		MessageBox(x.why());
	}
//*******************************************************************
	time = timer.read();
	totTime += time;
	sprintf(str,fmt,time);
	_ssavet->SetText(str);
	sprintf(str,fmt,time*1000/obCount);
	_ssave->SetText(str);


	delete f;
	delete []p;

	// Load
	p = new OPersist*[_nObjects + 1];
	timer.start();
//*******************************************************************
	//  Reopen the file
	f = new OFile("ofile.tst",OFILE_OPEN_READ_ONLY);
	// Declare iterator for all objects in the file
	OIterator it(f);
	i = 0;
	// Iterate over objects to make sure each one is brought into memory.
	while(p[i++] = it++){}
//*******************************************************************
	if(i-1 != obCount)
		oFAssert(i-1 == obCount);
	time = timer.read();
	totTime += time;
	sprintf(str,fmt,time);
	_sloadt->SetText(str);
	sprintf(str,fmt,time*1000/obCount);
	_sload->SetText(str);

	// Erase
	timer.start();
	for(i = 0; i < obCount ; i++){
//*******************************************************************
		// Erase the object from the file
		f->detach(p[i]);
//*******************************************************************
	}
	time = timer.read();
	totTime += time;
	sprintf(str,fmt,time);
	_seraset->SetText(str);
	sprintf(str,fmt,time*1000/obCount);
	_serase->SetText(str);

	sprintf(str,fmt,totTime);
	_sallt->SetText(str);
	sprintf(str,fmt,totTime*1000/obCount);
	_sall->SetText(str);

	delete f;

	// Delete the now non-persistent objects.
	for(i = 0; i < obCount ; i++)
		delete p[i];
	delete []p;

}

