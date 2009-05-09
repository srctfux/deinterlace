//
// $Id$
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Jeroen Kooiman.  All rights reserved.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file WindowBorder.cpp CWindowBorder Implementation
 */

#include "stdafx.h"

#include "Bitmap.h"
#include "BitmapAsButton.h"
#include "WindowBorder.h"

#include "resource.h"
#include "DebugLog.h"

using namespace std;

/**

    Draws a border around an existing window using Windows GDI.
    All border pieces are extracted from a big bitmap.
    This big bitmap and the coordinaties for the border pieces
    are specified in an .ini file.

    With an additional mask bitmap (black&white), part of the
    border can be transparent (use SetWindowRgn).

    In addition to this, buttons can be added to the border.
    They are CBitmapAsButton classes.
*/

const TCHAR* CWindowBorder::szBorderNames[] = {
        _T("TopLeftCorner"),_T("TopLeft"),_T("TopMiddle"),_T("TopRight"),_T("TopRightCorner"),
        _T("RightTop"),_T("RightMiddle"),_T("RightBottom"),
        _T("BottomRightCorner"),_T("BottomRight"),_T("BottomMiddle"),_T("BottomLeft"),_T("BottomLeftCorner"),
        _T("LeftBottom"),_T("LeftMiddle"),_T("LeftTop"),
};


CWindowBorder::CWindowBorder(HWND hWnd, HINSTANCE hInst, BOOL (*CustomGetClientRect)(HWND hWnd, LPRECT lpRect) ):
    IsBorderVisible(0),
    bBitmapsChanged(FALSE),
    DefaultColorRef(0),
    TopSize(0),
    BottomSize(0),
    LeftSize(0),
    RightSize(0),
    hLastRegion(NULL),
    SolidTopSize(0),
    SolidRightSize(0),
    SolidBottomSize(0),
    SolidLeftSize(0),
    m_hBrush(NULL),
    m_IsSkinned(0)
{
    this->hWnd = hWnd;
    this->hResourceInst = hInst;
    this->CustomGetClientRect = CustomGetClientRect;
}

CWindowBorder::~CWindowBorder()
{
    if (m_hBrush != NULL)
    {
        ::DeleteObject(m_hBrush);
        m_hBrush = NULL;
    }
}

//Width of bitmap at border position 'Pos'
int CWindowBorder::BmpWidth(int Pos)
{
    if (!Bitmaps[Pos]) return 0;
    return Bitmaps[Pos]->Width();
}

int CWindowBorder::BmpHeight(int Pos)
{
    if (!Bitmaps[Pos]) return 0;
    return Bitmaps[Pos]->Height();
}

//Width the border of position 'Pos'
int CWindowBorder::BorderWidth(int Pos)
{
    switch (Pos)
    {
        case WINDOWBORDER_TOPLEFTCORNER:
        case WINDOWBORDER_BOTTOMLEFTCORNER:
        case WINDOWBORDER_LEFTBOTTOM:
        case WINDOWBORDER_LEFTMIDDLE:
        case WINDOWBORDER_LEFTTOP:
            return LeftSize;
        case WINDOWBORDER_TOPLEFT:
        case WINDOWBORDER_TOPMIDDLE:
        case WINDOWBORDER_TOPRIGHT:
            return BmpWidth(Pos);
        case WINDOWBORDER_TOPRIGHTCORNER:
        case WINDOWBORDER_RIGHTTOP:
        case WINDOWBORDER_RIGHTMIDDLE:
        case WINDOWBORDER_RIGHTBOTTOM:
        case WINDOWBORDER_BOTTOMRIGHTCORNER:
            return RightSize;
        case WINDOWBORDER_BOTTOMRIGHT:
        case WINDOWBORDER_BOTTOMMIDDLE:
        case WINDOWBORDER_BOTTOMLEFT:
            return BmpWidth(Pos);
    }
    return 0;
}

//Height of the border of position 'Pos'
int CWindowBorder::BorderHeight(int Pos)
{
    switch (Pos)
    {
        case WINDOWBORDER_TOPLEFTCORNER:
        case WINDOWBORDER_TOPLEFT:
        case WINDOWBORDER_TOPMIDDLE:
        case WINDOWBORDER_TOPRIGHT:
        case WINDOWBORDER_TOPRIGHTCORNER:
            return TopSize;
        case WINDOWBORDER_BOTTOMLEFTCORNER:
        case WINDOWBORDER_BOTTOMRIGHT:
        case WINDOWBORDER_BOTTOMMIDDLE:
        case WINDOWBORDER_BOTTOMLEFT:
        case WINDOWBORDER_BOTTOMRIGHTCORNER:
            return BottomSize;

        case WINDOWBORDER_LEFTBOTTOM:
        case WINDOWBORDER_LEFTMIDDLE:
        case WINDOWBORDER_LEFTTOP:
            return BmpHeight(Pos);
        case WINDOWBORDER_RIGHTTOP:
        case WINDOWBORDER_RIGHTMIDDLE:
        case WINDOWBORDER_RIGHTBOTTOM:
            return BmpHeight(Pos);
    }
    return 0;
}

