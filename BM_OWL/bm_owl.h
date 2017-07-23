#if !defined(__bm_owl_h)              // Sentry, use file only if it's not already included.
#define __bm_owl_h

/*  Project ofileg


	SUBSYSTEM:    bm_owl.exe Application
	FILE:         bm_owl.h
    AUTHOR:       


	OVERVIEW
    ========
	*/

#include <owl\applicat.h>
#include <owl\decframe.h>
#include <owl\dialog.h>
#include <owl\static.h>
#include <owl\edit.h>

#include <owl\applicat.h>
#include <owl\decframe.h>


#include "bm_owl.rh"            // Definition of all resources.


//
// FrameWindow must be derived to override Paint for Preview and Print.
//
//{{TDecoratedFrame = SDIDecFrame}}
class SDIDecFrame : public TDecoratedFrame {
public:
    SDIDecFrame (TWindow *parent, const char far *title, TWindow *clientWnd, bool trackMenuSelection = false, TModule *module = 0);
    ~SDIDecFrame ();
};    //{{SDIDecFrame}}


//{{TApplication = ofilegApp}}
class ofilegApp : public TApplication {
private:

public:
    ofilegApp ();
    virtual ~ofilegApp ();

//{{ofilegAppVIRTUAL_BEGIN}}
public:
	virtual void InitMainWindow();
protected:
    void CmHelpAbout ();

DECLARE_RESPONSE_TABLE(ofilegApp);
};    //{{ofilegApp}}



//{{TDialog = ofilegTDLGClient}}
class ofilegTDLGClient : public TDialog {
public:
	 ofilegTDLGClient (TWindow *parent, TResId resId = IDD_CLIENT, TModule *module = 0);
    virtual ~ofilegTDLGClient ();
	void SetupWindow();
    void CmStart();
	void EvCompacting(int compactRatio);


private:
	TEdit *_enClasses,*_enObjects,*_eavSize;
	long  _nClasses,_nObjects,_avSize;
	TStatic *_sinsert,*_saccess,*_ssave,*_sload,*_serase,*_sall;
	TStatic *_sinsertt,*_saccesst,*_ssavet,*_sloadt,*_seraset,*_sallt;

  DECLARE_RESPONSE_TABLE(ofilegTDLGClient);

};    //{{ofilegTDLGClient}}


#endif                                      // __bm_owl_h sentry.
