#ifndef _TOOLBARCONTROL_H_
#define _TOOLBARCONTROL_H_

#include "Setting.h"
#include "ToolbarWindow.h"
#include "Toolbars.h"
#include "Events.h"

class CToolbarControl : public CSettingsHolder, public CEventObject
{
protected:
    CToolbarWindow *Toolbar1;                 //Main toolbar 1
	vector<CToolbar1Bar*> Toolbar1Bars;				  //Bar for childs

    CToolbarChannels *Toolbar1Channels;;      //Childs of the toolbar
    CToolbarVolume *Toolbar1Volume;
    CToolbarLogo *Toolbar1Logo;	

	typedef struct
	{
		int l;
		int t;
		int r;
		int b;
		int child_lr;
		int child_tb;
	} TToolbar1Margins;

	TToolbar1Margins	MarginsTop;
	TToolbar1Margins	MarginsBottom;
	TToolbar1Margins	MarginsDefault;
public:
    CToolbarControl(long SetMessage);
    ~CToolbarControl();

	virtual void OnEvent(CEventObject *pEventObject, eEventType Event, long OldValue, long NewValue, eEventType *ComingUp);

    void Set(HWND hWnd, LPCSTR szSkinName, int ForceHide = 0);
    void Adjust(HWND hWnd, BOOL bRedraw);
    void UpdateMenu(HMENU hMenu);
    void Free();
    void AdjustArea(LPRECT lpRect, int Crop);
	BOOL PtInToolbar(POINT Pt);
	int  CreateToolbar1Bar();

	BOOL ProcessToolbar1Selection(HWND hWnd, UINT uItem);

private:
    void CreateSettings(LPCSTR IniSection);

    DEFINE_YESNO_CALLBACK_SETTING(CToolbarControl, ShowToolbar1);
    DEFINE_SLIDER_CALLBACK_SETTING(CToolbarControl, Toolbar1Position);

    DEFINE_SLIDER_CALLBACK_SETTING(CToolbarControl, Toolbar1Channels);
    DEFINE_SLIDER_CALLBACK_SETTING(CToolbarControl, Toolbar1Volume);
};


#endif