//Subtract (Crop == 1) or add (Crop==0) the border from/to the area 'ar'
//
void CWindowBorder::AdjustArea(RECT *ar, int Crop)
{
    if (IsBorderVisible<=0)
    {
        return; //No border
    }

    FindBorderSizes();

    if (Crop)
    {
        ar->top    += TopSize;
        ar->bottom -= BottomSize;
        ar->left   += LeftSize;
        ar->right  -= RightSize;
    }
    else
    {
        ar->top    -= TopSize;
        ar->bottom += BottomSize;
        ar->left   -= LeftSize;
        ar->right  += RightSize;
    }
}




// Create window region
// Merges all regions of all border bitmaps if the size/construction has changed.
HRGN CWindowBorder::MakeRegion(LPRECT lpRcExtra)
{
    BOOL bMerge = TRUE;
    if (RegionList.size()>0)
    {
       if (!FindLocations())
       {
           bMerge = FALSE;
       }
    }
    else
    {
        FindLocations();
    }

   if (((rcWindow.right - rcWindow.left)==0) || ((rcWindow.bottom - rcWindow.top)==0))
   {
       return NULL;
   }

   if (bMerge)
   {
        RegionList.clear();
        MergeBorderRegions(RegionList, lpRcExtra);
   }

   RECT rc;
   rc.left = 0;
   rc.top  = 0;
   rc.right = rcWindow.right-rcWindow.left;
   rc.bottom= rcWindow.bottom-rcWindow.top;

   if (lpRcExtra != NULL)
   {
       rc.right += lpRcExtra->left + lpRcExtra->right;
       rc.bottom += lpRcExtra->top + lpRcExtra->bottom;
   }

   hLastRegion = CBitmapHolder::CreateWindowRegion(rc, RegionList);
   return hLastRegion;
}

//Calculate border size from all bitmaps widths&heights
BOOL CWindowBorder::FindBorderSizes()
{
    if (bBitmapsChanged)
    {
        bBitmapsChanged = FALSE;

        if (Bitmaps.size() == 0) {
          for (int Pos = 0; Pos < WINDOWBORDER_LASTONE; Pos++)
          {
              Bitmaps.push_back(NULL);
          }
        }

        #define ADJUSTWIDTH(Pos) \
          if ((Bitmaps[(Pos)].IsValid()) && (Bitmaps[(Pos)]->Width() > MaxWidth)) { MaxWidth = Bitmaps[(Pos)]->Width(); }
        #define ADJUSTHEIGHT(Pos) \
          if ((Bitmaps[(Pos)].IsValid()) && (Bitmaps[(Pos)]->Height() > MaxHeight)) { MaxHeight = Bitmaps[(Pos)]->Height(); }

        int MaxWidth;
        int MaxHeight;

        MaxWidth = SolidLeftSize;
        ADJUSTWIDTH(WINDOWBORDER_TOPLEFTCORNER);
        ADJUSTWIDTH(WINDOWBORDER_LEFTTOP);
        ADJUSTWIDTH(WINDOWBORDER_LEFTMIDDLE);
        ADJUSTWIDTH(WINDOWBORDER_LEFTBOTTOM);
        ADJUSTWIDTH(WINDOWBORDER_BOTTOMLEFTCORNER);
        LeftSize = MaxWidth;

        MaxWidth = SolidRightSize;
        ADJUSTWIDTH(WINDOWBORDER_TOPRIGHTCORNER);
        ADJUSTWIDTH(WINDOWBORDER_RIGHTTOP);
        ADJUSTWIDTH(WINDOWBORDER_RIGHTMIDDLE);
        ADJUSTWIDTH(WINDOWBORDER_RIGHTBOTTOM);
        ADJUSTWIDTH(WINDOWBORDER_BOTTOMRIGHTCORNER);
        RightSize = MaxWidth;

        MaxHeight = SolidTopSize;
        ADJUSTHEIGHT(WINDOWBORDER_TOPLEFTCORNER);
        ADJUSTHEIGHT(WINDOWBORDER_TOPLEFT);
        ADJUSTHEIGHT(WINDOWBORDER_TOPMIDDLE);
        ADJUSTHEIGHT(WINDOWBORDER_TOPRIGHT);
        ADJUSTHEIGHT(WINDOWBORDER_TOPRIGHTCORNER);
        TopSize = MaxHeight;

        MaxHeight = SolidBottomSize;
        ADJUSTHEIGHT(WINDOWBORDER_BOTTOMRIGHTCORNER);
        ADJUSTHEIGHT(WINDOWBORDER_BOTTOMRIGHT);
        ADJUSTHEIGHT(WINDOWBORDER_BOTTOMMIDDLE);
        ADJUSTHEIGHT(WINDOWBORDER_BOTTOMLEFT);
        ADJUSTHEIGHT(WINDOWBORDER_BOTTOMLEFTCORNER);
        BottomSize = MaxHeight;

        Locations.clear();

        return TRUE;
    }
    return FALSE;
}

