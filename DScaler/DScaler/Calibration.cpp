/////////////////////////////////////////////////////////////////////////////
// $Id: Calibration.cpp,v 1.40 2002-02-09 02:44:56 laurentg Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 Laurent Garnier.  All rights reserved.
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
// Revision 1.39  2002/02/08 00:36:06  laurentg
// Support of a new type of file : DScaler patterns
//
// Revision 1.38  2001/12/08 13:43:20  adcockj
// Fixed logging and memory leak bugs
//
// Revision 1.37  2001/12/05 21:45:10  ittarnavsky
// added changes for the AudioDecoder and AudioControls support
//
// Revision 1.36  2001/11/29 17:30:51  adcockj
// Reorgainised bt848 initilization
// More Javadoc-ing
//
// Revision 1.35  2001/11/24 22:57:02  laurentg
// Copyright line restored
//
// Revision 1.34  2001/11/23 10:49:16  adcockj
// Move resource includes back to top of files to avoid need to rebuild all
//
// Revision 1.33  2001/11/22 13:32:03  adcockj
// Finished changes caused by changes to TDeinterlaceInfo - Compiles
//
// Revision 1.32  2001/11/21 12:32:11  adcockj
// Renamed CInterlacedSource to CSource in preparation for changes to DEINTERLACE_INFO
//
// Revision 1.31  2001/11/09 12:42:07  adcockj
// Separated most resources out into separate dll ready for localization
//
// Revision 1.30  2001/11/02 16:30:07  adcockj
// Check in merged code from multiple cards branch into main tree
//
// Revision 1.29  2001/11/01 11:35:23  adcockj
// Pre release changes to version, help, comment and headers
//
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 23 Jul 2001   Laurent Garnier       File created
//
/////////////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "Calibration.h"
#include "DScaler.h"
#include "Setting.h"
#include "OSD.h"
#include "Providers.h"
#include "DebugLog.h"
#include "AspectRatio.h"
#include "OutThreads.h"
#include "Other.h"


// Minimum time in milliseconds between two consecutive evaluations
#define	MIN_TIME_BETWEEN_CALC	35
// Number of calculations to do on successive frames before to decide what to adjust
#define NB_CALCULATIONS_LOW     10
#define NB_CALCULATIONS_HIGH    50
// Delta used to stop the search process when the current value for setting implies
// a result that is superior to this delta when compared to the previous found best result
#define DELTA_STOP              5
// Maximum value
#define MAX_VALUE               1000000000

// Macro to restrict range to [0,255]
#define LIMIT(x) (((x)<0)?0:((x)>255)?255:(x))

// Macro to return the absolute value
#define ABSOLUTE_VALUE(x) ((x) < 0) ? -(x) : (x)


static long SourceOverscan = 0;
static long LeftCropping = 8;
static long RightCropping = 16;
static BOOL ShowRGBDelta = TRUE;
static BOOL ShowYUVDelta = TRUE;


/////////////////////////////////////////////////////////////////////////////
// Class CColorBar

CColorBar::CColorBar(unsigned short int left, unsigned short int right, unsigned short int top, unsigned short int bottom, BOOL YUV, unsigned char R_Y, unsigned char G_U, unsigned char B_V)
{ 
    m_LeftBorder = left; 
    m_RightBorder = right; 
    m_TopBorder = top; 
    m_BottomBorder = bottom;
    if (YUV)
    {
        ref_Y_val = R_Y;
        ref_U_val = G_U;
        ref_V_val = B_V;
        // Calculate RGB values from YUV values
        YUV2RGB(ref_Y_val, ref_U_val, ref_V_val, &ref_R_val, &ref_G_val, &ref_B_val);
    }
    else
    {
        ref_R_val = R_Y;
        ref_G_val = G_U;
        ref_B_val = B_V;
        // Calculate YUV values from RGB values
        RGB2YUV(ref_R_val, ref_G_val, ref_B_val, &ref_Y_val, &ref_U_val, &ref_V_val);
    }

    cpt_Y = cpt_U = cpt_V = cpt_nb = 0;
}

CColorBar::CColorBar(CColorBar* pColorBar)
{
    unsigned char val1, val2, val3;
    unsigned short int left, right, top, bottom;

    pColorBar->GetPosition(&left, &right, &top, &bottom);
    m_LeftBorder = left; 
    m_RightBorder = right; 
    m_TopBorder = top; 
    m_BottomBorder = bottom;

    pColorBar->GetRefColor(FALSE, &val1, &val2, &val3);
    ref_R_val = val1;
    ref_G_val = val2;
    ref_B_val = val3;

    pColorBar->GetRefColor(TRUE, &val1, &val2, &val3);
    ref_Y_val = val1;
    ref_U_val = val2;
    ref_V_val = val3;

    cpt_Y = cpt_U = cpt_V = cpt_nb = 0;
}

// This methode returns the position of the color bar
void CColorBar::GetPosition(unsigned short int* left, unsigned short int* right, unsigned short int* top, unsigned short int* bottom)
{
    *left = m_LeftBorder;
    *right = m_RightBorder;
    *top = m_TopBorder;
    *bottom = m_BottomBorder;
}

// This methode returns the reference color
// If parameter YUV is TRUE, it returns YUV values else RGB values
void CColorBar::GetRefColor(BOOL YUV, unsigned char* pR_Y, unsigned char* pG_U, unsigned char* pB_V)
{ 
    if (YUV)
    {
        *pR_Y = ref_Y_val;
        *pG_U = ref_U_val;
        *pB_V = ref_V_val;
    }
    else
    {
        *pR_Y = ref_R_val;
        *pG_U = ref_G_val;
        *pB_V = ref_B_val;
    }
}

// This methode returns the calculated average color
// If parameter YUV is TRUE, it returns YUV values else RGB values
void CColorBar::GetCurrentAvgColor(BOOL YUV, unsigned char* pR_Y, unsigned char* pG_U, unsigned char* pB_V)
{ 
    if (YUV)
    {
        *pR_Y = Y_val;
        *pG_U = U_val;
        *pB_V = V_val;
    }
    else
    {
        *pR_Y = R_val;
        *pG_U = G_val;
        *pB_V = B_val;
    }
}

// This methode returns the delta between reference color and calculated average color
// It includes a delta for each color component + the sum of these three absolute deltas.
// If parameter YUV is TRUE, it returns YUV values else RGB values
void CColorBar::GetDeltaColor(BOOL YUV, int* pR_Y, int* pG_U, int* pB_V, int* pTotal)
{
    if (YUV)
    {
        *pR_Y = Y_val - ref_Y_val;
        *pG_U = U_val - ref_U_val;
        *pB_V = V_val - ref_V_val;
    }
    else
    {
        *pR_Y = R_val - ref_R_val;
        *pG_U = G_val - ref_G_val;
        *pB_V = B_val - ref_B_val;
    }

    *pTotal = 0;
    *pTotal += ABSOLUTE_VALUE(*pR_Y);
    *pTotal += ABSOLUTE_VALUE(*pG_U);
    *pTotal += ABSOLUTE_VALUE(*pB_V);
}

