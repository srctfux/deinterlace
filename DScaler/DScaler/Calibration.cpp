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
// Change Log
//
// Date          Developer             Changes
//
// 23 Jul 2001   Laurent Garnier       File created
//
/////////////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "Calibration.h"
#include "resource.h"
#include "DScaler.h"
#include "Settings.h"
#include "OSD.h"
#include "DebugLog.h"


// Minimum time in milliseconds between two consecutive evaluations
#define	MIN_TIME_BETWEEN_CALC	100

// Macro to restrict range to [0,255]
#define LIMIT(x) (((x)<0)?0:((x)>255)?255:(x))


/////////////////////////////////////////////////////////////////////////////
// Class CColorBar

CColorBar::CColorBar(unsigned short int left, unsigned short int right, unsigned short int top, unsigned short int bottom, eColor color, BOOL YUV, unsigned char R_Y, unsigned char G_U, unsigned char B_V)
{ 
    unsigned char Y, U, V, R, G, B;
    left_border = left; 
    right_border = right; 
    top_border = top; 
    bottom_border = bottom;
    abstract_color = color;
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

    // TEMPORARY tests for YUV <=> RGB
    Y = 100; U = 50; V = 75;
    LOG(5, "Y = %d U = %d V = %d", Y, U, V);
    YUV2RGB(Y, U, V, &R, &G, &B);
    LOG(5, "=> YUV2RGB R = %d G = %d B = %d", R, G, B);
    RGB2YUV(R, G, B, &Y, &U, &V);
    LOG(5, "=> YUV2RGB => RGB2YUV Y = %d U = %d V = %d", Y, U, V);
    Y = 50; U = 150; V = 100;
    LOG(5, "Y = %d U = %d V = %d", Y, U, V);
    YUV2RGB(Y, U, V, &R, &G, &B);
    LOG(5, "=> YUV2RGB R = %d G = %d B = %d", R, G, B);
    RGB2YUV(R, G, B, &Y, &U, &V);
    LOG(5, "=> YUV2RGB => RGB2YUV Y = %d U = %d V = %d", Y, U, V);
    Y = 200; U = 225; V = 175;
    LOG(5, "Y = %d U = %d V = %d", Y, U, V);
    YUV2RGB(Y, U, V, &R, &G, &B);
    LOG(5, "=> YUV2RGB R = %d G = %d B = %d", R, G, B);
    RGB2YUV(R, G, B, &Y, &U, &V);
    LOG(5, "=> YUV2RGB => RGB2YUV Y = %d U = %d V = %d", Y, U, V);
}

// This method returns the abstract color
eColor CColorBar::GetAbstractColor()
{
    return abstract_color;
}

// This methode returns the reference color
// If parameter YUV is TRUE, it returns YUV values else RGB values
void CColorBar::GetRefColor(BOOL YUV, unsigned char *pR_Y, unsigned char *pG_U, unsigned char *pB_V)
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
void CColorBar::GetCurrentAvgColor(BOOL YUV, unsigned char *pR_Y, unsigned char *pG_U, unsigned char *pB_V)
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
void CColorBar::GetDeltaColor(BOOL YUV, int *pR_Y, int *pG_U, int *pB_V, int *pTotal)
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
    if (*pR_Y < 0)
    {
        *pTotal -= *pR_Y;
    }
    else
    {
        *pTotal += *pR_Y;
    }
    if (*pG_U < 0)
    {
        *pTotal -= *pG_U;
    }
    else
    {
        *pTotal += *pG_U;
    }
    if (*pB_V < 0)
    {
        *pTotal -= *pB_V;
    }
    else
    {
        *pTotal += *pB_V;
    }
}