//Calculate locations for all border bitmaps
// and buttons at the border
BOOL CWindowBorder::FindLocations()
{
    RECT m;
    if (CustomGetClientRect != NULL)
    {
        CustomGetClientRect(hWnd, &m);
    }
    else
    {
        GetClientRect(hWnd, &m);
    }

    FindBorderSizes();

    if ((Locations.size()>0) && ((m.right-m.left) == (rcWindow.right-rcWindow.left)) && ((m.bottom-m.top) == (rcWindow.bottom-rcWindow.top)) )
    {
        // not changed
        return FALSE;
    }

    rcWindow = m;
    Locations.clear();

    int Pos = WINDOWBORDER_TOPLEFTCORNER;
    RECT rc;

    //Top left
    ::SetRect(&rc,m.left,m.top,m.left+BorderWidth(Pos),m.top+BorderHeight(Pos)); Locations.push_back(rc); Pos++;

    ::SetRect(&rc,m.left+BorderWidth(Pos-1),m.top,m.left+BorderWidth(Pos-1)+BorderWidth(Pos),m.top+BorderHeight(Pos));  Locations.push_back(rc); Pos++;
    ::SetRect(&rc,m.left+BorderWidth(Pos-2)+BorderWidth(Pos-1),m.top,m.right-BorderWidth(Pos+1)-BorderWidth(Pos+2),m.top+BorderHeight(Pos));  Locations.push_back(rc); Pos++;
    ::SetRect(&rc,m.right-BorderWidth(Pos)-BorderWidth(Pos+1),m.top,m.right-BorderWidth(Pos+1),m.top+BorderHeight(Pos));  Locations.push_back(rc); Pos++;

    //Top right
    ::SetRect(&rc,m.right-BorderWidth(Pos),m.top,m.right,m.top+BorderHeight(Pos));  Locations.push_back(rc); Pos++;

    ::SetRect(&rc,m.right-BorderWidth(Pos),m.top+BorderHeight((Pos-1)),m.right,m.top+BorderHeight(Pos-1)+BorderHeight(Pos));  Locations.push_back(rc); Pos++;
    ::SetRect(&rc,m.right-BorderWidth(Pos),m.top+BorderHeight((Pos-1))+BorderHeight(Pos-2),m.right,m.bottom - BorderHeight(Pos+1)-BorderHeight(Pos+2));  Locations.push_back(rc); Pos++;
    ::SetRect(&rc,m.right-BorderWidth(Pos),m.bottom-BorderHeight(Pos+1)-BorderHeight(Pos),m.right,m.bottom-BorderHeight(Pos+1));  Locations.push_back(rc); Pos++;

    //Bottom right
    ::SetRect(&rc,m.right-BorderWidth(Pos),m.bottom-BorderHeight(Pos),m.right,m.bottom);  Locations.push_back(rc); Pos++;

    ::SetRect(&rc,m.right-BorderWidth(Pos-1)-BorderWidth(Pos),m.bottom-BorderHeight(Pos),m.right-BorderWidth(Pos-1),m.bottom);  Locations.push_back(rc); Pos++;
    ::SetRect(&rc,m.left+BorderWidth(Pos+1)+BorderWidth(Pos+2),m.bottom-BorderHeight(Pos),m.right-BorderWidth(Pos-1)-BorderWidth(Pos-2),m.bottom);  Locations.push_back(rc); Pos++;
    ::SetRect(&rc,m.left+BorderWidth(Pos+1),m.bottom-BorderHeight(Pos),m.left+BorderWidth(Pos+1)+BorderWidth(Pos),m.bottom);  Locations.push_back(rc); Pos++;

    //Bottom left
    ::SetRect(&rc,m.left,m.bottom-BorderHeight(Pos),m.left+BorderWidth(Pos),m.bottom);  Locations.push_back(rc); Pos++;

    ::SetRect(&rc,m.left,m.bottom-BorderHeight(Pos)-BorderHeight(Pos-1),m.left+BorderWidth(Pos),m.bottom-BorderHeight(Pos-1));  Locations.push_back(rc); Pos++;
    ::SetRect(&rc,m.left,m.top+BorderHeight(Pos+1)+BorderHeight(0),m.left+BorderWidth(Pos),m.bottom-BorderHeight(Pos-1)-BorderHeight(Pos-2));  Locations.push_back(rc); Pos++;
    ::SetRect(&rc,m.left,m.top+BorderHeight(0),m.left+BorderWidth(Pos),m.top+BorderHeight(0)+BorderHeight(Pos));  Locations.push_back(rc); Pos++;

    //for (Pos = 0; Pos < 16; Pos++) { LOG(2,_T("BRDR: Location [%i] = %d,%d - %d,%d"),Pos,Locations[Pos].left,Locations[Pos].top,Locations[Pos].right,Locations[Pos].bottom); }

    // Calculate button locations
    int i;
    for (i = 0; i < Buttons.size(); i++)
    {
        if ((Buttons[i].Button.IsValid()) && (Buttons[i].Button->hWnd() != NULL))
        {
            rc.left = Buttons[i].Location.x;
            rc.top  = Buttons[i].Location.y;

            LPRECT rcWin = &rcWindow;
            if ( (Buttons[i].RelativePosition >= 0) &&  (Buttons[i].RelativePosition < WINDOWBORDER_LASTONE) )
            {
                rcWin = &Locations[ Buttons[i].RelativePosition ];
            }

            if (rc.left == -1)  //align left
            {
                rc.left = rcWin->left;
            }
            else if (rc.left == -2) //align center
            {
                rc.left = rcWin->left + ((rcWin->right-rcWin->left)- Buttons[i].Button->Width())/2;
            }
            else if (rc.left == -3) //align right
            {
                rc.left = rcWin->right- Buttons[i].Button->Width();
            }
            else
            {
                rc.left = rcWin->left + rc.left;
            }

            if (rc.top == -1)  //align top
            {
                rc.top = rcWin->top;
            }
            else if (rc.top == -2) //align center
            {
                rc.top = rcWin->top + ((rcWin->bottom-rcWin->top) - Buttons[i].Button->Height())/2;
            }
            else if (rc.top == -3) //align bottom
            {
                rc.top = rcWin->bottom - Buttons[i].Button->Height();
            }
            else
            {
                rc.top = rcWin->top + rc.top;
            }

            rc.right = rc.left + Buttons[i].Button->Width();
            rc.bottom = rc.top + Buttons[i].Button->Height();
        }
        else
        {
            rc.left=rc.right=rc.top=rc.bottom=0;
        }
        Locations.push_back(rc); Pos++;
    }

    UpdateButtonLocations();

    return TRUE;
}

