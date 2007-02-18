/////////////////////////////////////////////////////////////////////////////
// $Id: IOutput.h,v 1.4 2007-02-18 15:02:16 robmuller Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 John Adcock.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////
//
//  This file is subject to the terms of the GNU General Public License as
//  published by the Free Software Foundation.  A copy of this license is
//  included with this software distribution in the file COPYING.  If you
//  do not have a copy, you may obtain a copy by writing to the Free
//  Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//
//  This software is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __IOUTPUT_H___
#define __IOUTPUT_H___

#include "settings.h"

#define DSCALER_MAX_WIDTH 1024
#define DSCALER_MAX_HEIGHT 768

class IOutput;

extern IOutput *ActiveOutput; // currently active output .. may be overlay or d3d


enum OUTPUTTYPES { OUT_OVERLAY, OUT_D3D };

SETTING* Overlay_GetSetting(OTHER_SETTING Setting);
void Overlay_ReadSettingsFromIni();
void Overlay_WriteSettingsToIni(BOOL bOptimizeFileAccess);



// Interface for an output to screen (currently overlay, d3d9)
class IOutput
{
protected:
    bool m_bSettingInitialized;
    SETTING OtherSettings[OTHER_SETTING_LASTONE];   
    
public:
	DWORD DestSizeAlign;
	DWORD SrcSizeAlign;
    


	IOutput(void);
	virtual ~IOutput(void);

	// Get Hold of the Other.c file settings

	virtual void SetCurrentMonitor(HWND hWnd)=0;
	virtual void CheckChangeMonitor(HWND hWnd)=0;
	virtual void GetMonitorRect(HWND hWnd, RECT* rect);

	virtual void ExitDD(void)=0;
	virtual BOOL InitDD(HWND hWnd)=0;
	virtual BOOL CanDoOverlayColorControl()=0;
	virtual BOOL OverlayActive()=0;
	virtual BOOL Overlay_Update(LPRECT pSrcRect, LPRECT pDestRect, DWORD dwFlags)=0;
	virtual BOOL Overlay_Create()=0;
	virtual void Overlay_Clean()=0;
	virtual BOOL Overlay_Destroy()=0;
	virtual DWORD Overlay_ColorMatch(LPDIRECTDRAWSURFACE pdds, COLORREF rgb)=0;
	virtual COLORREF Overlay_GetColor()=0;
	virtual COLORREF Overlay_GetCorrectedColor(HDC hDC)=0;
	virtual void Overlay_SetColorControls()=0;
	virtual BOOL Overlay_Lock_Back_Buffer(TDeinterlaceInfo* pInfo, BOOL bUseExtraBuffer)=0;
	virtual BOOL Overlay_Lock(TDeinterlaceInfo* pInfo)=0;
	virtual BOOL Overlay_Unlock_Back_Buffer(BOOL bUseExtraBuffer)=0;
	virtual BOOL Overlay_Unlock()=0;
	virtual BOOL Overlay_Flip(DWORD FlipFlag, BOOL bUseExtraBuffer, BYTE* lpExternalMemoryBuffer, int ExternalPitch, TDeinterlaceInfo* pInfo)=0;
	virtual HDC Overlay_GetDC()=0;
	virtual void Overlay_ReleaseDC(HDC hDC)=0;
    virtual void WaitForVerticalBlank() = 0;
	virtual void Overlay_SetRGB(BOOL IsRGB)=0;
	virtual BOOL Overlay_GetRGB()=0;

	virtual CTreeSettingsGeneric* Other_GetTreeSettingsPage()=0;

	virtual OUTPUTTYPES Type()=0;

    virtual void InitOtherSettings()=0;
    SETTING* GetOtherSettings();
};



#endif
