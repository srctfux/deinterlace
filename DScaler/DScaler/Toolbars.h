/////////////////////////////////////////////////////////////////////////////
// $Id: Toolbars.h,v 1.11 2003-10-27 10:39:54 adcockj Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2002 John Adcock.  All rights reserved.
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

/** 
 * @file Toolbars.h Toolbars Header file
 */
 
#ifndef _TOOLBARS_H_
#define _TOOLBARS_H_

#include "Events.h"
#include "ToolbarWindow.h"

//For eSoundChannel
#include "SoundChannel.h"


class CToolbarChannels : public CToolbarChild, public CEventObject 
{
public:
    CToolbarChannels(CToolbarWindow *pToolbar);
    ~CToolbarChannels();

	HWND CreateFromDialog(LPCTSTR lpTemplate, HINSTANCE hResourceInst);
    void Reset();
        
	void OnEvent(CEventObject *pEventObject, eEventType Event, long OldValue, long NewValue, eEventType *ComingUp);
	void UpdateWindow() { UpdateControls(NULL, FALSE); }

	LRESULT MyComboProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
private:
	int LastChannel;
	WNDPROC m_oldComboProc;
	HICON m_hIconChannelUp;
	HICON m_hIconChannelDown;
	HICON m_hIconChannelPrevious;

    void UpdateControls(HWND hWnd,bool bInitDialog);
    LRESULT ToolbarChildProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);    
	
	static LRESULT CALLBACK MyComboProcWrap(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
};



class CToolbarVolume : public CToolbarChild, public CEventObject
{
public:
    CToolbarVolume(CToolbarWindow *pToolbar);
	~CToolbarVolume();

    LRESULT ToolbarChildProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    
    void Reset();

	void OnEvent(CEventObject *pEventObject, eEventType Event, long OldValue, long NewValue, eEventType *ComingUp);
	void UpdateWindow() { UpdateControls(NULL, FALSE); }
private:
    BOOL m_Mute;
    int  m_Volume;
	int  m_VolumeMin;
	int  m_VolumeMax;
	BOOL m_UseMixer;
	eSoundChannel m_SoundChannel;

	HICON m_hIconMute;
	HICON m_hIconUnMute;

	HICON m_hIconMono;
	HICON m_hIconStereo;
	HICON m_hIconLang1;
	HICON m_hIconLang2;
        
    void UpdateControls(HWND hWnd, bool bInitDialog);	
};


class CToolbarMediaPlayer : public CToolbarChild, public CEventObject
{
public:
    CToolbarMediaPlayer(CToolbarWindow *pToolbar);
	~CToolbarMediaPlayer();

    LRESULT ToolbarChildProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    
    void Reset();

	void OnEvent(CEventObject *pEventObject, eEventType Event, long OldValue, long NewValue, eEventType *ComingUp);
	void UpdateWindow() { UpdateControls(NULL, FALSE); }
private:
	int m_Elapsed;	// In 1/10 of seconds
	int m_Duration;	// In 1/10 of seconds

	HICON m_hIconPlay;
	HICON m_hIconPause;
	HICON m_hIconStop;
        
    void UpdateControls(HWND hWnd, bool bInitDialog);	
};


class CToolbarLogo : public CToolbarChild, public CEventObject 
{
private:
	int OriginalLogoWidth;
	int OriginalLogoHeight;
public:
    CToolbarLogo(CToolbarWindow *pToolbar);	
	HWND CreateFromDialog(LPCTSTR lpTemplate, HINSTANCE hResourceInst);
    LRESULT ToolbarChildProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);        
    void Reset();    
};


class CToolbar1Bar : public CToolbarChild, public CEventObject 
{
private:
	int OriginalWidth;
	int OriginalHeight;
	int LeftMargin;
	int RightMargin;
	HWND hWndPicture;
	HBITMAP hBmp;	
public:
    CToolbar1Bar(CToolbarWindow *pToolbar);	
	~CToolbar1Bar();
	HWND Create(LPCSTR lpClassName, HINSTANCE hResourceInst);
    LRESULT ToolbarChildProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);	
	BOOL LoadSkin(const char *szSkinIniFile, const char *szSection, vector<int> *Results, CBitmapCache *pBitmapCache);
	void ClearSkin();
    void Reset();
    HWND GethWndPicture() { return hWndPicture; }
	void Margins(int l,int r);
};


//Toolbar management
void SetToolbars(HWND hWnd, LPCSTR szSkinName);
void UpdateToolbars(HWND hWnd, BOOL bRedraw);
void Toolbars_UpdateMenu(HMENU hMenu);
BOOL ProcessToolbar1Selection(HWND hWnd, UINT uItem);
void FreeToolbars();
void Toolbars_AdjustArea(LPRECT lpRect, int Crop);

#endif