/**
    Merge connecting rects of the same line
*/
void CWindowBorder::MergeLineRegion(int y,POINT *pRowList,int RowListSize,int TotalWidth,vector<RECT>& AllRegions)
{
    RECT rc;
    rc.top=y;
    rc.bottom=y+1;
    if (RowListSize==0)
    {
        rc.left=0;
        rc.right=TotalWidth;
        AllRegions.push_back(rc);
        return;
    }

    if (RowListSize==1)
    {
        rc.left=pRowList[0].x;
        rc.right=pRowList[0].y;
        AllRegions.push_back(rc);
        return;
    }

    vector<int> SortedOrder(RowListSize);

    int i;
    int j;
    int n;

    for (i = 0; i < RowListSize; i++)
    {
        SortedOrder[i] = i;
    }
    for(i = 0; i < RowListSize ; i++)
    {
        for(j = i+1; j < RowListSize; j++)
        {
            if (pRowList[SortedOrder[i]].x > pRowList[SortedOrder[j]].x)
            {
                int tmp = SortedOrder[i];
                SortedOrder[i] = SortedOrder[j];
                SortedOrder[j] = tmp;
            }
        }
    }

    rc.left=pRowList[SortedOrder[0]].x;
    rc.right=pRowList[SortedOrder[0]].y;

    for (i = 1; i < RowListSize; i++)
    {
        n = SortedOrder[i];
        if (pRowList[n].x == rc.right)
        {
            rc.right = pRowList[n].y;
        }
        else
        {
           AllRegions.push_back(rc);
           rc.top = y;
           rc.bottom = y+1;
           rc.left = pRowList[n].x;
           rc.right= pRowList[n].y;
        }
    }
    AllRegions.push_back(rc);
}