// This method analyzed the overlay buffer to calculate average color
// in the zone defined by the color bar
void CColorBar::CalcAvgColor(short **Lines, int height, int width)
{ 
    int left, right, top, bottom, i, j;
    unsigned int Y, U, V, nb_Y, nb_U, nb_V;
    BYTE *buf;

    // Calculate the exact coordinates of rectangular zone in the buffer
    left = width * left_border / 10000;
    right = width * right_border / 10000;
    top = height * top_border / 10000;
    bottom = height * bottom_border / 10000;

    // Sum separately Y, U and V in this rectangular zone
    // Each line is like this : YUYVYUYV...
    Y = 0; nb_Y = 0;
    U = 0; nb_U = 0;
    V = 0; nb_V = 0;
    for (i = top ; i <= bottom ; i++)
    {
        for (j = left ; j <= right ; j++)
        {
            buf = (BYTE *)Lines[i];
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
    if (nb_Y > 0)
    {
		Y_val = (Y + (nb_Y / 2)) / nb_Y;
    }
    else
    {
		Y_val = 0;
    }
    if (nb_U > 0)
    {
		U_val = (U + (nb_U / 2)) / nb_U;
    }
    else
    {
		U_val = 0;
    }
    if (nb_V > 0)
    {
		V_val = (V + (nb_V / 2)) / nb_V;
    }
    else
    {
		V_val = 0;
    }

    LOG(5, "CalcAvgColor YUV %d %d %d %d %d %d %d %d %d", Y_val, U_val, V_val, left, right, top, bottom, height, width);

    // Save corresponding RGB values too
    YUV2RGB(Y_val, U_val, V_val, &R_val, &G_val, &B_val);

    LOG(5, "CalcAvgColor RGB %d %d %d %d %d %d %d %d %d", R_val, G_val, B_val, left, right, top, bottom, height, width);
}

// Convert RGB to YUV
void CColorBar::RGB2YUV(unsigned char R, unsigned char G, unsigned char B, unsigned char *pY, unsigned char *pU, unsigned char *pV)
{
    unsigned int y, cr, cb;

//    y  = ( 50396*R + 33058*G +  6405*B + 1048576)>>16;
    y  = ( 16840*R + 33058*G +  6405*B + 1048576)>>16;
    cr = ( 28781*R - 24110*G -  4671*B + 8388608)>>16;
    cb = ( -9713*R - 19068*G + 28781*B + 8388608)>>16;

//    LOG(5, "RGB2YUV %d %d %d", y, cb, cr);

    *pY = LIMIT(y);
    *pU = LIMIT(cb);
    *pV = LIMIT(cr);
}

// Convert YUV to RGB
void CColorBar::YUV2RGB(unsigned char Y, unsigned char U, unsigned char V, unsigned char *pR, unsigned char *pG, unsigned char *pB)
{
    int y, cr, cb, r, g, b;

    cb = U - 128;
    cr = V - 128;
    y = Y - 16;

    r = ( 76284*y + 104595*cr             )>>16;
    g = ( 76284*y -  53281*cr -  25624*cb )>>16;
    b = ( 76284*y             + 132252*cb )>>16;

    *pR = LIMIT(r);
    *pG = LIMIT(g);
    *pB = LIMIT(b);
}


/////////////////////////////////////////////////////////////////////////////
// Class CTestPattern

CTestPattern::CTestPattern(char* name, eTypeTestPattern type, eVideoFormat format)
{
    strcpy(pattern_name, name);
    type_content = type;
    video_format = format;
    nb_color_bars = 0;
    for (int i = 0 ; i < MAX_COLOR_BARS ; i++)
    {
        color_bars[i] = NULL;
    }
    idx_color_bar = -1;
}

CTestPattern::~CTestPattern()
{
    for (int i = 0 ; i < MAX_COLOR_BARS ; i++)
    {
        if (color_bars[i] != NULL)
        {
            delete color_bars[i];
        }
    }
}

// This method returns the name of the test pattern
char *CTestPattern::GetName()
{
    return pattern_name;
}

// This method returns the type of content for in the test pattern
eTypeTestPattern CTestPattern::GetType()
{
    return type_content;
}

// This method returns the video format of the test pattern
eVideoFormat CTestPattern::GetVideoFormat()
{
    return video_format;
}

// This method allows to add a new color bar to the test pattern
// Returns 0 if color bar is correctly added
int CTestPattern::AddColorBar(unsigned short int left, unsigned short int right, unsigned short int top, unsigned short int bottom, eColor color, BOOL YUV, unsigned char R_Y, unsigned char G_U, unsigned char B_V)
{
    int i;

    for (i = 0 ; i < MAX_COLOR_BARS ; i++)
    {
        if (color_bars[i] == NULL)
        {
            break;
        }
    }
    if (i < MAX_COLOR_BARS)
    {
        color_bars[i] = new CColorBar(left, right, top, bottom, color, YUV, R_Y, G_U, B_V);
        nb_color_bars++;
        return 0;
    }
    else
    {
        return -1;
    }
}

// This method returns the first color bar of the test pattern
// The first is the first added
// Returns NULL pointer if no color bar are defined for the test pattern
CColorBar *CTestPattern::GetFirstColorBar()
{
    for (idx_color_bar = 0 ; idx_color_bar < MAX_COLOR_BARS ; idx_color_bar++)
    {
        if (color_bars[idx_color_bar] != NULL)
        {
            return color_bars[idx_color_bar];
        }
    }
    idx_color_bar = -1;
    return NULL;
}

// This method returns the next color bar of the test pattern
// GetFirstColorBar must be called at least one time before calling GetNextColorBar
// Returns NULL pointer if we were already on the last color bar
CColorBar *CTestPattern::GetNextColorBar()
{
    for (idx_color_bar++; idx_color_bar < MAX_COLOR_BARS ; idx_color_bar++)
    {
        if (color_bars[idx_color_bar] != NULL)
        {
            return color_bars[idx_color_bar];
        }
    }
    idx_color_bar = -1;
    return NULL;
}

// This method returns the (first) bar having the color searched
// Returns NULL pointer if no bar in the test pattern has correct color
CColorBar *CTestPattern::GetColorBar(eColor color)
{
    CColorBar *bar;

    bar = GetFirstColorBar();
    while (bar != NULL)
    {
        if (bar->GetAbstractColor() == color)
        {
            break;
        }
        bar = GetNextColorBar();
    }
    return bar;
}

// This method analyzes the current overlay buffer
void CTestPattern::CalcCurrentPattern(short **Lines, int height, int width)
{
    // Do the job for each defined color bar
    for (int i = 0 ; i < MAX_COLOR_BARS ; i++)
    {
        if (color_bars[i] != NULL)
        {
            color_bars[i]->CalcAvgColor(Lines, height, width);
        }
    }
}

// This methode returns the sum of delta between reference color
// and calculated average color through all the color bars
void CTestPattern::GetSumDeltaColor(BOOL YUV, int *pR_Y, int *pG_U, int *pB_V, int *pTotal)
{
    int i, j;
    int delta[4];
    int sum_delta[4];

    // Set the sums to 0
    for (j = 0 ; j <= 3 ; j++)
    {
        sum_delta[j] = 0;
    }
    // Go through all the color bars
    for (i = 0 ; i < MAX_COLOR_BARS ; i++)
    {
        if (color_bars[i] != NULL)
        {
            color_bars[i]->GetDeltaColor(YUV, &delta[0], &delta[1], &delta[2], &delta[3]);
            for (j = 0 ; j <= 3 ; j++)
            {
                sum_delta[j] += delta[j];
            }
        }
    }

    *pR_Y = sum_delta[0];
    *pG_U = sum_delta[1];
    *pB_V = sum_delta[2];
    *pTotal = sum_delta[3];
}
	
/////////////////////////////////////////////////////////////////////////////
// Class CCalibration

CCalibration::CCalibration()
{
    nb_test_patterns = 0;
    for (int i = 0 ; i < MAX_TEST_PATTERNS ; i++)
    {
        test_patterns[i] = NULL;
    }
    current_test_pattern = NULL;
    type_calibration = CAL_MANUAL;
    running = FALSE;
    last_tick_count = -1;
    LoadTestPatterns();
}

CCalibration::~CCalibration()
{
    UnloadTestPatterns();
}

// This method loads all the predefined test patterns
void CCalibration::LoadTestPatterns()
{
    // Create all the test patterns

//    test_patterns[nb_test_patterns] = new CTestPattern("THX Optimode - Monitor Performance", PAT_GRAY_AND_COLOR, FORMAT_NTSC);
    test_patterns[nb_test_patterns] = new CTestPattern("THX Optimode - Monitor Performance", PAT_RANGE_OF_GRAY, FORMAT_NTSC);
//    test_patterns[nb_test_patterns]->AddColorBar( 278, 1042, 2396, 4167, COLOR_OTHER, FALSE, 251, 252, 251);
//    test_patterns[nb_test_patterns]->AddColorBar(1458, 2222, 2396, 4167, COLOR_OTHER, FALSE, 188, 190,   0);
//    test_patterns[nb_test_patterns]->AddColorBar(2708, 3472, 2396, 4167, COLOR_OTHER, FALSE,   0, 188, 185);
//    test_patterns[nb_test_patterns]->AddColorBar(3889, 4653, 2396, 4167, COLOR_OTHER, FALSE,   0, 188,   0);
//    test_patterns[nb_test_patterns]->AddColorBar(5139, 5903, 2396, 4167, COLOR_OTHER, FALSE, 187,   0, 187);
//    test_patterns[nb_test_patterns]->AddColorBar(6319, 7083, 2396, 4167, COLOR_RED,   FALSE, 186,   0,   0);
//    test_patterns[nb_test_patterns]->AddColorBar(7569, 8333, 2396, 4167, COLOR_BLUE,  FALSE,  0,    0, 187);
//    test_patterns[nb_test_patterns]->AddColorBar(8750, 9514, 2396, 4167, COLOR_OTHER, FALSE,  0,    0,   0);
    test_patterns[nb_test_patterns]->AddColorBar( 347,  764, 6875, 7500, COLOR_BLACK, FALSE,  0,    0,   0);
    test_patterns[nb_test_patterns]->AddColorBar(1181, 1597, 6875, 7500, COLOR_OTHER, FALSE,  23,  24,  23);
    test_patterns[nb_test_patterns]->AddColorBar(2014, 2431, 6875, 7500, COLOR_OTHER, FALSE,  49,  50,  49);
    test_patterns[nb_test_patterns]->AddColorBar(7083, 7500, 6875, 7500, COLOR_OTHER, FALSE, 201, 203, 202);
    test_patterns[nb_test_patterns]->AddColorBar(7917, 8333, 6875, 7500, COLOR_OTHER, FALSE, 227, 228, 227);
    test_patterns[nb_test_patterns]->AddColorBar(8750, 9167, 6875, 7500, COLOR_WHITE, FALSE, 252, 253, 252);
    nb_test_patterns++;

    test_patterns[nb_test_patterns] = new CTestPattern("THX Optimode - Tint Color", PAT_COLOR, FORMAT_NTSC);
    test_patterns[nb_test_patterns]->AddColorBar( 417, 1111, 521, 5729, COLOR_OTHER, FALSE, 188, 189, 188);
    test_patterns[nb_test_patterns]->AddColorBar(1806, 2500, 521, 5729, COLOR_OTHER, FALSE, 189, 190,   0);
    test_patterns[nb_test_patterns]->AddColorBar(3194, 3889, 521, 5729, COLOR_OTHER, FALSE,   0, 190, 188);
    test_patterns[nb_test_patterns]->AddColorBar(4583, 5278, 521, 5729, COLOR_OTHER, FALSE,   0, 189,   0);
    test_patterns[nb_test_patterns]->AddColorBar(6042, 6736, 521, 5729, COLOR_OTHER, FALSE, 188,   0, 190);
    test_patterns[nb_test_patterns]->AddColorBar(7431, 8125, 521, 5729, COLOR_RED,   FALSE, 189,   0,   0);
    test_patterns[nb_test_patterns]->AddColorBar(8889, 9583, 521, 5729, COLOR_BLUE,  FALSE,   0,   0, 189);
    nb_test_patterns++;

    test_patterns[nb_test_patterns] = new CTestPattern("VE - T 18 C 2 - range of gray", PAT_RANGE_OF_GRAY, FORMAT_NTSC);
    test_patterns[nb_test_patterns]->AddColorBar( 417,  972, 521, 9375, COLOR_BLACK, FALSE,   0,   0,   0);
    test_patterns[nb_test_patterns]->AddColorBar(1458, 2014, 521, 9375, COLOR_OTHER, FALSE,  30,  30,  30);
    test_patterns[nb_test_patterns]->AddColorBar(2361, 2917, 521, 9375, COLOR_OTHER, FALSE,  60,  60,  60);
    test_patterns[nb_test_patterns]->AddColorBar(3333, 3889, 521, 9375, COLOR_OTHER, FALSE,  90,  90,  90);
    test_patterns[nb_test_patterns]->AddColorBar(4236, 4792, 521, 9375, COLOR_OTHER, FALSE, 116, 118, 118);
    test_patterns[nb_test_patterns]->AddColorBar(5208, 5764, 521, 9375, COLOR_OTHER, FALSE, 150, 150, 150);
    test_patterns[nb_test_patterns]->AddColorBar(6181, 6736, 521, 9375, COLOR_OTHER, FALSE, 177, 179, 179);
    test_patterns[nb_test_patterns]->AddColorBar(7153, 7639, 521, 9375, COLOR_OTHER, FALSE, 206, 209, 205);
    test_patterns[nb_test_patterns]->AddColorBar(8125, 8681, 521, 9375, COLOR_OTHER, FALSE, 236, 236, 236);
    test_patterns[nb_test_patterns]->AddColorBar(9097, 9653, 521, 9375, COLOR_WHITE, FALSE, 254, 254, 254);
    nb_test_patterns++;

    test_patterns[nb_test_patterns] = new CTestPattern("VE - T 18 C 10 - color bars", PAT_COLOR, FORMAT_NTSC);
    test_patterns[nb_test_patterns]->AddColorBar( 417, 1111, 521, 5208, COLOR_OTHER, FALSE, 190, 190, 190);
    test_patterns[nb_test_patterns]->AddColorBar(1806, 2500, 521, 5208, COLOR_OTHER, FALSE, 198, 199,   0);
    test_patterns[nb_test_patterns]->AddColorBar(3194, 3889, 521, 5208, COLOR_OTHER, FALSE,   0, 190, 189);
    test_patterns[nb_test_patterns]->AddColorBar(4583, 5278, 521, 5208, COLOR_OTHER, FALSE,   0, 190,   0);
    test_patterns[nb_test_patterns]->AddColorBar(6042, 6736, 521, 5208, COLOR_OTHER, FALSE, 190,   0, 189);
    test_patterns[nb_test_patterns]->AddColorBar(7431, 8125, 521, 5208, COLOR_RED,   FALSE, 188,   0,   0);
    test_patterns[nb_test_patterns]->AddColorBar(8889, 9583, 521, 5208, COLOR_BLUE,  FALSE,   0,   0, 187);
    nb_test_patterns++;

    test_patterns[nb_test_patterns] = new CTestPattern("AVIA - T 1 C 7 - range of gray", PAT_RANGE_OF_GRAY, FORMAT_NTSC);
    test_patterns[nb_test_patterns]->AddColorBar( 208,  764, 521, 9375, COLOR_WHITE, FALSE, 254, 251, 255);
    test_patterns[nb_test_patterns]->AddColorBar(1111, 1667, 521, 9375, COLOR_OTHER, FALSE, 226, 223, 227);
    test_patterns[nb_test_patterns]->AddColorBar(2014, 2569, 521, 9375, COLOR_OTHER, FALSE, 200, 197, 201);
    test_patterns[nb_test_patterns]->AddColorBar(2917, 3472, 521, 9375, COLOR_OTHER, FALSE, 172, 169, 173);
    test_patterns[nb_test_patterns]->AddColorBar(3819, 4375, 521, 9375, COLOR_OTHER, FALSE, 145, 142, 146);
    test_patterns[nb_test_patterns]->AddColorBar(4722, 5278, 521, 9375, COLOR_OTHER, FALSE, 117, 114, 118);
    test_patterns[nb_test_patterns]->AddColorBar(5625, 6181, 521, 9375, COLOR_OTHER, FALSE,  90,  87,  91);
    test_patterns[nb_test_patterns]->AddColorBar(6528, 7083, 521, 9375, COLOR_OTHER, FALSE,  62,  59,  63);
    test_patterns[nb_test_patterns]->AddColorBar(7431, 7986, 521, 9375, COLOR_OTHER, FALSE,  34,  31,  35);
    test_patterns[nb_test_patterns]->AddColorBar(8333, 8819, 521, 9375, COLOR_OTHER, FALSE,   7,   4,   8);
    test_patterns[nb_test_patterns]->AddColorBar(9236, 9722, 521, 9375, COLOR_BLACK, FALSE,   1,   0,   2);
    nb_test_patterns++;

    test_patterns[nb_test_patterns] = new CTestPattern("AVIA - T 4 C 4 - color bars", PAT_COLOR, FORMAT_NTSC);
    test_patterns[nb_test_patterns]->AddColorBar( 417, 1111, 521, 4167, COLOR_OTHER, FALSE, 190, 189, 190);
    test_patterns[nb_test_patterns]->AddColorBar(1806, 2500, 521, 4167, COLOR_OTHER, FALSE, 190, 189,   0);
    test_patterns[nb_test_patterns]->AddColorBar(3194, 3889, 521, 4167, COLOR_OTHER, FALSE,   0, 189, 188);
    test_patterns[nb_test_patterns]->AddColorBar(4583, 5278, 521, 4167, COLOR_OTHER, FALSE,   0, 189,   0);
    test_patterns[nb_test_patterns]->AddColorBar(6042, 6736, 521, 4167, COLOR_OTHER, FALSE, 190,   0, 191);
    test_patterns[nb_test_patterns]->AddColorBar(7431, 8125, 521, 4167, COLOR_RED,   FALSE, 190,   0,   0);
    test_patterns[nb_test_patterns]->AddColorBar(8889, 9583, 521, 4167, COLOR_BLUE,  FALSE,   0,   0, 190);
    nb_test_patterns++;

    test_patterns[nb_test_patterns] = new CTestPattern("LAL - T 36 C 1 - range of gray", PAT_RANGE_OF_GRAY, FORMAT_PAL_BDGHI);
    test_patterns[nb_test_patterns]->AddColorBar( 278,  833, 434, 9549, COLOR_BLACK, FALSE,   0,   0,   0);
    test_patterns[nb_test_patterns]->AddColorBar(1181, 1736, 434, 9549, COLOR_OTHER, FALSE,  20,  23,  20);
    test_patterns[nb_test_patterns]->AddColorBar(2014, 2569, 434, 9549, COLOR_OTHER, FALSE,  46,  49,  45);
    test_patterns[nb_test_patterns]->AddColorBar(2917, 3472, 434, 9549, COLOR_OTHER, FALSE,  72,  75,  71);
    test_patterns[nb_test_patterns]->AddColorBar(3819, 4375, 434, 9549, COLOR_OTHER, FALSE,  98, 101,  97);
    test_patterns[nb_test_patterns]->AddColorBar(4722, 5278, 434, 9549, COLOR_OTHER, FALSE, 123, 125, 122);
    test_patterns[nb_test_patterns]->AddColorBar(5556, 6111, 434, 9549, COLOR_OTHER, FALSE, 149, 151, 148);
    test_patterns[nb_test_patterns]->AddColorBar(6458, 7014, 434, 9549, COLOR_OTHER, FALSE, 174, 177, 173);
    test_patterns[nb_test_patterns]->AddColorBar(7361, 7917, 434, 9549, COLOR_OTHER, FALSE, 201, 203, 200);
    test_patterns[nb_test_patterns]->AddColorBar(8264, 8819, 434, 9549, COLOR_OTHER, FALSE, 227, 229, 226);
    test_patterns[nb_test_patterns]->AddColorBar(9167, 9722, 434, 9549, COLOR_WHITE, FALSE, 252, 252, 252);
    nb_test_patterns++;

    test_patterns[nb_test_patterns] = new CTestPattern("LAL - T 31 C 1 - color bars", PAT_COLOR, FORMAT_PAL_BDGHI);
    test_patterns[nb_test_patterns]->AddColorBar( 556, 1111, 434, 9549, COLOR_WHITE, FALSE, 253, 253, 253);
    test_patterns[nb_test_patterns]->AddColorBar(1736, 2292, 434, 9549, COLOR_OTHER, FALSE, 189, 191,   0);
    test_patterns[nb_test_patterns]->AddColorBar(2917, 3472, 434, 9549, COLOR_OTHER, FALSE,   0, 189, 188);
    test_patterns[nb_test_patterns]->AddColorBar(4097, 4653, 434, 9549, COLOR_OTHER, FALSE,   0, 190,   0);
    test_patterns[nb_test_patterns]->AddColorBar(5278, 5833, 434, 9549, COLOR_OTHER, FALSE, 189,   0, 189);
    test_patterns[nb_test_patterns]->AddColorBar(6458, 7014, 434, 9549, COLOR_RED,   FALSE, 189,   0,   0);
    test_patterns[nb_test_patterns]->AddColorBar(7708, 8264, 434, 9549, COLOR_BLUE,  FALSE,   0,   0, 189);
    test_patterns[nb_test_patterns]->AddColorBar(8889, 9444, 434, 9549, COLOR_BLACK, FALSE,   0,   0,   0);
    nb_test_patterns++;
}

// This method unloads all the predefined test patterns
void CCalibration::UnloadTestPatterns()
{
    // Destroy all test patterns
    for (int i = 0 ; i < MAX_TEST_PATTERNS ; i++)
    {
        if (test_patterns[i] != NULL)
        {
            delete test_patterns[i];
            test_patterns[i] = NULL;
        }
    }
    nb_test_patterns = 0;
}

void CCalibration::UpdateMenu(HMENU hMenu)
{
    HMENU           hMenuPatterns;
    MENUITEMINFO    MenuItemInfo;
    int             i;
	char			*name;

    hMenuPatterns = GetPatternsSubmenu();
    if (hMenuPatterns == NULL) return;

    i = GetMenuItemCount(hMenuPatterns);
    while (i)
    {
        i--;
        RemoveMenu(hMenuPatterns, i, MF_BYPOSITION);
    }

	for (i=0; i < MAX_TEST_PATTERNS ; i++)
	{
		if (test_patterns[i] != NULL)
        {
			name = test_patterns[i]->GetName();
			if (strlen (name) > 0)
			{
	            MenuItemInfo.cbSize = sizeof (MenuItemInfo);
		        MenuItemInfo.fType = MFT_STRING;
			    MenuItemInfo.dwTypeData = name;
				MenuItemInfo.cch = strlen (name);

	            MenuItemInfo.fMask = MIIM_TYPE | MIIM_ID;
		        MenuItemInfo.wID = IDM_PATTERN_SELECT + i + 1;
			    InsertMenuItem(hMenuPatterns, i, TRUE, &MenuItemInfo);
			}
        }
    }
}

void CCalibration::SetMenu(HMENU hMenu)
{
    HMENU   hMenuPatterns;
    int     i;
	char	*name;

    if ((current_test_pattern != NULL) && (current_test_pattern->GetVideoFormat() != Setting_GetValue(BT848_GetSetting(TVFORMAT))))
    {
        current_test_pattern = NULL;
    }

    hMenuPatterns = GetPatternsSubmenu();
    if (hMenuPatterns == NULL) return;

    for (i=0; i < MAX_TEST_PATTERNS ; i++)
    {
		if (test_patterns[i] != NULL)
        {
			name = test_patterns[i]->GetName();
			if (strlen (name) > 0)
			{
				EnableMenuItem(hMenuPatterns, i, running ? MF_BYPOSITION | MF_GRAYED : MF_BYPOSITION | MF_ENABLED);
				EnableMenuItem(hMenuPatterns, i, (running || (test_patterns[i]->GetVideoFormat() != Setting_GetValue(BT848_GetSetting(TVFORMAT)))) ? MF_BYPOSITION | MF_GRAYED : MF_BYPOSITION | MF_ENABLED);
				CheckMenuItem(hMenuPatterns, i, (current_test_pattern == test_patterns[i]) ? MF_BYPOSITION | MF_CHECKED : MF_BYPOSITION | MF_UNCHECKED);
			}
        }
    }
	
	EnableMenuItem(hMenu, IDM_START_MANUAL_CALIBRATION, (running || (current_test_pattern == NULL)) ? MF_GRAYED : MF_ENABLED);
	EnableMenuItem(hMenu, IDM_START_AUTO_CALIBRATION, MF_GRAYED);
	EnableMenuItem(hMenu, IDM_START_AUTO_CALIBRATION2, (running || (current_test_pattern == NULL) || ((current_test_pattern->GetType() != PAT_GRAY_AND_COLOR) && (current_test_pattern->GetType() != PAT_RANGE_OF_GRAY))) ? MF_GRAYED : MF_ENABLED);
	EnableMenuItem(hMenu, IDM_START_AUTO_CALIBRATION3, (running || (current_test_pattern == NULL) || ((current_test_pattern->GetType() != PAT_GRAY_AND_COLOR) && (current_test_pattern->GetType() != PAT_COLOR))) ? MF_GRAYED : MF_ENABLED);
	EnableMenuItem(hMenu, IDM_STOP_CALIBRATION, (!running || (current_test_pattern == NULL)) ? MF_GRAYED : MF_ENABLED);
}

void CCalibration::SelectTestPattern(int num)
{
	if (test_patterns[num] != NULL)
	{
		current_test_pattern = test_patterns[num];
	}
	else
	{
		current_test_pattern = NULL;
	}
}

CTestPattern *CCalibration::GetCurrentTestPattern()
{
	return current_test_pattern;
}

void CCalibration::Start(eTypeCalibration type)
{
    if (current_test_pattern == NULL)
        return;

	type_calibration = type;
	running = TRUE;

    switch (type_calibration)
    {
    case CAL_AUTO_BRIGHT_CONTRAST:
        initial_step = 1;
        nb_steps = 6;
        break;
    case CAL_AUTO_COLOR:
        initial_step = 7;
        nb_steps = 6;
        break;
    case CAL_AUTO_FULL:
        initial_step = 1;
        nb_steps = 12;
        break;
    case CAL_MANUAL:
    default:
        initial_step = 0;
        nb_steps = 0;
        break;
    }
    current_step = initial_step;

    // Display the specific OSD screen
    OSD_ShowInfosScreen(hWnd, 4, 0);
}

void CCalibration::Stop()
{
	running = FALSE;

    // Erase the OSD screen
    OSD_Clear(hWnd);
}

BOOL CCalibration::IsRunning()
{
	return running;
}

BOOL CCalibration::GetCurrentStep()
{
	return current_step;
}

eTypeCalibration CCalibration::GetType()
{
	return type_calibration;
}

void CCalibration::Make(short **Lines, int height, int width, int tick_count)
{
	if (!running
	 || (current_test_pattern == NULL)
	 || ((last_tick_count != -1) && ((tick_count - last_tick_count) < MIN_TIME_BETWEEN_CALC)))
		return;

    // Test to check if all steps are already executed
    if ((current_step - initial_step) >= nb_steps)
    {
        current_step = 0;
    }

    switch (current_step)
    {
    case 0:     // Calibration finished
        // Calculations with current setitngs
        current_test_pattern->CalcCurrentPattern(Lines, height, width);
        break;

    case 1:     // Step to initialize next step
        if (step_init(COLOR_BLACK, BRIGHTNESS, -128, 127))
        {
            // We jump to next step
            current_step++;
        }
        else
        {
            // We stop calibration
            current_step = 0;
        }
        break;

    case 2:     // Step to find a short range for brightness setting
        if (step_process(Lines, height, width, BRIGHTNESS, 4))
        {
            // We jump to next step
            current_step++;
        }
        break;

    case 3:     // Step to initialize next step
        if (step_init(COLOR_WHITE, CONTRAST, 0, 511))
        {
            // We jump to next step
            current_step++;
        }
        else
        {
            // We stop calibration
            current_step = 0;
        }
        break;

    case 4:     // Step to find a short range for contrast setting
        if (step_process(Lines, height, width, CONTRAST, 4))
        {
            // We jump to next step
            current_step++;
        }
        break;

    case 5:     // Step to initialize next step
        step_init2(BRIGHTNESS, -128, 127, CONTRAST, 0, 511);

        // We jump to next step
        current_step++;
        break;

    case 6:     // Step to adjust fine brightness + contradt
        if (step_process2(Lines, height, width, BRIGHTNESS, CONTRAST, 4))
        {
            // We jump to next step
            current_step++;
        }
        break;

    case 7:     // Step to initialize next step
        if (step_init(COLOR_BLUE, SATURATIONU, 0, 511))
        {
            // We jump to next step
            current_step++;
        }
        else
        {
            // We stop calibration
            current_step = 0;
        }
        break;

    case 8:     // Step to find a short range for saturation U setting
        if (step_process(Lines, height, width, SATURATIONU, 7))
        {
            // We jump to next step
            current_step++;
        }
        break;

   case 9:     // Step to initialize next step
        if (step_init(COLOR_RED, SATURATIONV, 0, 511))
        {
            // We jump to next step
            current_step++;
        }
        else
        {
            // We stop calibration
            current_step = 0;
        }
        break;

   case 10:    // Step to find a short range for saturation V setting
        if (step_process(Lines, height, width, SATURATIONV, 5))
        {
            // We jump to next step
            current_step++;
        }
        break;

    case 11:    // Step to initialize next step
        step_init2(SATURATIONU, 0, 511, SATURATIONV, 0, 511);

        // We jump to next step
        current_step++;
        break;

    case 12:    // Step to adjust fine color saturation
        if (step_process2(Lines, height, width, SATURATIONU, SATURATIONV, 8))
        {
            // We jump to next step
            current_step++;
        }
        break;

    default:
        break;
    }

	last_tick_count = tick_count;
}

BOOL CCalibration::step_init(eColor color, BT848_SETTING setting, int min, int max)
{
    // Get the bar to use for this step
    bar_to_study = current_test_pattern->GetColorBar(color);
    if (bar_to_study == NULL)
    {
        return FALSE;
    }
    else
    {
        // Initialize
        min_val = min;
        max_val = max;
        current_val = min_val;
        best_val = min_val;
        best_val2 = max_val;
        min_dif = 100000;

        // Set the setting to its minimum
        Setting_SetValue(BT848_GetSetting(setting), current_val);

        return TRUE;
    }
}

BOOL CCalibration::step_process(short **Lines, int height, int width, BT848_SETTING setting, unsigned int sig_component)
{
    int val[4];
    BOOL YUV;
    int idx;
    int dif;

    // Calculations with current settings
    current_test_pattern->CalcCurrentPattern(Lines, height, width);

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
    bar_to_study->GetDeltaColor(YUV, &val[0], &val[1], &val[2], &val[3]);
    dif = val[idx];
    if (dif < 0)
    {
        dif = -dif;
    }
    if (dif < min_dif)
    {
        min_dif = dif;
        best_val = current_val;
        best_val2 = current_val;
    }
    else if (dif == min_dif)
    {
        best_val2 = current_val;
    }
    LOG(3, "Automatic Calibration - setp %d - %d %d %d", current_step, current_val, dif, min_dif);

    if ((current_val < max_val) && ((dif - min_dif) <= 6))
    {
        // Increase the setting
        current_val++;
        Setting_SetValue(BT848_GetSetting(setting), current_val);

        return FALSE;
    }
    else
    {
        // Set the setitng to the best value found
        // If several settings give same result, we choose the middle setting
        if (best_val != best_val2)
        {
            val[0] = best_val + (best_val2 - best_val) / 2;
        }
        else
        {
            val[0] = best_val;
        }
        Setting_SetValue(BT848_GetSetting(setting), val[0]);

        LOG(2, "Automatic Calibration - step %d finished - %d %d => %d", current_step, best_val, best_val2, val[0]);

        return TRUE;
    }
}

void CCalibration::step_init2(BT848_SETTING setting1, int min1, int max1, BT848_SETTING setting2, int min2, int max2)
{
    // Initialize
    min_val = Setting_GetValue(BT848_GetSetting(setting1)) - 5;
    if (min_val < min1)
    {
        min_val = min1;
    }
    max_val = Setting_GetValue(BT848_GetSetting(setting1)) + 5;
    if (max_val > max1)
    {
        max_val = max1;
    }
    current_val = min_val;
    best_val = min_val;

    min_val2 = Setting_GetValue(BT848_GetSetting(setting2)) - 5;
    if (min_val2 < min2)
    {
        min_val2 = min2;
    }
    max_val2 = Setting_GetValue(BT848_GetSetting(setting2)) + 5;
    if (max_val2 > max2)
    {
        max_val2 = max2;
    }
    current_val2 = min_val2;
    best_val2 = min_val2;

    min_dif = 100000;

    // Set the settings to their minimum
    Setting_SetValue(BT848_GetSetting(setting1), current_val);
    Setting_SetValue(BT848_GetSetting(setting2), current_val2);
}

BOOL CCalibration::step_process2(short **Lines, int height, int width, BT848_SETTING setting1, BT848_SETTING setting2, unsigned int sig_component)
{
    int val[4];
    BOOL YUV;
    int idx;
    int dif;

    // Calculations with current settings
    current_test_pattern->CalcCurrentPattern(Lines, height, width);

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
    current_test_pattern->GetSumDeltaColor(YUV, &val[0], &val[1], &val[2], &val[3]);
    dif = val[idx];
    if (dif < 0)
    {
        dif = -dif;
    }
    if (dif < min_dif)
    {
        min_dif = dif;
        best_val = current_val;
        best_val2 = current_val2;
    }
    LOG(3, "Automatic Calibration - setp %d - %d %d => %d %d", current_step, current_val, current_val2, dif, min_dif);

    if (current_val2 < max_val2)
    {
        // Increase the second setting
        current_val2++;
        Setting_SetValue(BT848_GetSetting(setting2), current_val2);

        return FALSE;
    }
    else if (current_val < max_val)
    {
        // Increase the first setting
        current_val++;
        Setting_SetValue(BT848_GetSetting(setting1), current_val);

        // Set the second setting to its minimum
        current_val2 = min_val2;
        Setting_SetValue(BT848_GetSetting(setting2), current_val2);

        return FALSE;
    }
    else
    {
        // Set the setitngs to the best values found
        Setting_SetValue(BT848_GetSetting(setting1), best_val);
        Setting_SetValue(BT848_GetSetting(setting2), best_val2);

        LOG(2, "Automatic Calibration - step %d finished - %d %d => %d", current_step, best_val, best_val2, min_dif);

        return TRUE;
    }
}

/////////////////////////////////////////////////////////////////////////////

CCalibration *pCalibration = NULL;