// This method analyzed the overlay buffer to calculate average color
// in the zone defined by the color bar
BOOL CColorBar::CalcAvgColor(BOOL reinit, unsigned int nb_calc_needed, TDeinterlaceInfo* pInfo)
{ 
    int left, right, top, bottom, i, j;
    unsigned int Y, U, V, nb_Y, nb_U, nb_V;
    BYTE* buf;
    int overscan;
    int left_crop, total_crop;
    int width = pInfo->FrameWidth;
    int height = pInfo->FieldHeight;

    if (reinit)
    {
        cpt_Y = 0;
        cpt_U = 0;
        cpt_V = 0;
        cpt_nb = 0;
    }

    // Calculate the exact coordinates of rectangular zone in the buffer
    overscan = SourceOverscan*  width / (height*  2);
    left_crop = ((LeftCropping*  width) + 500) / 1000;
    total_crop = (((LeftCropping + RightCropping)*  width) + 500) / 1000;
    left = (width + total_crop - 2*  overscan)*  m_LeftBorder / 10000 - left_crop + overscan;
    if (left < 0)
    {
        left = 0;
    }
    else if (left >= width)
    {
        left = width - 1;
    }
    right = (width + total_crop - 2*  overscan)*  m_RightBorder / 10000 - left_crop + overscan;
    if (right < 0)
    {
        right = 0;
    }
    else if (right >= width)
    {
        right = width - 1;
    }
    overscan = SourceOverscan;
    top = (height - overscan)*  m_TopBorder / 10000 + overscan / 2;
    bottom = (height - overscan)*  m_BottomBorder / 10000 + overscan / 2;

    // Sum separately Y, U and V in this rectangular zone
    // Each line is like this : YUYVYUYV...
    Y = 0; nb_Y = 0;
    U = 0; nb_U = 0;
    V = 0; nb_V = 0;
    for (i = top ; i <= bottom ; i++)
    {
        for (j = left ; j <= right ; j++)
        {
            buf = pInfo->PictureHistory[0]->pData + (i * pInfo->InputPitch);
            Y += buf[j*2];
            nb_Y++;
            if (j % 2)
            {
                V += buf[j*2+1];
                nb_V++;
            }
            else
            {
                U += buf[j*2+1];
                nb_U++;
            }
        }
    }

    // Calculate the average for Y, U and V
    cpt_Y += Y;
    cpt_U += U;
    cpt_V += V;
    cpt_nb++;

    if (cpt_nb >= nb_calc_needed)
    {
        if (nb_Y > 0)
        {
            Y_val = (cpt_Y + (cpt_nb*  nb_Y / 2)) / (cpt_nb*  nb_Y);
        }
        else
        {
            Y_val = 0;
        }
        if (nb_Y > 0)
        {
            U_val = (cpt_U + (cpt_nb*  nb_U / 2)) / (cpt_nb*  nb_U);
        }
        else
        {
            U_val = 0;
        }
        if (nb_Y > 0)
        {
            V_val = (cpt_V + (cpt_nb*  nb_V / 2)) / (cpt_nb*  nb_V);
        }
        else
        {
            V_val = 0;
        }
        V_val = (cpt_V + (cpt_nb*  nb_V / 2)) / (cpt_nb*  nb_V);

        // Save corresponding RGB values too
        YUV2RGB(Y_val, U_val, V_val, &R_val, &G_val, &B_val);

        LOG(5, "CalcAvgColor YUV %d %d %d %d %d %d %d %d %d", Y_val, U_val, V_val, left, right, top, bottom, height, width);
        LOG(5, "CalcAvgColor RGB %d %d %d %d %d %d %d %d %d", R_val, G_val, B_val, left, right, top, bottom, height, width);

        cpt_Y = 0;
        cpt_U = 0;
        cpt_V = 0;
        cpt_nb = 0;

        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

// This method draws in the video signal a rectangle around the color bar
void CColorBar::DrawPosition(TDeinterlaceInfo* pInfo)
{
    int left, right, top, bottom, i;
    BYTE* buf;
    int overscan;
    int left_crop, total_crop;
    int width = pInfo->FrameWidth;
    int height = pInfo->FieldHeight;

    // Calculate the exact coordinates of rectangular zone in the buffer
    overscan = SourceOverscan*  width / (height*  2);
    left_crop = ((LeftCropping*  width) + 500) / 1000;
    total_crop = (((LeftCropping + RightCropping)*  width) + 500) / 1000;
    left = (width + total_crop - 2*  overscan)*  m_LeftBorder / 10000 - left_crop + overscan;
    if (left < 0)
    {
        left = 0;
    }
    else if (left >= width)
    {
        left = width - 1;
    }
    right = (width + total_crop - 2*  overscan)*  m_RightBorder / 10000 - left_crop + overscan;
    if (right < 0)
    {
        right = 0;
    }
    else if (right >= width)
    {
        right = width - 1;
    }
    overscan = SourceOverscan;
    top = (height - overscan)*  m_TopBorder / 10000 + overscan / 2;
    bottom = (height - overscan)*  m_BottomBorder / 10000 + overscan / 2;

    if ((left % 2) == 1)
    {
        left--;
    }
    if ((right % 2) == 1)
    {
        right--;
    }

    for (i = top ; i <= bottom ; i++)
    {
        buf = pInfo->PictureHistory[0]->pData + (i * pInfo->InputPitch);
        buf[left*2  ] = (ref_Y_val < 128) ? 235 : 16;
        buf[left*2+1] = 128;
        buf[left*2+2] = (ref_Y_val < 128) ? 235 : 16;
        buf[left*2+3] = 128;
        buf[right*2  ] = (ref_Y_val < 128) ? 235 : 16;
        buf[right*2+1] = 128;
        buf[right*2+2] = (ref_Y_val < 128) ? 235 : 16;
        buf[right*2+3] = 128;
    }

    right++;
    for (i = left ; i <= right ; i++)
    {
        buf = pInfo->PictureHistory[0]->pData + (top * pInfo->InputPitch);
        buf[i*2] = (ref_Y_val < 128) ? 235 : 16;
        buf[i*2+1] = 128;
        buf = pInfo->PictureHistory[0]->pData + (bottom * pInfo->InputPitch);
        buf[i*2] = (ref_Y_val < 128) ? 235 : 16;
        buf[i*2+1] = 128;
    }
}

void CColorBar::Draw(BYTE* buffer, int height, int width)
{
    int left, right, top, bottom;

    // Calculate the exact coordinates of rectangular zone in the buffer
    left = width * m_LeftBorder / 10000;
    if (left < 0)
    {
        left = 0;
    }
    else if (left >= width)
    {
        left = width - 1;
    }
    right = width * m_RightBorder / 10000;
    if (right < 0)
    {
        right = 0;
    }
    else if (right >= width)
    {
        right = width - 1;
    }
    top = height * m_TopBorder / 10000;
    if (top < 0)
    {
        top = 0;
    }
    else if (top >= height)
    {
        top = height - 1;
    }
    bottom = height * m_BottomBorder / 10000;
    if (bottom < 0)
    {
        bottom = 0;
    }
    else if (bottom >= height)
    {
        bottom = height - 1;
    }

    for (int i = top ; i <= bottom ; i++)
    {
        for (int j = left ; j <= right ; j++)
        {
            buffer[i*width*2+j*2  ] = ref_Y_val;
            buffer[i*width*2+j*2+1] = (j % 2) ? ref_V_val : ref_U_val;
        }
    }
}

// Convert RGB to YUV
void CColorBar::RGB2YUV(unsigned char R, unsigned char G, unsigned char B, unsigned char* pY, unsigned char* pU, unsigned char* pV)
{
    unsigned int y, cr, cb;

//    y  = ( 16840*R + 33058*G +  6405*B + 1048576)>>16;
//    cr = ( 28781*R - 24110*G -  4671*B + 8388608)>>16;
//    cb = ( -9713*R - 19068*G + 28781*B + 8388608)>>16;
    y  = ( 16843*R + 33030*G +  6423*B + 1048576)>>16;
    cr = ( 28770*R - 24117*G -  4653*B + 8388608)>>16;
    cb = ( -9699*R - 19071*G + 28770*B + 8388608)>>16;

    *pY = LIMIT(y);
    *pU = LIMIT(cb);
    *pV = LIMIT(cr);
}

// Convert YUV to RGB
void CColorBar::YUV2RGB(unsigned char Y, unsigned char U, unsigned char V, unsigned char* pR, unsigned char* pG, unsigned char* pB)
{
    int y, cr, cb, r, g, b;

    cb = U - 128;
    cr = V - 128;
    y = Y - 16;

    r = ( 76284*y + 104595*cr             )>>16;
    g = ( 76284*y -  53281*cr -  25625*cb )>>16;
    b = ( 76284*y             + 132252*cb )>>16;

    *pR = LIMIT(r);
    *pG = LIMIT(g);
    *pB = LIMIT(b);
}


/////////////////////////////////////////////////////////////////////////////
// Class CSubPattern

CSubPattern::CSubPattern(eTypeAdjust type)
{
    m_TypeAdjust = type;
}

CSubPattern::~CSubPattern()
{
    m_ColorBars.clear();
}

// This method returns the type of settings that can be adjusted with this sub-pattern
eTypeAdjust CSubPattern::GetTypeAdjust()
{
    return m_TypeAdjust;
}

// This method analyzes the current overlay buffer
BOOL CSubPattern::CalcCurrentSubPattern(BOOL reinit, unsigned int nb_calc_needed, TDeinterlaceInfo* pInfo)
{
    BOOL result_avail;

    // Do the job for each defined color bar
    for(vector<CColorBar*>::iterator it = m_ColorBars.begin(); 
        it != m_ColorBars.end(); 
        ++it)
    {
        result_avail = (*it)->CalcAvgColor(reinit, nb_calc_needed, pInfo);
    }

    return result_avail;
}

// This methode returns the sum of absolute delta between reference color
// and calculated average color through all the color bars
void CSubPattern::GetSumDeltaColor(BOOL YUV, int* pR_Y, int* pG_U, int* pB_V, int* pTotal)
{
    int delta[4];
    int sum_delta[4];

    // Set the sums to 0
    for (int j = 0 ; j <= 3 ; j++)
    {
        sum_delta[j] = 0;
    }
    // Go through all the color bars
    for(vector<CColorBar*>::iterator it = m_ColorBars.begin(); 
        it != m_ColorBars.end(); 
        ++it)
    {
        (*it)->GetDeltaColor(YUV, &delta[0], &delta[1], &delta[2], &delta[3]);
        for (j = 0 ; j <= 3 ; j++)
        {
            sum_delta[j] += ABSOLUTE_VALUE(delta[j]);
        }
    }

    *pR_Y = sum_delta[0];
    *pG_U = sum_delta[1];
    *pB_V = sum_delta[2];
    *pTotal = sum_delta[3];
}
	
// This method draws in the video signal rectangles around each color bar of the sub-pattern
void CSubPattern::DrawPositions(TDeinterlaceInfo* pInfo)
{
    // Do the job for each defined color bar
    for(vector<CColorBar*>::iterator it = m_ColorBars.begin(); 
        it != m_ColorBars.end(); 
        ++it)
    {
        (*it)->DrawPosition(pInfo);
    }
}

/////////////////////////////////////////////////////////////////////////////
// Class CTestPattern

CTestPattern::CTestPattern(char* name, int height)
{
    strcpy(m_PatternName, name);
    m_Width = 720;
    m_Height = height;
}

CTestPattern::CTestPattern(LPCSTR FileName)
{
    CColorBar* color_bar;
    CColorBar* color_bar1;
    CColorBar* color_bar2;
    CColorBar* color_bar3;
    CColorBar* color_bar4;
    CColorBar* color_bar5;
    CColorBar* color_bar6;
    CColorBar* color_bar7;
    CColorBar* color_bar8;
    CColorBar* color_bar9;
    CColorBar* color_bar10;
    CColorBar* color_bar11;
    CSubPattern* sub_pattern;
    char BufferLine[512];
    char *Buffer;
    FILE* FilePat;
    int i_val[16];
    char s_val[512];
    int n;
    eTypeAdjust TypeAdjust;

    if (!strcmp(FileName, "THX_NTSC_1.pat"))
    {

    strcpy(m_PatternName, "THX Optimode Ntsc - Monitor Performance");
    m_Width = 720;
    m_Height = 480;

    color_bar1 = new CColorBar( 347,  764, 6875, 7500, FALSE,   0,   0,   0);
    m_ColorBars.push_back(color_bar1);
    color_bar2 = new CColorBar(1181, 1597, 6875, 7500, FALSE,  22,  24,  23);
    m_ColorBars.push_back(color_bar2);
    color_bar3 = new CColorBar(2014, 2431, 6875, 7500, FALSE,  48,  50,  49);
    m_ColorBars.push_back(color_bar3);
    color_bar4 = new CColorBar(7083, 7500, 6875, 7500, FALSE, 201, 203, 202);
    m_ColorBars.push_back(color_bar4);
    color_bar5 = new CColorBar(7917, 8333, 6875, 7500, FALSE, 227, 228, 227);
    m_ColorBars.push_back(color_bar5);
    color_bar6 = new CColorBar(8750, 9167, 6875, 7500, FALSE, 252, 253, 252);
    m_ColorBars.push_back(color_bar6);

    sub_pattern = new CSubPattern(ADJ_BRIGHTNESS_CONTRAST);
    sub_pattern->m_ColorBars.push_back(color_bar1);
    sub_pattern->m_ColorBars.push_back(color_bar2);
    sub_pattern->m_ColorBars.push_back(color_bar3);
    sub_pattern->m_ColorBars.push_back(color_bar4);
    sub_pattern->m_ColorBars.push_back(color_bar5);
    sub_pattern->m_ColorBars.push_back(color_bar6);
    m_SubPatterns.push_back(sub_pattern);

    sub_pattern = new CSubPattern(ADJ_BRIGHTNESS);
    sub_pattern->m_ColorBars.push_back(color_bar1);
    sub_pattern->m_ColorBars.push_back(color_bar2);
    sub_pattern->m_ColorBars.push_back(color_bar3);
    m_SubPatterns.push_back(sub_pattern);

    sub_pattern = new CSubPattern(ADJ_CONTRAST);
    sub_pattern->m_ColorBars.push_back(color_bar4);
    sub_pattern->m_ColorBars.push_back(color_bar5);
    sub_pattern->m_ColorBars.push_back(color_bar6);
    m_SubPatterns.push_back(sub_pattern);

    color_bar1 = new CColorBar( 278, 1042, 2396, 4167, FALSE, 251, 252, 251);
    m_ColorBars.push_back(color_bar1);
    color_bar2 = new CColorBar(1458, 2222, 2396, 4167, FALSE, 188, 190,   0);
    m_ColorBars.push_back(color_bar2);
    color_bar3 = new CColorBar(2708, 3472, 2396, 4167, FALSE,   0, 188, 185);
    m_ColorBars.push_back(color_bar3);
    color_bar4 = new CColorBar(3889, 4653, 2396, 4167, FALSE,   0, 188,   0);
    m_ColorBars.push_back(color_bar4);
    color_bar5 = new CColorBar(5139, 5903, 2396, 4167, FALSE, 187,   0, 187);
    m_ColorBars.push_back(color_bar5);
    color_bar6 = new CColorBar(6319, 7083, 2396, 4167, FALSE, 186,   0,   0);
    m_ColorBars.push_back(color_bar6);
    color_bar7 = new CColorBar(7569, 8333, 2396, 4167, FALSE,   0,   0, 187);
    m_ColorBars.push_back(color_bar7);
    color_bar8 = new CColorBar(8750, 9514, 2396, 4167, FALSE,   0,   0,   0);
    m_ColorBars.push_back(color_bar8);
    
    sub_pattern = new CSubPattern(ADJ_COLOR);
    sub_pattern->m_ColorBars.push_back(color_bar1);
    sub_pattern->m_ColorBars.push_back(color_bar2);
    sub_pattern->m_ColorBars.push_back(color_bar3);
    sub_pattern->m_ColorBars.push_back(color_bar4);
    sub_pattern->m_ColorBars.push_back(color_bar5);
    sub_pattern->m_ColorBars.push_back(color_bar6);
    sub_pattern->m_ColorBars.push_back(color_bar7);
    sub_pattern->m_ColorBars.push_back(color_bar8);
    m_SubPatterns.push_back(sub_pattern);

    sub_pattern = new CSubPattern(ADJ_SATURATION_U);
    sub_pattern->m_ColorBars.push_back(color_bar7);
    m_SubPatterns.push_back(sub_pattern);

    sub_pattern = new CSubPattern(ADJ_SATURATION_V);
    sub_pattern->m_ColorBars.push_back(color_bar6);
    m_SubPatterns.push_back(sub_pattern);

    sub_pattern = new CSubPattern(ADJ_HUE);
    sub_pattern->m_ColorBars.push_back(color_bar3);
    sub_pattern->m_ColorBars.push_back(color_bar4);
    sub_pattern->m_ColorBars.push_back(color_bar5);
    m_SubPatterns.push_back(sub_pattern);

    CreateGlobalSubPattern();

    }
    else if (!strcmp(FileName, "THX_NTSC_2.pat"))
    {

    strcpy(m_PatternName, "THX Optimode Ntsc - tint color");
    m_Width = 720;
    m_Height = 480;

    color_bar1 = new CColorBar( 417, 1111, 2500, 5625, FALSE, 188, 189, 188);
    m_ColorBars.push_back(color_bar1);
    color_bar2 = new CColorBar(1806, 2500, 2500, 5625, FALSE, 189, 190,   0);
    m_ColorBars.push_back(color_bar2);
    color_bar3 = new CColorBar(3194, 3889, 2500, 5625, FALSE,   0, 190, 188);
    m_ColorBars.push_back(color_bar3);
    color_bar4 = new CColorBar(4583, 5278, 2500, 5625, FALSE,   0, 189,   0);
    m_ColorBars.push_back(color_bar4);
    color_bar5 = new CColorBar(6042, 6736, 2500, 5625, FALSE, 188,   0, 190);
    m_ColorBars.push_back(color_bar5);
    color_bar6 = new CColorBar(7431, 8125, 2500, 5625, FALSE, 189,   0,   0);
    m_ColorBars.push_back(color_bar6);
    color_bar7 = new CColorBar(8889, 9583, 2500, 5625, FALSE,   0,   0, 189);
    m_ColorBars.push_back(color_bar7);
    
    sub_pattern = new CSubPattern(ADJ_COLOR);
    sub_pattern->m_ColorBars.push_back(color_bar1);
    sub_pattern->m_ColorBars.push_back(color_bar2);
    sub_pattern->m_ColorBars.push_back(color_bar3);
    sub_pattern->m_ColorBars.push_back(color_bar4);
    sub_pattern->m_ColorBars.push_back(color_bar5);
    sub_pattern->m_ColorBars.push_back(color_bar6);
    sub_pattern->m_ColorBars.push_back(color_bar7);
    m_SubPatterns.push_back(sub_pattern);

    sub_pattern = new CSubPattern(ADJ_SATURATION_U);
    sub_pattern->m_ColorBars.push_back(color_bar7);
    m_SubPatterns.push_back(sub_pattern);

    sub_pattern = new CSubPattern(ADJ_SATURATION_V);
    sub_pattern->m_ColorBars.push_back(color_bar6);
    m_SubPatterns.push_back(sub_pattern);

    sub_pattern = new CSubPattern(ADJ_HUE);
    sub_pattern->m_ColorBars.push_back(color_bar3);
    sub_pattern->m_ColorBars.push_back(color_bar4);
    sub_pattern->m_ColorBars.push_back(color_bar5);
    m_SubPatterns.push_back(sub_pattern);

    CreateGlobalSubPattern();

    }
    else if (!strcmp(FileName, "AVIA_NTSC_1.pat"))
    {

    strcpy(m_PatternName, "AVIA - T 1 C 7 - range of gray");
    m_Width = 720;
    m_Height = 480;
    
    color_bar1 = new CColorBar( 208,  764, 2500, 7500, FALSE, 254, 251, 255);
    m_ColorBars.push_back(color_bar1);
    color_bar2 = new CColorBar(1111, 1667, 2500, 7500, FALSE, 226, 223, 227);
    m_ColorBars.push_back(color_bar2);
    color_bar3 = new CColorBar(2014, 2569, 2500, 7500, FALSE, 200, 197, 201);
    m_ColorBars.push_back(color_bar3);
    color_bar4 = new CColorBar(2917, 3472, 2500, 7500, FALSE, 172, 169, 173);
    m_ColorBars.push_back(color_bar4);
    color_bar5 = new CColorBar(3819, 4375, 2500, 7500, FALSE, 145, 142, 146);
    m_ColorBars.push_back(color_bar5);
    color_bar6 = new CColorBar(4722, 5278, 2500, 7500, FALSE, 117, 114, 118);
    m_ColorBars.push_back(color_bar6);
    color_bar7 = new CColorBar(5625, 6181, 2500, 7500, FALSE,  90,  87,  91);
    m_ColorBars.push_back(color_bar7);
    color_bar8 = new CColorBar(6528, 7083, 2500, 7500, FALSE,  62,  59,  63);
    m_ColorBars.push_back(color_bar8);
    color_bar9 = new CColorBar(7431, 7986, 2500, 7500, FALSE,  34,  31,  35);
    m_ColorBars.push_back(color_bar9);
    color_bar10 = new CColorBar(8333, 8889, 2500, 7500, FALSE,   7,   4,   8);
    m_ColorBars.push_back(color_bar10);
    color_bar11 = new CColorBar(9236, 9792, 2500, 7500, FALSE,   1,   0,   2);
    m_ColorBars.push_back(color_bar11);
    
    sub_pattern = new CSubPattern(ADJ_BRIGHTNESS_CONTRAST);
    sub_pattern->m_ColorBars.push_back(color_bar1);
    sub_pattern->m_ColorBars.push_back(color_bar2);
    sub_pattern->m_ColorBars.push_back(color_bar3);
    sub_pattern->m_ColorBars.push_back(color_bar4);
    sub_pattern->m_ColorBars.push_back(color_bar5);
    sub_pattern->m_ColorBars.push_back(color_bar6);
    sub_pattern->m_ColorBars.push_back(color_bar7);
    sub_pattern->m_ColorBars.push_back(color_bar8);
    sub_pattern->m_ColorBars.push_back(color_bar9);
    sub_pattern->m_ColorBars.push_back(color_bar10);
    sub_pattern->m_ColorBars.push_back(color_bar11);
    m_SubPatterns.push_back(sub_pattern);

    sub_pattern = new CSubPattern(ADJ_BRIGHTNESS);
    sub_pattern->m_ColorBars.push_back(color_bar8);
    sub_pattern->m_ColorBars.push_back(color_bar9);
    sub_pattern->m_ColorBars.push_back(color_bar10);
    sub_pattern->m_ColorBars.push_back(color_bar11);
    m_SubPatterns.push_back(sub_pattern);

    sub_pattern = new CSubPattern(ADJ_CONTRAST);
    sub_pattern->m_ColorBars.push_back(color_bar1);
    sub_pattern->m_ColorBars.push_back(color_bar2);
    sub_pattern->m_ColorBars.push_back(color_bar3);
    sub_pattern->m_ColorBars.push_back(color_bar4);
    m_SubPatterns.push_back(sub_pattern);

    CreateGlobalSubPattern();

    }
    else if (!strcmp(FileName, "AVIA_NTSC_2.pat"))
    {

    strcpy(m_PatternName, "AVIA - T 4 C 4 - color bars");
    m_Width = 720;
    m_Height = 480;
    
    color_bar1 = new CColorBar( 417, 1111, 2083, 4167, FALSE, 190, 190, 190);
    m_ColorBars.push_back(color_bar1);
    color_bar2 = new CColorBar(1806, 2500, 2083, 4167, FALSE, 190, 189,   0);
    m_ColorBars.push_back(color_bar2);
    color_bar3 = new CColorBar(3194, 3889, 2083, 4167, FALSE,   0, 189, 188);
    m_ColorBars.push_back(color_bar3);
    color_bar4 = new CColorBar(4583, 5278, 2083, 4167, FALSE,   0, 189,   0);
    m_ColorBars.push_back(color_bar4);
    color_bar5 = new CColorBar(6042, 6736, 2083, 4167, FALSE, 190,   0, 191);
    m_ColorBars.push_back(color_bar5);
    color_bar6 = new CColorBar(7431, 8125, 2083, 4167, FALSE, 190,   0,   0);
    m_ColorBars.push_back(color_bar6);
    color_bar7 = new CColorBar(8889, 9583, 2083, 4167, FALSE,   0,   0, 190);
    m_ColorBars.push_back(color_bar7);
    
    sub_pattern = new CSubPattern(ADJ_COLOR);
    sub_pattern->m_ColorBars.push_back(color_bar1);
    sub_pattern->m_ColorBars.push_back(color_bar2);
    sub_pattern->m_ColorBars.push_back(color_bar3);
    sub_pattern->m_ColorBars.push_back(color_bar4);
    sub_pattern->m_ColorBars.push_back(color_bar5);
    sub_pattern->m_ColorBars.push_back(color_bar6);
    sub_pattern->m_ColorBars.push_back(color_bar7);
    m_SubPatterns.push_back(sub_pattern);

    sub_pattern = new CSubPattern(ADJ_SATURATION_U);
    sub_pattern->m_ColorBars.push_back(color_bar7);
    m_SubPatterns.push_back(sub_pattern);

    sub_pattern = new CSubPattern(ADJ_SATURATION_V);
    sub_pattern->m_ColorBars.push_back(color_bar6);
    m_SubPatterns.push_back(sub_pattern);

    sub_pattern = new CSubPattern(ADJ_HUE);
    sub_pattern->m_ColorBars.push_back(color_bar3);
    sub_pattern->m_ColorBars.push_back(color_bar4);
    sub_pattern->m_ColorBars.push_back(color_bar5);
    m_SubPatterns.push_back(sub_pattern);

    CreateGlobalSubPattern();

    }
    else
    {

    FilePat = fopen(FileName, "r");
    if (!FilePat)
    {
        m_PatternName[0] = '\0';
        m_Width = 0;
        m_Height = 0;
        return;
    }

    while(!feof(FilePat))
    {
        if(fgets(BufferLine, 512, FilePat))
        {
            BufferLine[511] = '\0';
            Buffer = BufferLine;
            while(strlen(Buffer) > 0 && *Buffer <= ' ')
            {
                Buffer++;
            }
            if(strlen(Buffer) == 0 || *Buffer == '#' || *Buffer == ';')
            {
                continue;
            }
            while(strlen(Buffer) > 0 && Buffer[strlen(Buffer) - 1] <= ' ')
            {
                Buffer[strlen(Buffer) - 1] = '\0';
            }
            if (strlen(Buffer) == 0)
            {
                continue;
            }
            if (sscanf(Buffer, "PAT %d %d %s", &i_val[0], &i_val[1], s_val) == 3)
            {
                LOG(5,"PAT %d %d %s", i_val[0], i_val[1], s_val);
                m_Width = i_val[0];
                m_Height = i_val[1];
                strcpy(m_PatternName, strstr(&Buffer[4], s_val));
            }
            else if (sscanf(Buffer, "RECT %d %d %d %d %s %d %d %d", &i_val[0], &i_val[1], &i_val[2], &i_val[3], s_val, &i_val[4], &i_val[5], &i_val[6]) == 8)
            {
                if (!strcmp(s_val, "RGB"))
                {
                    LOG(5,"RECT RGB %d %d %d %d %d %d %d", i_val[0], i_val[1], i_val[2], i_val[3], i_val[4], i_val[5], i_val[6]);
                    color_bar = new CColorBar(i_val[0], i_val[1], i_val[2], i_val[3], FALSE, i_val[4], i_val[5], i_val[6]);
                    m_ColorBars.push_back(color_bar);
                }
                else if (!strcmp(s_val, "YUV"))
                {
                    LOG(5,"RECT YUV %d %d %d %d %d %d %d", i_val[0], i_val[1], i_val[2], i_val[3], i_val[4], i_val[5], i_val[6]);
                    color_bar = new CColorBar(i_val[0], i_val[1], i_val[2], i_val[3], TRUE, i_val[4], i_val[5], i_val[6]);
                    m_ColorBars.push_back(color_bar);
                }
            }
            else if ((n = sscanf(Buffer, "GRP %s %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", s_val, &i_val[0], &i_val[1], &i_val[2], &i_val[3], &i_val[4], &i_val[5], &i_val[6], &i_val[7], &i_val[8], &i_val[9], &i_val[10], &i_val[11], &i_val[12], &i_val[13], &i_val[14], &i_val[15])) >= 2)
            {
                LOG(5,"GRP %s", s_val);
                if (!strcmp(s_val, "BRIGHTNESS_CONTRAST"))
                {
                    TypeAdjust = ADJ_BRIGHTNESS_CONTRAST;
                }
                else if (!strcmp(s_val, "BRIGHTNESS"))
                {
                    TypeAdjust = ADJ_BRIGHTNESS;
                }
                else if (!strcmp(s_val, "CONTRAST"))
                {
                    TypeAdjust = ADJ_CONTRAST;
                }
                else if (!strcmp(s_val, "COLOR"))
                {
                    TypeAdjust = ADJ_COLOR;
                }
                else if (!strcmp(s_val, "SATURATION_U"))
                {
                    TypeAdjust = ADJ_SATURATION_U;
                }
                else if (!strcmp(s_val, "SATURATION_V"))
                {
                    TypeAdjust = ADJ_SATURATION_V;
                }
                else if (!strcmp(s_val, "HUE"))
                {
                    TypeAdjust = ADJ_HUE;
                }
                else
                {
                    TypeAdjust = ADJ_MANUAL   ;
                }
                if (TypeAdjust != ADJ_MANUAL)
                {
                    sub_pattern = new CSubPattern(TypeAdjust);
                    m_SubPatterns.push_back(sub_pattern);
                    for (int i(0) ; i < (n-1) ; i++)
                    {
                        if (i_val[i] > 0 && i_val[i] <= m_ColorBars.size())
                        {
                            LOG(5,"GRP %d", i_val[i]);
                            sub_pattern->m_ColorBars.push_back(m_ColorBars[i_val[i]-1]);
                        }
                    }
                }
            }
        }
    }

    fclose(FilePat);

    CreateGlobalSubPattern();

    Log();
    }
}

CTestPattern::~CTestPattern()
{
    for(vector<CSubPattern*>::iterator it = m_SubPatterns.begin(); 
        it != m_SubPatterns.end(); 
        ++it)
    {
        delete *it;
    }
    m_SubPatterns.clear();
    for(vector<CColorBar*>::iterator it2 = m_ColorBars.begin(); 
        it2 != m_ColorBars.end(); 
        ++it2)
    {
        delete *it2;
    }
    m_ColorBars.clear();
}

// This method returns the name of the test pattern
char* CTestPattern::GetName()
{
    return m_PatternName;
}

// This method returns the width of the test pattern
int CTestPattern::GetWidth()
{
    return m_Width;
}

// This method returns the height (number of lines) of the test pattern
int CTestPattern::GetHeight()
{
    return m_Height;
}

// This method allows to create a new sub-pattern to the test pattern
// which is a merge of all the others sub-patterns
// Returns 0 if the sub-pattern is correctly created
void CTestPattern::CreateGlobalSubPattern()
{
    CSubPattern* sub_pattern;

    // Create the new sub-pattern
    sub_pattern = new CSubPattern(ADJ_MANUAL);

    for(vector<CColorBar*>::iterator it = m_ColorBars.begin(); 
        it != m_ColorBars.end(); 
        ++it)
    {
        sub_pattern->m_ColorBars.push_back(*it);
    }
    m_SubPatterns.push_back(sub_pattern);
}

// This method determines the type of content in the test pattern
// going all over the sub-patterns
eTypeContentPattern CTestPattern::DetermineTypeContent()
{
    if ( (GetSubPattern(ADJ_BRIGHTNESS) != NULL)
      && (GetSubPattern(ADJ_CONTRAST) != NULL)
      && (GetSubPattern(ADJ_SATURATION_U) != NULL)
      && (GetSubPattern(ADJ_SATURATION_V) != NULL)
      && (GetSubPattern(ADJ_HUE) != NULL) )
        return PAT_GRAY_AND_COLOR;

    if ( (GetSubPattern(ADJ_BRIGHTNESS) != NULL)
      && (GetSubPattern(ADJ_CONTRAST) != NULL) )
        return PAT_RANGE_OF_GRAY;

    if ( (GetSubPattern(ADJ_SATURATION_U) != NULL)
      && (GetSubPattern(ADJ_SATURATION_V) != NULL)
      && (GetSubPattern(ADJ_HUE) != NULL) )
        return PAT_COLOR;

    return PAT_UNKNOWN;
}

// This method returns the (first) sub-pattern allowing to adjust particular settings
// Returns NULL pointer if no sub-pattern allows to do this type of adjustments
CSubPattern* CTestPattern::GetSubPattern(eTypeAdjust type_adjust)
{
    for(vector<CSubPattern*>::iterator it = m_SubPatterns.begin(); 
        it != m_SubPatterns.end(); 
        ++it)
    {
        if ((*it)->GetTypeAdjust() == type_adjust)
        {
            return *it;
        }
    }
    return NULL;
}

void CTestPattern::Draw(BYTE* buffer)
{
    // Background
    for (int i = 0 ; i < m_Height ; i++)
    {
        for (int j = 0 ; j < m_Width ; j++)
        {
            buffer[i*m_Width*2+j*2  ] = 235;
            buffer[i*m_Width*2+j*2+1] = 128;
        }
    }

    // Do the job for each defined color bar
    for(vector<CColorBar*>::iterator it = m_ColorBars.begin(); 
        it != m_ColorBars.end(); 
        ++it)
    {
        (*it)->Draw(buffer, m_Height, m_Width);
    }
}

void CTestPattern::Log()
{
    unsigned char R, G, B, Y, U, V;
    unsigned short int left, right, top, bottom;

    LOG(3, "Pattern %s %dx%d", m_PatternName, m_Width, m_Height);
    for(vector<CColorBar*>::iterator it2 = m_ColorBars.begin(); 
        it2 != m_ColorBars.end(); 
        ++it2)
    {
        (*it2)->GetPosition(&left, &right, &top, &bottom);
        (*it2)->GetRefColor(FALSE, &R, &G, &B);
        (*it2)->GetRefColor(TRUE, &Y, &U, &V);
        LOG(3, "   T %4d B %4d L %4d R %4d - RGB %3d %3d %3d YUV %3d %3d %3d", top, bottom, left, right, R, G, B, Y, U, V);
    }
    for(vector<CSubPattern*>::iterator it = m_SubPatterns.begin(); 
        it != m_SubPatterns.end(); 
        ++it)
    {
        LOG(3, "   Sub-pattern %d", (*it)->GetTypeAdjust());
        for(vector<CColorBar*>::iterator it2 = (*it)->m_ColorBars.begin(); 
            it2 != (*it)->m_ColorBars.end(); 
            ++it2)
        {
            (*it2)->GetPosition(&left, &right, &top, &bottom);
            LOG(3, "      T %4d B %4d L %4d R %4d", top, bottom, left, right);
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
// Class CCalSetting

CCalSetting::CCalSetting(CSimpleSetting* pSetting)
{
    m_pSetting = pSetting;
    min = pSetting->GetMin();
    max = pSetting->GetMax();
    current_value = pSetting->GetValue();
    SetFullRange();
    InitResult();
}

BOOL CCalSetting::Update()
{
    int new_value = m_pSetting->GetValue();
    if (new_value != current_value)
    {
        current_value = new_value;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

void CCalSetting::Save()
{
    char szText[256];
    saved_value = current_value;
    m_pSetting->GetDisplayText(szText);
    LOG(2, "Automatic Calibration - %s saved value = %d", szText, saved_value);
}

void CCalSetting::Restore()
{
    char szText[256];
    saved_value = current_value;
    m_pSetting->GetDisplayText(szText);
    LOG(2, "Automatic Calibration - %s restored value = %d", szText, saved_value);
}

void CCalSetting::SetFullRange()
{
    SetRange(min, max);
}

void CCalSetting::SetRange(int min_val, int max_val)
{
    int i, j;

    min_value = min_val;
    max_value = max_val;
    for (i=0 ; i<16 ; i++)
    {
        mask_input[i] = 0;
    }
    for (i=min_val ; i<=max_val ; i++)
    {
        j = i - min;
        mask_input[j/32] |= (1 << (j%32));
    }
    char szText[256];
    m_pSetting->GetDisplayText(szText);
    LOG(3, "Automatic Calibration - %s range => min = %d max = %d", szText, min_value, max_value);
}

void CCalSetting::SetRange(int delta)
{
    int min_val, max_val;

    min_val = m_pSetting->GetValue() - delta;
    if (min_val < min)
    {
        min_val = min;
    }
    max_val = m_pSetting->GetValue() + delta;
    if (max_val > max)
    {
        max_val = max;
    }
    SetRange(min_val, max_val);
}

void CCalSetting::SetRange(int* mask)
{
    int i, nb;

    for (i=0 ; i<16 ; i++)
    {
        mask_input[i] = mask[i];
    }
    for (i=0,nb=0 ; i<=(max - min) ; i++)
    {
        if (mask[i/32] & (1 << (i%32)))
        {
            nb++;
            if (nb == 1)
            {
                min_value = i + min;
                max_value = i + min;
            }
            else
            {
                max_value = i + min;
            }
        }
    }
}

int CCalSetting::GetRange(int* mask, int* min_val, int* max_val)
{
    int i, nb;

    for (i=0,nb=0 ; i<=(max - min) ; i++)
    {
        if (mask_input[i/32] & (1 << (i%32)))
        {
            nb++;
        }
    }

    for (i=0 ; i<16; i++)
    {
        mask[i] = mask_input[i];
    }
   * min_val = min_value;
   * max_val = max_value;

    return nb;
}

void CCalSetting::AdjustMin()
{
    Adjust(min_value);
}

void CCalSetting::AdjustMax()
{
    Adjust(max_value);
}

void CCalSetting::AdjustDefault()
{
    Adjust(m_pSetting->GetDefault());
}

BOOL CCalSetting::AdjustNext()
{
    int i, j;

    if (end)
    {
        return FALSE;
    }

    for (i=(current_value+1) ; i<=max_value ; i++)
    {
        j = i - min;
        if (mask_input[j/32] & (1 << (j%32)))
        {
            Adjust(i);
            return TRUE;
        }
    }

    return FALSE;
}

void CCalSetting::AdjustBest()
{
    int nb_min;
    int best_val_min;
    int best_val_max;
    int mask[16];

    nb_min = GetResult(&mask[0], &best_val_min, &best_val_max);
    if (nb_min > 0)
    {
        // Set the setting to one of the best found values
        Adjust(best_val_max);
    }
    else
    {
        // Set the setting to its default value
        AdjustDefault();
    }
    char szText[256];
    m_pSetting->GetDisplayText(szText);
    LOG(2, "Automatic Calibration - %s finished - %d values between %d and %d => %d", szText, nb_min, best_val_min, best_val_max, current_value);
}

void CCalSetting::InitResult()
{
    int i;

    min_diff = MAX_VALUE;
    max_diff = 0;
    desc = FALSE;
    for (i=0 ; i<16 ; i++)
    {
        mask_output[i] = 0;
    }
    end = FALSE;
}

BOOL CCalSetting::UpdateResult(int diff, int threshold, BOOL only_one)
{
    int i, j;
    BOOL min_found = FALSE;

    if (diff > max_diff)
    {
        max_diff = diff;
    }
    i = current_value - min;
    if (diff < min_diff)
    {
        min_diff = diff;

        for (j=0 ; j<16 ; j++)
        {
            mask_output[j] = 0;
        }
        mask_output[i/32] = (1 << (i%32));

        if ((threshold >= 0) && ((max_diff - min_diff) > threshold))
        {
            desc = TRUE;
        }
    }
    else if ((diff == min_diff) && !only_one)
    {
        mask_output[i/32] |= (1 << (i%32));
    }
    else if ((threshold >= 0) && ((diff - min_diff) > threshold))
    {
        end = TRUE;
        min_found = desc;
    }
    char szText[256];
    m_pSetting->GetDisplayText(szText);
    LOG(3, "Automatic Calibration - %s value %d => result = %d min = %d", szText, current_value, diff, min_diff);
    return min_found;
}

int CCalSetting::GetResult(int* mask, int* min_val, int* max_val)
{
    int i;
    int nb_min;
    int best_val_min;
    int best_val_max;

    for (i=0,nb_min=0 ; i<=(max - min) ; i++)
    {
        if (mask_output[i/32] & (1 << (i%32)))
        {
            nb_min++;
            if (nb_min == 1)
            {
                best_val_min = i + min;
                best_val_max = i + min;
            }
            else
            {
                best_val_max = i + min;
            }
        }
    }

    if (nb_min > 0)
    {
        for (i=0 ; i<16; i++)
        {
            mask[i] = mask_output[i];
        }
       * min_val = best_val_min;
       * max_val = best_val_max;
    }

    return nb_min;
}

void CCalSetting::Adjust(int value)
{
    current_value = value;
    m_pSetting->SetValue(current_value);
}

/////////////////////////////////////////////////////////////////////////////
// Class CCalibration

CCalibration::CCalibration()
{
    m_CurTestPat = NULL;
    m_CurSubPat = NULL;
    m_TypeCalibration = CAL_MANUAL;
    m_IsRunning = FALSE;

    brightness   = NULL;
    contrast     = NULL;
    saturation_U = NULL;
    saturation_V = NULL;
    hue          = NULL;

    last_tick_count = -1;
    LoadTestPatterns();
}

CCalibration::~CCalibration()
{
    UnloadTestPatterns();

    if (brightness != NULL)
        delete brightness;
    if (contrast != NULL)
        delete contrast;
    if (saturation_U != NULL)
        delete saturation_U;
    if (saturation_V != NULL)
        delete saturation_V;
    if (hue != NULL)
        delete hue;
}

// This method loads all the predefined test patterns
void CCalibration::LoadTestPatterns()
{
    CTestPattern* pattern;
    char BufferLine[512];
    char *Buffer;
    struct stat st;
    FILE* File;
    
    File = fopen("patterns/card_calibr.d3u", "r");
    if(File != NULL)
    {
        while(!feof(File))
        {
            if(fgets(BufferLine, 512, File))
            {
                BufferLine[511] = '\0';
                Buffer = BufferLine;
                while(strlen(Buffer) > 0 && *Buffer <= ' ')
                {
                    Buffer++;
                }
                if(strlen(Buffer) > 0 && *Buffer != '#' && *Buffer != ';')
                {
                    // take care of stuff that is at end of the line
                    while(strlen(Buffer) > 0 && Buffer[strlen(Buffer) - 1] <= ' ')
                    {
                        Buffer[strlen(Buffer) - 1] = '\0';
                    }
                    if (strlen(Buffer) > 0 && !stat(Buffer, &st))
                    {
                        pattern = new CTestPattern(Buffer);
                        m_TestPatterns.push_back(pattern);
                    }
                }
            }
        }
        fclose(File);
    }
}

// This method unloads all the predefined test patterns
void CCalibration::UnloadTestPatterns()
{
    // Destroy all test patterns
    for(vector<CTestPattern*>::iterator it = m_TestPatterns.begin(); 
        it != m_TestPatterns.end(); 
        ++it)
    {
        delete *it;
    }
    m_TestPatterns.clear();
    m_CurTestPat = NULL;
    m_CurSubPat = NULL;
}

BOOL CCalibration::ProcessSelection(HWND hWnd, WORD wMenuId)
{
    int i = 0;
    for(vector<CTestPattern*>::iterator it = m_TestPatterns.begin(); 
        it != m_TestPatterns.end(); 
        ++it, ++i)
    {
		if (wMenuId == IDM_PATTERN_SELECT + i + 1)
        {
			pCalibration->SelectTestPattern(i);
			return TRUE;
        }
    }
	return FALSE;
}


void CCalibration::UpdateMenu(HMENU hMenu)
{
    HMENU           hMenuPatterns;
    MENUITEMINFO    MenuItemInfo;
    int             i;
	char*		    name;

    hMenuPatterns = GetPatternsSubmenu();
    if (hMenuPatterns == NULL) return;

    i = GetMenuItemCount(hMenuPatterns);
    while (i)
    {
        i--;
        RemoveMenu(hMenuPatterns, i, MF_BYPOSITION);
    }

    i = 0;
    for(vector<CTestPattern*>::iterator it = m_TestPatterns.begin(); 
        it != m_TestPatterns.end(); 
        ++it, ++i)
    {
		name = (*it)->GetName();

        MenuItemInfo.cbSize = sizeof (MenuItemInfo);
        MenuItemInfo.fType = MFT_STRING;
	    MenuItemInfo.dwTypeData = name;
		MenuItemInfo.cch = strlen (name);

        MenuItemInfo.fMask = MIIM_TYPE | MIIM_ID;
        MenuItemInfo.wID = IDM_PATTERN_SELECT + i + 1;
	    InsertMenuItem(hMenuPatterns, i, TRUE, &MenuItemInfo);
    }
}

void CCalibration::SetMenu(HMENU hMenu)
{
    HMENU   hMenuPatterns;
    int     i;
	char	*name;
    eTypeContentPattern type_content;

    if ((m_CurTestPat != NULL) && (m_CurTestPat->GetHeight() != Providers_GetCurrentSource()->GetHeight()))
    {
        m_CurTestPat = NULL;
    }

    if (m_CurTestPat != NULL)
    {
        type_content = m_CurTestPat->DetermineTypeContent();
    }
    else
    {
        type_content = PAT_UNKNOWN;
    }

    hMenuPatterns = GetPatternsSubmenu();
    if (hMenuPatterns == NULL) return;

    i = 0;
    for(vector<CTestPattern*>::iterator it = m_TestPatterns.begin(); 
        it != m_TestPatterns.end(); 
        ++it, ++i)
    {
		name = (*it)->GetName();
		EnableMenuItem(hMenuPatterns, i, m_IsRunning ? MF_BYPOSITION | MF_GRAYED : MF_BYPOSITION | MF_ENABLED);
		EnableMenuItem(hMenuPatterns, i, (m_IsRunning || ((*it)->GetHeight() != Providers_GetCurrentSource()->GetHeight())) ? MF_BYPOSITION | MF_GRAYED : MF_BYPOSITION | MF_ENABLED);
		CheckMenuItem(hMenuPatterns, i, (m_CurTestPat == (*it)) ? MF_BYPOSITION | MF_CHECKED : MF_BYPOSITION | MF_UNCHECKED);
    }
	
	EnableMenuItem(hMenu, IDM_START_MANUAL_CALIBRATION, (m_IsRunning || (m_CurTestPat == NULL)) ? MF_GRAYED : MF_ENABLED);
	EnableMenuItem(hMenu, IDM_START_AUTO_CALIBRATION, (m_IsRunning || (m_CurTestPat == NULL) || (type_content != PAT_GRAY_AND_COLOR)) ? MF_GRAYED : MF_ENABLED);
	EnableMenuItem(hMenu, IDM_START_AUTO_CALIBRATION2, (m_IsRunning || (m_CurTestPat == NULL) || ((type_content != PAT_GRAY_AND_COLOR) && (type_content != PAT_RANGE_OF_GRAY))) ? MF_GRAYED : MF_ENABLED);
	EnableMenuItem(hMenu, IDM_START_AUTO_CALIBRATION3, (m_IsRunning || (m_CurTestPat == NULL) || ((type_content != PAT_GRAY_AND_COLOR) && (type_content != PAT_COLOR))) ? MF_GRAYED : MF_ENABLED);
	EnableMenuItem(hMenu, IDM_STOP_CALIBRATION, (!m_IsRunning || (m_CurTestPat == NULL)) ? MF_GRAYED : MF_ENABLED);
}

void CCalibration::SelectTestPattern(int num)
{
    if ( (num >= 0) && (num < m_TestPatterns.size()) )
	{
		m_CurTestPat = m_TestPatterns[num];
	}
	else
	{
		m_CurTestPat = NULL;
	}
}

CTestPattern* CCalibration::GetCurrentTestPattern()
{
	return m_CurTestPat;
}

CSubPattern* CCalibration::GetSubPattern(eTypeAdjust type_adjust)
{
    CSubPattern* sub_pattern = NULL;

    if (m_CurTestPat != NULL)
    {
        sub_pattern = m_CurTestPat->GetSubPattern(type_adjust);
    }

    return sub_pattern;
}

CSubPattern* CCalibration::GetCurrentSubPattern()
{
	return m_CurSubPat;
}

void CCalibration::Start(eTypeCalibration type)
{
    if (m_CurTestPat == NULL)
        return;

    delete brightness;
    delete contrast;
    delete saturation_U;
    delete saturation_V;
    delete hue;

    CSource* pSource = Providers_GetCurrentSource();
    CSimpleSetting* pSetting = NULL;

    /// \todo this is bad coding sort this out
    brightness   = new CCalSetting(static_cast<CSimpleSetting*>(pSource->GetBrightness()));
    contrast     = new CCalSetting(static_cast<CSimpleSetting*>(pSource->GetContrast()));
    saturation_U = new CCalSetting(static_cast<CSimpleSetting*>(pSource->GetSaturationU()));
    saturation_V = new CCalSetting(static_cast<CSimpleSetting*>(pSource->GetSaturationV()));
    hue          = new CCalSetting(static_cast<CSimpleSetting*>(pSource->GetHue()));

	m_TypeCalibration = type;

    // Update the objet with current video settings
    brightness->Update();
    contrast->Update();
    saturation_U->Update();
    saturation_V->Update();
    hue->Update();

    // Save the current video settings to restore them later if necessary
    brightness->Save();
    contrast->Save();
    saturation_U->Save();
    saturation_V->Save();
    hue->Save();

    // Set the overscan to a value specific to calibration
    AspectSettings.InitialOverscan = SourceOverscan;
    WorkoutOverlaySize(TRUE);

    switch (m_TypeCalibration)
    {
    case CAL_AUTO_BRIGHT_CONTRAST:
        initial_step = 1;
        nb_steps = 11;
        brightness->AdjustDefault();
        contrast->AdjustDefault();
        break;
    case CAL_AUTO_COLOR:
        initial_step = 12;
        nb_steps = 12;
        saturation_U->AdjustDefault();
        saturation_V->AdjustDefault();
        hue->AdjustDefault();
        break;
    case CAL_AUTO_FULL:
        initial_step = 1;
        nb_steps = 23;
        brightness->AdjustDefault();
        contrast->AdjustDefault();
        saturation_U->AdjustDefault();
        saturation_V->AdjustDefault();
        hue->AdjustDefault();
        break;
    case CAL_MANUAL:
    default:
        initial_step = 0;
        nb_steps = 1;
        break;
    }
    current_step = initial_step;
    full_range = FALSE;
    nb_tries = 0;
    first_calc = TRUE;

    // Display the specific OSD screen
    OSD_ShowInfosScreen(hWnd, 4, 0);

    m_IsRunning = TRUE;
}

void CCalibration::Stop()
{
    if (m_TypeCalibration != CAL_MANUAL)
    {
        OSD_ShowInfosScreen(hWnd, 4, 0);
        if ( (current_step != -1)
          || (MessageBox(hWnd, "Do you want to keep the current settings ?", "DScaler Question", MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON1) == IDNO) )
        {
            brightness->Restore();
            contrast->Restore();
            saturation_U->Restore();
            saturation_V->Restore();
            hue->Restore();
        }
    }

    // Restore the usual overscan
    Providers_GetCurrentSource()->SetOverscan();
    WorkoutOverlaySize(TRUE);

    // Erase the OSD screen
    OSD_Clear(hWnd);

	m_IsRunning = FALSE;
}

BOOL CCalibration::IsRunning()
{
	return m_IsRunning;
}

BOOL CCalibration::GetCurrentStep()
{
	return current_step;
}

eTypeCalibration CCalibration::GetType()
{
	return m_TypeCalibration;
}

void CCalibration::Make(TDeinterlaceInfo* pInfo, int tick_count)
{
    int nb1, nb2, nb3;
    int min, max;
    int mask[16];
    BOOL new_settings;
    BOOL found;

	if (!m_IsRunning
	 || (m_CurTestPat == NULL))
		return;

	if ((last_tick_count != -1) && ((tick_count - last_tick_count) < MIN_TIME_BETWEEN_CALC))
    {
        if (m_CurSubPat != NULL)
        {
            m_CurSubPat->DrawPositions(pInfo);
        }
		return;
    }

    last_tick_count = tick_count;

    switch (current_step)
    {
    case -1:    // Automatic calibration finished 
        Stop();
        break;

    case 0:     // Manual calibration
        m_CurSubPat = GetSubPattern(ADJ_MANUAL);
        if (m_CurSubPat == NULL)
        {
            break;
        }

        new_settings = FALSE;
        new_settings |= brightness->Update();
        new_settings |= contrast->Update();
        new_settings |= saturation_U->Update();
        new_settings |= saturation_V->Update();
        new_settings |= hue->Update();

        // Calculations with current setitngs
        if ( m_CurSubPat->CalcCurrentSubPattern(first_calc || new_settings, NB_CALCULATIONS_LOW, pInfo)
          && (m_TypeCalibration != CAL_MANUAL) )
        {
            current_step = -1;
            last_tick_count = tick_count + 500;
        }
        first_calc = FALSE;
        break;

    case 1:
        brightness->SetRange((nb_tries == 0) ? 75 : 25);
        if (step_init(ADJ_BRIGHTNESS, brightness, (CCalSetting* )NULL, (CCalSetting* )NULL))
        {
            LOG(2, "Automatic Calibration - brightness - reduced range - try %d", nb_tries+1);
            current_step++;
        }
        else
        {
            // We stop calibration
            current_step = initial_step + nb_steps;
        }
        break;

    case 2:     // Step to find a short range for brightness setting
        if (step_process(pInfo, 1, NB_CALCULATIONS_LOW, TRUE, FALSE, &found))
        {
            current_step += (found ? 3 : 1);
        }
        break;

    case 3:
        brightness->SetFullRange();
        if (step_init(ADJ_BRIGHTNESS, brightness, (CCalSetting* )NULL, (CCalSetting* )NULL))
        {
            LOG(2, "Automatic Calibration - brightness - full range - try %d", nb_tries+1);
            current_step++;
        }
        else
        {
            // We stop calibration
            current_step = initial_step + nb_steps;
        }
        break;

    case 4:     // Step to find a short range for brightness setting
        if (step_process(pInfo, 1, NB_CALCULATIONS_LOW, TRUE, FALSE, &found))
        {
            current_step++;
        }
        break;

    case 5:
        contrast->SetRange((nb_tries == 0) ? 50 : 25);
        if (step_init(ADJ_CONTRAST, contrast, (CCalSetting* )NULL, (CCalSetting* )NULL))
        {
            LOG(2, "Automatic Calibration - contrast - reduced range - try %d", nb_tries+1);
            current_step++;
        }
        else
        {
            // We stop calibration
            current_step = initial_step + nb_steps;
        }
        break;

    case 6:     // Step to find a short range for contrast setting
        if (step_process(pInfo, 1, NB_CALCULATIONS_LOW, TRUE, FALSE, &found))
        {
            current_step += (found ? 3 : 1);
        }
        break;

    case 7:
        contrast->SetFullRange();
        if (step_init(ADJ_CONTRAST, contrast, (CCalSetting* )NULL, (CCalSetting* )NULL))
        {
            LOG(2, "Automatic Calibration - contrast - full range - try %d", nb_tries+1);
            current_step++;
        }
        else
        {
            // We stop calibration
            current_step = initial_step + nb_steps;
        }
        break;

    case 8:     // Step to find a short range for contrast setting
        if (step_process(pInfo, 1, NB_CALCULATIONS_LOW, TRUE, FALSE, &found))
        {
            current_step++;
        }
        break;

    case 9:
        nb_tries++;
        if (nb_tries < 2)
        {
            current_step -= 8;
        }
        else
        {
            nb_tries = 0;
            current_step++;
        }
        break;

    case 10:
        if (brightness->GetResult(mask, &min, &max) > 0)
        {
            brightness->SetRange(mask);
        }
        else
        {
            brightness->SetRange(0);
        }
        nb1 = brightness->GetRange(mask, &min, &max);
        if (contrast->GetResult(mask, &min, &max) > 0)
        {
            contrast->SetRange(mask);
        }
        else
        {
            contrast->SetRange(0);
        }
        nb2 = contrast->GetRange(mask, &min, &max);
        if ((nb1 == 1) && (nb2 == 1))
        {
            current_step += 2;
            break;
        }
        if (step_init(ADJ_BRIGHTNESS_CONTRAST, brightness, contrast, (CCalSetting* )NULL))
        {
            LOG(2, "Automatic Calibration - brightness + contrast - %d %d", nb1, nb2);
            current_step++;
        }
        else
        {
            current_step += 2;
        }
        break;

    case 11:     // Step to adjust fine brightness + contradt
        if (step_process(pInfo, 1, NB_CALCULATIONS_HIGH, FALSE, TRUE, &found))
        {
            current_step++;
        }
        break;

    case 12:
        saturation_U->SetRange(75);
        if (step_init(ADJ_SATURATION_U, saturation_U, (CCalSetting* )NULL, (CCalSetting* )NULL))
        {
            LOG(2, "Automatic Calibration - saturation U - reduced range - try %d", nb_tries+1);
            current_step++;
        }
        else
        {
            // We stop calibration
            current_step = initial_step + nb_steps;
        }
        break;

    case 13:     // Step to find a short range for saturation U setting
        if (step_process(pInfo, 2, NB_CALCULATIONS_LOW, TRUE, FALSE, &found))
        {
            current_step += (found ? 3 : 1);
        }
        break;

    case 14:
        saturation_U->SetFullRange();
        if (step_init(ADJ_SATURATION_U, saturation_U, (CCalSetting* )NULL, (CCalSetting* )NULL))
        {
            LOG(2, "Automatic Calibration - saturation U - full range - try %d", nb_tries+1);
            current_step++;
        }
        else
        {
            // We stop calibration
            current_step = initial_step + nb_steps;
        }
        break;

    case 15:     // Step to find a short range for saturation U setting
        if (step_process(pInfo, 2, NB_CALCULATIONS_LOW, TRUE, FALSE, &found))
        {
            current_step++;
        }
        break;

    case 16:
        saturation_V->SetRange(75);
        if (step_init(ADJ_SATURATION_V, saturation_V, (CCalSetting* )NULL, (CCalSetting* )NULL))
        {
            LOG(2, "Automatic Calibration - saturation V - reduced range - try %d", nb_tries+1);
            current_step++;
        }
        else
        {
            // We stop calibration
            current_step = initial_step + nb_steps;
        }
        break;

    case 17:    // Step to find a short range for saturation V setting
        if (step_process(pInfo, 3, NB_CALCULATIONS_LOW, TRUE, FALSE, &found))
        {
            current_step += (found ? 3 : 1);
        }
        break;

    case 18:
        saturation_V->SetFullRange();
        if (step_init(ADJ_SATURATION_V, saturation_V, (CCalSetting* )NULL, (CCalSetting* )NULL))
        {
            LOG(2, "Automatic Calibration - saturation V - full range - try %d", nb_tries+1);
            current_step++;
        }
        else
        {
            // We stop calibration
            current_step = initial_step + nb_steps;
        }
        break;

    case 19:    // Step to find a short range for saturation V setting
        if (step_process(pInfo, 3, NB_CALCULATIONS_LOW, TRUE, FALSE, &found))
        {
            current_step++;
        }
        break;

    case 20:
        hue->SetRange(30);
        if (step_init(ADJ_HUE, hue, (CCalSetting* )NULL, (CCalSetting* )NULL))
        {
            LOG(2, "Automatic Calibration - hue - reduced range - try %d", nb_tries+1);
            current_step++;
        }
        else
        {
            // We stop calibration
            current_step = initial_step + nb_steps;
        }
        break;

    case 21:    // Step to find a short range for hue setting
        if (step_process(pInfo, 2, NB_CALCULATIONS_LOW, TRUE, FALSE, &found))
        {
            current_step++;
        }
        break;

    case 22:
        if (saturation_U->GetResult(mask, &min, &max) > 0)
        {
            saturation_U->SetRange(mask);
        }
        else
        {
            saturation_U->SetRange(0);
        }
        nb1 = saturation_U->GetRange(mask, &min, &max);
        if (saturation_V->GetResult(mask, &min, &max) > 0)
        {
            saturation_V->SetRange(mask);
        }
        else
        {
            saturation_V->SetRange(0);
        }
        nb2 = saturation_V->GetRange(mask, &min, &max);
        if (hue->GetResult(mask, &min, &max) > 0)
        {
            hue->SetRange(mask);
        }
        else
        {
            hue->SetRange(0);
        }
        nb3 = hue->GetRange(mask, &min, &max);
        if ((nb1 == 1) && (nb2 == 1) && (nb3 == 1))
        {
            current_step += 2;
            break;
        }
        if (step_init(ADJ_COLOR, saturation_U, saturation_V, hue))
        {
            LOG(2, "Automatic Calibration - saturation U + saturation V + hue - %d %d %d", nb1, nb2, nb3);
            current_step++;
        }
        else
        {
            current_step += 2;
        }
        break;

    case 23:    // Step to adjust fine color saturation and hue
        if (step_process(pInfo, 4, NB_CALCULATIONS_HIGH, FALSE, TRUE, &found))
        {
            current_step++;
        }
        break;

    default:
        break;
    }

    // Test to check if all steps are already done
    if ((current_step > 0) && ((current_step - initial_step) >= nb_steps))
    {
        current_step = 0;
        first_calc = TRUE;
    }

    if (m_CurSubPat != NULL)
    {
        m_CurSubPat->DrawPositions(pInfo);
    }
}

BOOL CCalibration::step_init(eTypeAdjust type_adjust, CCalSetting* _setting1, CCalSetting* _setting2, CCalSetting* _setting3)
{
    // Get the bar to use for this step
    m_CurSubPat = GetSubPattern(type_adjust);
    if (m_CurSubPat == NULL)
    {
        setting1 = (CCalSetting* )NULL;
        setting2 = (CCalSetting* )NULL;
        setting3 = (CCalSetting* )NULL;
        return FALSE;
    }
    else
    {
        // Initialize
        setting1 = _setting1;
        if (setting1 != (CCalSetting* )NULL)
        {
            // Set the settings to their minimum
            setting1->AdjustMin();
            setting1->InitResult();
        }
        setting2 = _setting2;
        if (setting2 != (CCalSetting* )NULL)
        {
            // Set the settings to their minimum
            setting2->AdjustMin();
            setting2->InitResult();
        }
        setting3 = _setting3;
        if (setting3 != (CCalSetting* )NULL)
        {
            // Set the settings to their minimum
            setting3->AdjustMin();
            setting3->InitResult();
        }

//        first_calc = TRUE;
        nb_calcul = 0;
        total_dif = 0;

        return TRUE;
    }
}

BOOL CCalibration::step_process(TDeinterlaceInfo* pInfo, unsigned int sig_component, unsigned int nb_calc, BOOL stop_when_found, BOOL only_one, BOOL* best_found)
{
    int val[4];
    BOOL YUV;
    int idx;
//    int dif;

    // Calculations with current settings
    m_CurSubPat->CalcCurrentSubPattern(TRUE, 1, pInfo);

    // See how good is the red result
    if ((sig_component >= 1) && (sig_component <= 4))
    {
        YUV = TRUE;
        idx = sig_component - 1;
    }
    else if ((sig_component >= 5) && (sig_component <= 8))
    {
        YUV = FALSE;
        idx = sig_component - 5;
    }
    m_CurSubPat->GetSumDeltaColor(YUV, &val[0], &val[1], &val[2], &val[3]);
//    dif = val[idx];
    total_dif += val[idx];
    nb_calcul++;

    // Waiting at least 5 calculations
    if (nb_calcul < nb_calc)
    {
        last_tick_count = -1;
        return FALSE;
    }

    if (setting1 != (CCalSetting* )NULL)
    {
       * best_found = setting1->UpdateResult(total_dif, stop_when_found ? DELTA_STOP*nb_calc : -1, only_one);
//       * best_found = setting1->UpdateResult(dif, stop_when_found ? DELTA_STOP : -1, only_one);
    }
    if (setting2 != (CCalSetting* )NULL)
    {
       * best_found = setting2->UpdateResult(total_dif, stop_when_found ? DELTA_STOP*nb_calc : -1, only_one);
//       * best_found = setting2->UpdateResult(dif, stop_when_found ? DELTA_STOP : -1, only_one);
    }
    if (setting3 != (CCalSetting* )NULL)
    {
       * best_found = setting3->UpdateResult(total_dif, stop_when_found ? DELTA_STOP*nb_calc : -1, only_one);
//       * best_found = setting3->UpdateResult(dif, stop_when_found ? DELTA_STOP : -1, only_one);
    }

    nb_calcul = 0;
    total_dif = 0;

    // Increase the third setting
    if ((setting3 != (CCalSetting* )NULL) && setting3->AdjustNext())
    {
        return FALSE;
    }
    // Increase the second setting
    else if ((setting2 != (CCalSetting* )NULL) && setting2->AdjustNext())
    {
        if (setting3 != (CCalSetting* )NULL)
        {
            // Set the third setting to its minimum
            setting3->AdjustMin();
        }
        return FALSE;
    }
    // Increase the first setting
    else if ((setting1 != (CCalSetting* )NULL) && setting1->AdjustNext())
    {
        if (setting2 != (CCalSetting* )NULL)
        {
            // Set the second setting to its minimum
            setting2->AdjustMin();
        }
        if (setting3 != (CCalSetting* )NULL)
        {
            // Set the third setting to its minimum
            setting3->AdjustMin();
        }
        return FALSE;
    }
    else
    {
        // Set the settings to the best values found
        if (setting1 != (CCalSetting* )NULL)
        {
            setting1->AdjustBest();
        }
        if (setting2 != (CCalSetting* )NULL)
        {
            setting2->AdjustBest();
        }
        if (setting3 != (CCalSetting* )NULL)
        {
            setting3->AdjustBest();
        }
        return TRUE;
    }
}

/////////////////////////////////////////////////////////////////////////////

CPatternHelper::CPatternHelper(CStillSource* pParent) :
    CStillSourceHelper(pParent)
{
}

BOOL CPatternHelper::OpenMediaFile(LPCSTR FileName)
{
    m_pParent->m_IsPictureRead = FALSE;

    m_pParent->m_Height = 480;
    m_pParent->m_Width = 720;
    CurrentX = m_pParent->m_Width;
    CurrentY = m_pParent->m_Height;

    CTestPattern pattern(FileName);

    if ( (pattern.GetWidth() > DSCALER_MAX_WIDTH) || (pattern.GetHeight() > DSCALER_MAX_HEIGHT) )
    {
        return FALSE;
    }

    m_pParent->m_Height = pattern.GetHeight();
    m_pParent->m_Width = pattern.GetWidth();
    CurrentX = m_pParent->m_Width;
    CurrentY = m_pParent->m_Height;

    // Allocate memory buffer to store the YUYV values
    m_pParent->m_OriginalFrame.pData = (BYTE*)malloc(m_pParent->m_Width * 2 * m_pParent->m_Height * sizeof(BYTE));
    if (m_pParent->m_OriginalFrame.pData == NULL)
    {
        return FALSE;
    }

    pattern.Draw(m_pParent->m_OriginalFrame.pData);

    m_pParent->m_IsPictureRead = TRUE;

    return TRUE;
}

void CPatternHelper::SaveSnapshot(LPCSTR FilePath, int Height, int Width, BYTE* pOverlay, LONG OverlayPitch)
{
    return;
}

/////////////////////////////////////////////////////////////////////////////

CCalibration* pCalibration = NULL;

/////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
/////////////////////////////////////////////////////////////////////////////

BOOL Calibr_Overscan_OnChange(long Overscan)
{
    SourceOverscan = Overscan;
    AspectSettings.InitialOverscan = SourceOverscan;
    WorkoutOverlaySize(TRUE);
    return FALSE;
}

SETTING CalibrSettings[CALIBR_SETTING_LASTONE] =
{
    {
        "Overscan for calibration", SLIDER, 0, (long*)&SourceOverscan,
         0, 0, 150, 1, 1,
         NULL,
        "Calibration", "SourceOverscan", Calibr_Overscan_OnChange,
    },
    {
        "Left player cropping", SLIDER, 0, (long*)&LeftCropping,
         8, 0, 50, 1, 1,
         NULL,
        "Calibration", "LeftPlayerCropping", NULL,
    },
    {
        "Right player cropping", SLIDER, 0, (long*)&RightCropping,
         16, 0, 50, 1, 1,
         NULL,
        "Calibration", "RightPlayerCropping", NULL,
    },
    {
        "Show RGB delta in OSD", ONOFF, 0, (long*)&ShowRGBDelta,
         TRUE, 0, 1, 1, 1,
         NULL,
        "Calibration", "ShowRGBDelta", NULL,
    },
    {
        "Show YUV delta in OSD", ONOFF, 0, (long*)&ShowYUVDelta,
         TRUE, 0, 1, 1, 1,
         NULL,
        "Calibration", "ShowYUVDelta", NULL,
    },
};


SETTING* Calibr_GetSetting(CALIBR_SETTING Setting)
{
    if(Setting > -1 && Setting < CALIBR_SETTING_LASTONE)
    {
        return &(CalibrSettings[Setting]);
    }
    else
    {
        return NULL;
    }
}

void Calibr_ReadSettingsFromIni()
{
    int i;
    for(i = 0; i < CALIBR_SETTING_LASTONE; i++)
    {
        Setting_ReadFromIni(&(CalibrSettings[i]));
    }
}

void Calibr_WriteSettingsToIni(BOOL bOptimizeFileAccess)
{
    int i;
    for(i = 0; i < CALIBR_SETTING_LASTONE; i++)
    {
        Setting_WriteToIni(&(CalibrSettings[i]), bOptimizeFileAccess);
    }
}