/**
    Merge rects of border bitmaps & display area of the window
*/
void CWindowBorder::MergeBorderRegions(vector<RECT>& AllRegions, LPRECT lpRcExtra)
{
    if(Bitmaps.size() < WINDOWBORDER_LASTONE)
    {
        return;
    }

    int TotalHeight = rcWindow.bottom-rcWindow.top;
    int TotalWidth = rcWindow.right-rcWindow.left;

    int ExtraStartX = 0;
    int ExtraStartY = 0;
    int ExtraEndX   = 0;
    int ExtraEndY   = 0;

    vector<RECT>* RegionLists[WINDOWBORDER_LASTONE];

    //Extra space
    if (lpRcExtra != NULL)
    {
        ExtraStartX = lpRcExtra->left;
        ExtraStartY = lpRcExtra->top;
        ExtraEndX   = lpRcExtra->right;
        ExtraEndY   = lpRcExtra->bottom;
    }

    // Get/make regions of individual bitmaps
    int Pos;
    for (Pos = WINDOWBORDER_TOPLEFTCORNER; Pos < WINDOWBORDER_LASTONE; Pos++)
    {
      if (Bitmaps[Pos].IsValid())
      {
          RegionLists[Pos] = &(Bitmaps[Pos]->GetRegionList());
      }
      else
      {
          RegionLists[Pos] = NULL;
      }
    }

    //Extra space on top
    if (ExtraStartY > 0)
    {
        for (int line=0; line < ExtraStartY; line++)
        {
            RECT Rect;
            ::SetRect(&Rect, 0, line, TotalWidth+ExtraStartX+ExtraEndY, line+1);
            AllRegions.push_back(Rect);
        }
    }

    POINT *pRowList = NULL;
    POINT *p;
    int *pRowListSize = NULL;
    //LPRECT lpRc;

    //Process top lines
    if (TopSize > 0)
    {
         vector<POINT> pRowList(TopSize * (TotalWidth+2));
         vector<int> pRowListSize(TopSize);

         int h;
         for (h = 0; h < TopSize; h++)
         {
             p = &pRowList[h * (TotalWidth+2) + pRowListSize[h] ];
             pRowListSize[h] = 0;

             // Extra space to the left and right
             if (ExtraStartX>0)
             {
                 p->x = 0;
                 p->y = ExtraStartX;
                 pRowListSize[h]++;
                 p++;
             }
             if (ExtraEndX>0)
             {
                 p->x = ExtraStartX + TotalWidth;
                 p->y = ExtraStartX + TotalWidth + ExtraEndX;
                 p++;
                 pRowListSize[h]++;
             }
        }

        //Process all top bitmaps
        for (Pos = WINDOWBORDER_TOPLEFTCORNER; Pos <= WINDOWBORDER_TOPRIGHTCORNER; Pos++)
        {
            if ((RegionLists[Pos] != NULL) && (RegionLists[Pos]->size() > 0))
            {
                for (int i = 0; i < RegionLists[Pos]->size(); i++)
                {
                    int h = (*RegionLists[Pos])[i].top;
                    if ((h>=0) && (h<TopSize))
                    {
                        p = &pRowList[h * (TotalWidth+2) + pRowListSize[h] ];
                        p->x = ExtraStartX + Locations[Pos].left + (*RegionLists[Pos])[i].left;
                        if (Pos == WINDOWBORDER_TOPMIDDLE) //variable size
                        {
                            p->y = p->x + Locations[Pos].right - Locations[Pos].left;
                        }
                        else
                        {
                            p->y = p->x + ((*RegionLists[Pos])[i].right - (*RegionLists[Pos])[i].left);
                        }
                        pRowListSize[h]++;
                    }
                }
            }
        }
        //Merge and add to total list
        for (h = 0; h < TopSize; h++)
        {
            MergeLineRegion(ExtraStartY+h,&pRowList[h*(TotalWidth+2)],pRowListSize[h], ExtraStartX+TotalWidth+ExtraEndX, AllRegions);
        }
    }

    //Process middle lines
    int MiddleHeight = TotalHeight - TopSize - BottomSize;
    for (int line=0; line < MiddleHeight; line++)
    {
        RECT Rect;
        ::SetRect(&Rect, 0, ExtraStartY+TopSize+line, ExtraStartX+TotalWidth+ExtraEndX, ExtraStartY+TopSize+line+1);
        AllRegions.push_back(Rect);
    }

    //Process bottom lines
    if (BottomSize > 0)
    {
        pRowList = new POINT[BottomSize * (TotalWidth+2)];
        pRowListSize = new int[BottomSize];
        int h;
        for (h = 0; h < BottomSize; h++)
        {
            p = &pRowList[h * (TotalWidth+2) + pRowListSize[h] ];
            pRowListSize[h] = 0;
            if (ExtraStartX > 0)
            {
                p->x = 0;
                p->y = ExtraStartX;
                pRowListSize[h]++;
                p++;
            }
            if (ExtraEndX > 0)
            {
                p->x = ExtraStartX + TotalWidth;
                p->y = ExtraStartX + TotalWidth + ExtraEndX;
                p++;
                pRowListSize[h]++;
            }
        }

        //Process all bitmaps at the bottom
        for (Pos = WINDOWBORDER_BOTTOMLEFTCORNER; Pos >= WINDOWBORDER_BOTTOMRIGHTCORNER; Pos--)
        {
         if ((RegionLists[Pos] != NULL) && (RegionLists[Pos]->size() > 0))
         {
             for (int i = 0; i < RegionLists[Pos]->size(); i++)
             {
                 int h = (*RegionLists[Pos])[i].top;
                 if ((h>=0) && (h<BottomSize))
                 {
                    p = &pRowList[h * (TotalWidth+2) + pRowListSize[h] ];
                    p->x = ExtraStartX + Locations[Pos].left + (*RegionLists[Pos])[i].left;
                    if (Pos == WINDOWBORDER_BOTTOMMIDDLE) //variable size
                    {
                        p->y = p->x + Locations[Pos].right - Locations[Pos].left;
                    }
                    else
                    {
                        p->y = p->x + ((*RegionLists[Pos])[i].right - (*RegionLists[Pos])[i].left);
                    }
                    pRowListSize[h]++;
                 }
             }
         }
        }

        for (h = 0; h < BottomSize; h++)
        {
        MergeLineRegion(h+ExtraStartY+TotalHeight-BottomSize,&pRowList[h*(TotalWidth+2)],pRowListSize[h], ExtraStartX+TotalWidth+ExtraEndX, AllRegions);
        }
    }

    //Extra space below
    if (ExtraEndY > 0)
    {
        for (int line=0; line < ExtraEndY; line++)
        {
            RECT Rect;
            ::SetRect(&Rect, 0, ExtraStartY+TotalHeight+line, TotalWidth+ExtraStartX+ExtraEndY, ExtraStartY+TotalHeight+line+1);
            AllRegions.push_back(Rect);
        }
    }
    //LOG(2,_T("Skin: %08x: Merged region list: %d pieces"),this->hWnd,AllRegions->size());
}


