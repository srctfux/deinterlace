#ifndef _WINDOWBORDER_H_
#define _WINDOWBORDER_H_

#include "bitmap.h"
#include "BitmapAsButton.h"

enum eWindowBorderPosition
{
    WINDOWBORDER_TOPLEFTCORNER = 0,
    WINDOWBORDER_TOPLEFT,
    WINDOWBORDER_TOPMIDDLE,
    WINDOWBORDER_TOPRIGHT,
    WINDOWBORDER_TOPRIGHTCORNER,
    WINDOWBORDER_RIGHTTOP,
    WINDOWBORDER_RIGHTMIDDLE,
    WINDOWBORDER_RIGHTBOTTOM,
    WINDOWBORDER_BOTTOMRIGHTCORNER,
    WINDOWBORDER_BOTTOMRIGHT,
    WINDOWBORDER_BOTTOMMIDDLE,
    WINDOWBORDER_BOTTOMLEFT,
    WINDOWBORDER_BOTTOMLEFTCORNER,
    WINDOWBORDER_LEFTBOTTOM,
    WINDOWBORDER_LEFTMIDDLE,
    WINDOWBORDER_LEFTTOP,
    WINDOWBORDER_LASTONE
};

//
//
//    0   1    -2-    3   5
//    
//   15                   4
//
//
//   |                    |
//   14                   6
//   |                    |
//    
//
//   13                   7
//
//   12   11   -10-    9  8



class CWindowBorder
{
protected:
    HWND hWnd;
    HINSTANCE hResourceInst;

    int IsBorderVisible;

    vector<CBitmapHolder*> Bitmaps;
    int SolidTopSize;
    int SolidRightSize;
    int SolidBottomSize;
    int SolidLeftSize;

    
    typedef struct {
      string sID;
      CBitmapAsButton *Button;
      POINT Location;
      int   RelativePosition;
      POINT LastLocation;

      string sIniEntryDefault;
      string sIniEntryMouseOver;
      string sIniEntryClick;
    } TButtonInfo;

    vector<TButtonInfo> Buttons;
    
    int TopSize;
    int RightSize;
    int BottomSize;
    int LeftSize;

    RECT rcWindow;

    COLORREF DefaultColorRef;
    HBRUSH m_hBrush;
    BOOL bBitmapsChanged;

    vector<RECT> Locations;
    vector<LPRECT> RegionList;

    HRGN hLastRegion;

    void MergeLineRegion(int y,POINT *RowList,int RowListSize,int TotalWidth,vector<LPRECT> *AllRegions);
    void MergeBorderRegions(vector<LPRECT> *AllRegions, LPRECT lpRcExtra);    
    BOOL FindBorderSizes();
    BOOL FindLocations();

    void UpdateButtonLocations();

    BOOL (*CustomGetClientRect)(HWND hWnd, LPRECT lpRect);
    
    int BmpWidth(int Position);
    int BmpHeight(int Position);
    int BorderWidth(int Position);
    int BorderHeight(int Position);

    static const char* szBorderNames[];
public:
    CWindowBorder(HWND hWnd, HINSTANCE hInst, BOOL (*CustomGetClientRect)(HWND hWnd, LPRECT lpRect) = NULL);
    ~CWindowBorder();
    
    void AdjustArea(RECT *ar, int Crop);
    

    void DefaultColor(COLORREF Color);
    void SolidBorder(int left,int top, int right, int bottom, COLORREF Color);
    HRGN MakeRegion(LPRECT lpRcExtra);
    
    BOOL SetBorderBitmap(eWindowBorderPosition Position, int State, HBITMAP hBmp, HBITMAP hBmpMask, int DrawMode, BOOL bDeleteBitmapOnExit = FALSE);
    BOOL SetButtonBitmap(string sID, int WhichBitmap, eWindowBorderPosition RelPos, int x, int y, HBITMAP hBmp, HBITMAP hBmpMask, BOOL bDeleteBitmapOnExit = FALSE);
    
    BOOL RegisterButton(string sID, eBitmapAsButtonType ButtonType, string sIniEntryDefault, string sIniEntryMouseOver, string sIniEntryClick, BUTTONPROC *pfnButtonProc);
    
    void Paint(HWND hWnd, HDC hDC, LPRECT lpRect, POINT *pPShift = NULL);
    
    BOOL Visible() { return (IsBorderVisible>0); };
    BOOL Show() { IsBorderVisible=1; UpdateButtonLocations(); return TRUE; }
    BOOL Hide() { IsBorderVisible=0; UpdateButtonLocations(); return TRUE; }

    BOOL LoadSkin(const char *szSkinIniFile,  const char *szSection, vector<int> *Results);
    void ClearSkin();

};



#endif

