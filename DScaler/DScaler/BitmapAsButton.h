#ifndef _BITMAPASBUTTON_H_
#define _BITMAPASBUTTON_H_

#include "Bitmap.h"

typedef LRESULT (__cdecl BUTTONPROC)(string sID, void *pThis, HWND hWndParent, UINT MouseFlags, HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

enum eBitmapAsButtonType
{
    BITMAPASBUTTON_PUSH = 0,
    BITMAPASBUTTON_CHECKBOX,
    BITMAPASBUTTON_SLIDER,
};

/** Simple button with different bitmaps for mouse over and click
    Calls user defined function for window message processing

    Button types:

     push button
        state: 0 = normal
               1 = mouse over
               2 = mouse down
     check box
        state: 0 = normal,   	unchecked
               1 = mouse over,  unchecked
               2 = mouse down,  unchecked
               3 = normal,  	checked
               4 = mouse over,  checked
               5 = mouse down,  checked
        
    slider bar:
        state: 0 = unselected bar
        state: 1 = unselected bar, mouse over
        state: 2 = unselected bar, mouse down
        state: 3 = selected bar
        state: 4 = selected bar, mouse over
        state: 5 = selected bar, mouse down        
        state: 6 = slider
        state: 7 = slider, mouse over
        state: 8 = slider, mouse down        
        
*/

class CBitmapAsButton {
protected:
    string sID;

    eBitmapAsButtonType  ButtonType;
    int  ButtonState;

    HWND hWndParent;
    HWND hWndButton;
    BOOL bFailed;

    CBitmapHolder m_bhBmp;
    
    int ButtonWidth;
    int ButtonHeight;
    
    BUTTONPROC* pfnButtonProc;
    void *pfnButtonProc_pThis;
    void *pOriginalProc;
    int OriginalWidth;
    int OriginalHeight;
    
    HCURSOR hCursorHand;
    //
    BOOL m_mouseldown;
    BOOL m_mouserdown;
    BOOL m_mouseover;
    BOOL m_trackmouse;
    // Checkbox specific
    BOOL Checked;
    // Slider specific
    int SliderPos;
    int SliderRangeMin;
    int SliderRangeMax;
protected:

    void Draw(HDC hDC, LPRECT lpRect);
    void SetWindowRegion();
    
    LRESULT ButtonProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

    static LRESULT CALLBACK StaticButtonProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);    
public:
    CBitmapAsButton(eBitmapAsButtonType ButtonType);
    ~CBitmapAsButton();

    HWND hWnd() { if (bFailed) return NULL; return hWndButton; }
    int Width() { return ButtonWidth; }
    int Height() { return ButtonHeight; }
    string GetID() { return sID; }

    void AddBitmap(int State, HBITMAP hBmp, HBITMAP hBmpMask, BOOL bDeleteBitmapOnExit = FALSE);

    HRGN SetRegion(int State);
    
    void SetProcessMessage(void *pThis, BUTTONPROC* pfnButtonProc) 
    { 
        this->pfnButtonProc_pThis = pThis;
        this->pfnButtonProc = pfnButtonProc; 
    }

    BOOL Create(string sID, HWND hWndParent, int x, int y, HINSTANCE hInst);
    BOOL TakeOver(HWND hWnd, string sID, HWND hWndParent);
    BOOL RestoreBack();
};


#endif