//Set bitmap for border piece 'Position'
BOOL CWindowBorder::SetBorderBitmap(eWindowBorderPosition Position, int State, SmartPtr<CBitmapState> BitmapState, int DrawMode)
{
    if ((Position<0) || (Position >= WINDOWBORDER_LASTONE))
    {
        return FALSE;
    }

    Bitmaps.resize(WINDOWBORDER_LASTONE);

    switch(Position)
    {
        case WINDOWBORDER_TOPMIDDLE:
        case WINDOWBORDER_RIGHTMIDDLE:
        case WINDOWBORDER_BOTTOMMIDDLE:
        case WINDOWBORDER_LEFTMIDDLE:
            //DrawMode ok (single, stretched, tiled)
            break;
        default:
            DrawMode = 0; //Just one piece
    }

    bBitmapsChanged = TRUE;

    if (!Bitmaps[Position])
    {
        Bitmaps[Position] = new CBitmapHolder(DrawMode);
    }

    Bitmaps[Position]->Add(BitmapState,State);
    return TRUE;
}


//Draw border bitmaps
void CWindowBorder::Paint(HWND hWnd, HDC hDC, LPRECT lpRect, POINT *pPShift)
{
    if (IsBorderVisible<=0) return;

    //LOG(2,_T("Skin: 0x%08x: Paint border 0x%08x"),this->hWnd,hWnd);

    FindBorderSizes();
    FindLocations();
    if (lpRect == NULL)
    {
        lpRect = &rcWindow;
    }

    //LOG(2,_T("Skin: 0x%08x: Update area (%d,%d,%d,%d) at (%d,%d)"),this->hWnd,lpRect->left,lpRect->top,lpRect->right,lpRect->bottom,(pPShift==NULL)?0:pPShift->x,(pPShift==NULL)?0:pPShift->y);

    if (Bitmaps.size() == 0) {
        for (int Pos = 0; Pos < WINDOWBORDER_LASTONE; Pos++)
        {
            Bitmaps.push_back(NULL);
        }
    }

    RECT rcPaint;

    // Find intersection with bitmaps & draw
    for (int Pos = 0; Pos < WINDOWBORDER_LASTONE; Pos++)
    {
       if ( ((Locations[Pos].right-Locations[Pos].left) > 0) &&
            ((Locations[Pos].bottom-Locations[Pos].top) > 0) )
       {
          IntersectRect(&rcPaint, &Locations[Pos], lpRect);
          if ( ((rcPaint.right-rcPaint.left)>0) &&
              ((rcPaint.bottom-rcPaint.top)>0) )
          {
             if (Bitmaps[Pos].IsValid())
             {
                POINT PBmp;
                PBmp.x = rcPaint.left - Locations[Pos].left;
                PBmp.y = rcPaint.top - Locations[Pos].top;

                if (pPShift!=NULL)
                {
                    rcPaint.left+=pPShift->x;
                    rcPaint.top+=pPShift->y;
                    rcPaint.right+=pPShift->x;
                    rcPaint.bottom+=pPShift->y;
                }

                Bitmaps[Pos]->Draw(hDC, &PBmp, &rcPaint, 0);
             }
             else
             {
                if (pPShift!=NULL)
                {
                    rcPaint.left+=pPShift->x;
                    rcPaint.top+=pPShift->y;
                    rcPaint.right+=pPShift->x;
                    rcPaint.bottom+=pPShift->y;
                }

                if (m_hBrush == NULL)
                {
                    m_hBrush = CreateSolidBrush(DefaultColorRef);
                }
                FillRect(hDC,&rcPaint,m_hBrush);
             }
          }
       }
    }
}

//Update button locations
void CWindowBorder::UpdateButtonLocations()
{
    if (Locations.size() != (WINDOWBORDER_LASTONE + Buttons.size()))
    {
        FindLocations();
        return;
    }
    for (int ButtonPos = 0; ButtonPos < Buttons.size(); ButtonPos++)
    {
        if ((Buttons[ButtonPos].Button.IsValid()) && (Buttons[ButtonPos].Button->hWnd()!=NULL))
        {
            if (IsBorderVisible)
            {
              if (  (Locations[WINDOWBORDER_LASTONE+ButtonPos].left != Buttons[ButtonPos].LastLocation.x)
                 || (Locations[WINDOWBORDER_LASTONE+ButtonPos].top  != Buttons[ButtonPos].LastLocation.y)
                 )
              {
                  SetWindowPos(Buttons[ButtonPos].Button->hWnd(),NULL,Locations[WINDOWBORDER_LASTONE+ButtonPos].left,Locations[WINDOWBORDER_LASTONE+ButtonPos].top,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
                  Buttons[ButtonPos].LastLocation.x = Locations[WINDOWBORDER_LASTONE+ButtonPos].left;
                  Buttons[ButtonPos].LastLocation.y = Locations[WINDOWBORDER_LASTONE+ButtonPos].top;
               }
               ShowWindow(Buttons[ButtonPos].Button->hWnd(),SW_SHOW);
            }
            else
            {
                ShowWindow(Buttons[ButtonPos].Button->hWnd(),SW_HIDE);
            }
        }
    }
}

void CWindowBorder::DefaultColor(COLORREF Color)
{
    if (m_hBrush != NULL)
    {
        ::DeleteObject(m_hBrush);
        m_hBrush = NULL;
    }
    DefaultColorRef = Color;
}


void CWindowBorder::SolidBorder(int left,int top, int right, int bottom, COLORREF Color)
{
    SolidTopSize = top;
    SolidRightSize = right;
    SolidBottomSize = bottom;
    SolidLeftSize = left;
    DefaultColor(Color);

    bBitmapsChanged = TRUE;
}


//Load bitmaps & buttons
BOOL CWindowBorder::LoadSkin(const TCHAR* szSkinIniFile, const TCHAR* szSection, vector<int> *Results)
{
    int Pos;
    int ButtonPos;

    CBitmapsFromIniSection BitmapsFromIniSection;

    if (Results != NULL)
    {
        (*Results).clear();
        for (Pos = 0; Pos < WINDOWBORDER_LASTONE+(Buttons.size()*3); Pos++)
        {
            (*Results).push_back(-1);
        }
    }

    // Read bitmaps
    for (Pos = 0; Pos < WINDOWBORDER_LASTONE; Pos++)
    {
        BitmapsFromIniSection.Register(szBorderNames[Pos]);
    }

    for (ButtonPos = 0; ButtonPos < Buttons.size(); ButtonPos++)
    {
        BitmapsFromIniSection.Register(Buttons[ButtonPos].sIniEntryDefault);
        BitmapsFromIniSection.Register(Buttons[ButtonPos].sIniEntryMouseOver);
        BitmapsFromIniSection.Register(Buttons[ButtonPos].sIniEntryClick);
    }

    if (BitmapsFromIniSection.Read(szSkinIniFile, szSection, _T("Bitmap"), _T("Mask")) < 0)
    {
        return FALSE;
    }

    m_IsSkinned = TRUE;

    // Load them
    for (Pos = 0; Pos < WINDOWBORDER_LASTONE; Pos++)
    {
        if (BitmapsFromIniSection.Get(szBorderNames[Pos]).IsValid())
        {
            SetBorderBitmap((eWindowBorderPosition)Pos,0,
                        BitmapsFromIniSection.Get(szBorderNames[Pos]), 2);
            LOG(2,_T("Skin: [%s] Load bitmap: pos %d ok (%s)."),szSection,Pos,szBorderNames[Pos]);
        }
    }

    for (ButtonPos = 0; ButtonPos < Buttons.size(); ButtonPos++)
    {
        tstring sExtra;
        POINT  PntLocation;
        int RelativePos;
        int ButtonSubPos;

        ButtonSubPos = 0;

        RelativePos = WINDOWBORDER_LASTONE;
        PntLocation.x = 0;
        PntLocation.y = 0;

        sExtra = BitmapsFromIniSection.Get(Buttons[ButtonPos].sIniEntryDefault)->m_ExtraInfo;

        if (sExtra.length() > 0)
        {
            tstring sLocationName;
            tstring sP1;
            tstring sP2;
            TCHAR* szExtra = (TCHAR*)sExtra.c_str();
            TCHAR* s = _tcschr(szExtra,',');
            TCHAR* s2;
            if (s!=NULL)
            {
                sLocationName = sExtra.substr(0, (s-szExtra));
                if ((s2 = _tcschr(s+1,','))!=NULL)
                {
                    sP1 = sExtra.substr((s+1-szExtra),s2-s-1);
                    s = s2;
                    if ((s2 = _tcschr(s+1,','))!=NULL)
                    {
                        //More
                    }
                    else
                    {
                        sP2 = sExtra.substr((s+1-szExtra),s2-s-1);
                    }
                }
            }
            if (!_tcsicmp(sP1.c_str(),_T("Left")))
            {
                PntLocation.x = -1;
            }
            else if ((!_tcsicmp(sP1.c_str(),_T("Center"))) || (!_tcsicmp(sP1.c_str(),_T("Middle"))))
            {
                PntLocation.x = -2;
            }
            else if (!_tcsicmp(sP1.c_str(),_T("Right")))
            {
                PntLocation.x = -3;
            }
            else
            {
                PntLocation.x = _ttoi(sP1.c_str());
                if (PntLocation.x<0) { PntLocation.x=0; }
            }

            if (!_tcsicmp(sP2.c_str(),_T("Top")))
            {
                PntLocation.y = -1;
            }
            else if ((!_tcsicmp(sP2.c_str(),_T("Center"))) || (!_tcsicmp(sP2.c_str(),_T("Middle"))))
            {
                PntLocation.y = -2;
            }
            else if (!_tcsicmp(sP2.c_str(),_T("Bottom")))
            {
                PntLocation.y = -3;
            }
            else
            {
                PntLocation.y = _ttoi(sP2.c_str());
                if (PntLocation.y<0) { PntLocation.y=0; }
            }

            int i = 0;
            for (i=0; i<WINDOWBORDER_LASTONE; i++)
            {
                if (_tcsicmp(szBorderNames[i],sLocationName.c_str())==0) break;
            }
            RelativePos = i;
        }

        if (SetButtonBitmap(Buttons[ButtonPos].sID, ButtonSubPos, (eWindowBorderPosition)RelativePos,
                PntLocation.x, PntLocation.y,
                BitmapsFromIniSection.Get(Buttons[ButtonPos].sIniEntryDefault)))
        {
        }
        ButtonSubPos++;

        if (SetButtonBitmap(Buttons[ButtonPos].sID, ButtonSubPos, (eWindowBorderPosition)RelativePos,
                PntLocation.x, PntLocation.y,
                BitmapsFromIniSection.Get(Buttons[ButtonPos].sIniEntryMouseOver)) )
        {
        }
        ButtonSubPos++;

        if (SetButtonBitmap(Buttons[ButtonPos].sID, ButtonSubPos, (eWindowBorderPosition)RelativePos,
                PntLocation.x, PntLocation.y,
                BitmapsFromIniSection.Get(Buttons[ButtonPos].sIniEntryClick)) )
        {
        }
    }

    return TRUE;
}

void CWindowBorder::ClearSkin()
{
    Bitmaps.clear();
    bBitmapsChanged = TRUE;

    Buttons.clear();
    RegionList.clear();

    m_IsSkinned = FALSE;

    FindBorderSizes();
    FindLocations();
}

//Register button to list
BOOL CWindowBorder::RegisterButton(tstring sID, eBitmapAsButtonType ButtonType, tstring sIniEntryDefault, tstring sIniEntryMouseOver, tstring sIniEntryClick, BUTTONPROC *pfnButtonProc)
{
    int Position = 0;
    while (Position < Buttons.size())
    {
        if (Buttons[Position].sID == sID) break;
        Position++;
    }

    if (Position >= Buttons.size())
    {
       TButtonInfo bi;
       bi.sID = sID;
       bi.Button = new CBitmapAsButton(ButtonType);
       bi.Button->Create(sID,hWnd,0,0,hResourceInst);

       bi.Location.x = 0;
       bi.Location.y = 0;
       bi.RelativePosition = -1;
       bi.LastLocation.x = -1;
       bi.LastLocation.y = -1;

       if (bi.Button->hWnd() == NULL)
       {
            return FALSE;
       }
       Buttons.push_back(bi);
    }

    Buttons[Position].sIniEntryDefault = sIniEntryDefault;
    Buttons[Position].sIniEntryMouseOver = sIniEntryMouseOver;
    Buttons[Position].sIniEntryClick = sIniEntryClick;

    Buttons[Position].Button->SetProcessMessage(this, pfnButtonProc);

    return TRUE;
}


BOOL CWindowBorder::SetButtonBitmap(tstring sID, int WhichBitmap, eWindowBorderPosition RelPos, int x, int y, SmartPtr<CBitmapState> BitmapState)
{
    int Position = 0;
    while (Position < Buttons.size())
    {
        if (Buttons[Position].sID == sID) break;
        Position++;
    }

    if (Position >= Buttons.size())
    {
        return FALSE; //unknown
    }

    if ( (!Buttons[Position].Button) || (Buttons[Position].Button->hWnd() == NULL))
    {
        return FALSE;
    }

    Buttons[Position].Location.x = x;
    Buttons[Position].Location.y = y;
    Buttons[Position].RelativePosition = RelPos;

    Buttons[Position].Button->AddBitmap(WhichBitmap, BitmapState);

    return TRUE;
}

