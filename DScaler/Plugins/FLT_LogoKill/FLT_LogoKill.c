/////////////////////////////////////////////////////////////////////////////
// $Id: FLT_LogoKill.c,v 1.14 2002-09-08 10:56:02 robmuller Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 John Adcock.  All rights reserved.
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
// This code is based on the 
// LogoAway VirtualDub filter by Krzysztof Wojdon (c) 2000
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.13  2002/08/17 11:42:06  kooiman
// Applied LogoKiller Filter Smoothing option from Jochen Trenner.
//
// Revision 1.12  2002/06/18 19:46:09  adcockj
// Changed appliaction Messages to use WM_APP instead of WM_USER
//
// Revision 1.11  2002/06/13 12:10:26  adcockj
// Move to new Setings dialog for filers, video deint and advanced settings
//
// Revision 1.10  2002/04/30 18:17:06  adcockj
// New weighted logo killer modes from Jochen Trenner
//
// Revision 1.9  2001/12/15 17:50:47  adcockj
// Fix for zero width bug
//
// Revision 1.8  2001/11/26 15:27:19  adcockj
// Changed filter structure
//
// Revision 1.7  2001/11/21 15:21:41  adcockj
// Renamed DEINTERLACE_INFO to TDeinterlaceInfo in line with standards
// Changed TDeinterlaceInfo structure to have history of pictures.
//
// Revision 1.6  2001/08/09 21:34:59  adcockj
// Fixed bugs raise by Timo and Keld
//
// Revision 1.5  2001/07/13 16:13:33  adcockj
// Added CVS tags and removed tabs
//
/////////////////////////////////////////////////////////////////////////////

#include "windows.h"
#include "DS_Filter.h"
#include "..\help\helpids.h"

long Top = 5;
long Left = 5;
long Width = 30;
long Height = 30;
long Max = 128;
long gUseSmoothing = TRUE;
long gLogoThreshold = 50;

typedef enum
{
    MODE_GREY,
    MODE_MAX,
    MODE_DYNAMIC_MAX,
    MODE_WEIGHTED_C,
    MODE_WEIGHTED_ASM,
    MODE_LASTONE,
} eMODE;

eMODE Mode = MODE_GREY;

LPCSTR ModeList[] =
{
    "Grey",
    "Limit To Max Value",
    "Dynamic Max",
    "Weighted Average (c)",
    "Weighted Average (asm)",
    "Dynamic Max",
};

SETTING FLT_LogoKillSettings[FLT_LOGOKILL_SETTING_LASTONE];


long LogoKiller(TDeinterlaceInfo* pInfo)
{
    BYTE* lpOverlay = pInfo->Overlay + Left * 8;
    const __int64 qwGrey = 0x7f7f7f7f7f7f7f7f;
    long Pitch = pInfo->OverlayPitch;

    // we use some of the integer SSE instructions these are supported
    // either by PIII and above or by Althons and above
    if((pInfo->CpuFeatureFlags & FEATURE_SSE) || (pInfo->CpuFeatureFlags & FEATURE_MMXEXT))
    {
        // we are OK to use everything
    }
    else
    {
        // we'll just fall back to GREY mode on
        // old style CPUs
        Mode = MODE_GREY;
    }

    // check bounds
    if((Top + Height) >= pInfo->FrameHeight ||
        (Left + Width) >= pInfo->FrameWidth / 4)
    {
        return 1000;
    }

    switch(Mode)
    {
    case MODE_DYNAMIC_MAX:
        // here we find the maximum luma from the top and bottom lines
        // then use this value to max out any brighter areas in the middle
        _asm
        {
            // start off with zero as the max luma value
            // for each of the pixels in the mmx 4
            mov eax, 0xff00
            pinsrw mm1, eax, 0
            pinsrw mm1, eax, 1
            pinsrw mm1, eax, 2
            pinsrw mm1, eax, 3
            
            // set edi to be the top right corner
            mov edi, lpOverlay
            mov ebx, Pitch
            mov eax, Top
            imul eax, ebx
            add edi, eax

            // loop through the top line looking for the max
            // luma values in each of the positions
            mov edx, edi
            mov ecx, Width
            LOOP_FINDMAX1:
            movq mm0, qword ptr[edx]
            pmaxub mm1, mm0
            add edx, 8
            dec ecx
            jnz LOOP_FINDMAX1

            // goto bottom line
            mov eax, Height
            sub ecx, 1
            imul eax, ebx
            add edi, eax

            // loop through the bottom line looking for the max
            // luma values in each of the positions
            mov edx, edi
            mov ecx, Width
            LOOP_FINDMAX2:
            movq mm0, qword ptr[edx]
            pmaxub mm1, mm0
            add edx, 8
            dec ecx
            jnz LOOP_FINDMAX2
            
            // go back to the top
            sub edi, eax

            // here we get the maximum
            // luma and have it in the end position
            movq mm2, mm1            
            movq mm3, mm1            
            movq mm4, mm1            
            psrlq mm2, 16
            psrlq mm3, 32
            psrlq mm4, 48
            pmaxub mm1, mm2
            pmaxub mm3, mm4
            pmaxub mm1, mm3
            
            // now fill mm1 with the dynamically found
            // max value
            pextrw eax, mm1, 0
            pinsrw mm1, eax, 1
            pinsrw mm1, eax, 2
            pinsrw mm1, eax, 3

            // loop through and use the max value
            mov eax, Height
            LOOP_DYN_MAX_OUTER:
            mov edx, edi
            mov ecx, Width
            LOOP_DYN_MAX_INNER:
            movq mm0, qword ptr[edx]
            pminub mm0, mm1
            movq qword ptr[edx], mm0
            add edx, 8
            dec ecx
            jnz LOOP_DYN_MAX_INNER
            add edi, ebx
            dec eax
            jnz LOOP_DYN_MAX_OUTER
        }
        break;
    case MODE_MAX:
        _asm
        {
            // set up mm1 as the max value to be used
            mov eax, 0xff00
            add eax, Max
            pinsrw mm1, eax, 0
            pinsrw mm1, eax, 1
            pinsrw mm1, eax, 2
            pinsrw mm1, eax, 3

            // set edi to top left
            mov edi, lpOverlay
            mov ebx, Pitch
            mov eax, Top
            imul eax, ebx
            add edi, eax
            // for each row
            mov eax, Height
            LOOP_MAX_OUTER:
            // for each 4 pixel chunk
            mov edx, edi
            mov ecx, Width
            LOOP_MAX_INNER:
            movq mm0, qword ptr[edx]
            pminub mm0, mm1
            movq qword ptr[edx], mm0
            add edx, 8
            dec ecx
            jnz LOOP_MAX_INNER
            add edi, ebx
            dec eax
            jnz LOOP_MAX_OUTER
        }
        break;
    // weighted average mode supllied by
    // Jochen Trenner
    case MODE_WEIGHTED_C:
		{
			int i,j;
			long Width2;
			long Mul1, Mul2, Mul3, Mul4;
			long Weight1, Weight2;
			long up1, up2, up3, up4, 
				down1, down2, down3, down4,
				left1, left2, left3, left4,
				right1, right2, right3, right4;
			long	ipo1, ipo2, ipo3, ipo4;
			BYTE* lpOverlay_Pointer;
			BYTE* lpCurrent_Pointer;
			int top_pitch;

			Width2=Width*2;
			top_pitch=Pitch*Top;
			lpOverlay_Pointer=lpOverlay+top_pitch;

			for (i=0;i<Height;i++)
			{
				left1=*(lpOverlay_Pointer+i*Pitch);
				left2=*(lpOverlay_Pointer+i*Pitch+1);
				left3=*(lpOverlay_Pointer+i*Pitch+2);
				left4=*(lpOverlay_Pointer+i*Pitch+3);
				right1=*(lpOverlay_Pointer+i*Pitch+Width2*4);
				right2=*(lpOverlay_Pointer+i*Pitch+Width2*4+1);
				right3=*(lpOverlay_Pointer+i*Pitch+Width2*4+2);
				right4=*(lpOverlay_Pointer+i*Pitch+Width2*4+3);
				Mul1=abs(Height-1-i);
				Mul2=i;
				Weight1=abs(((Height-1)/2)-i);
				++Weight1;
				for(j=0;j<Width2;j++)
				{
					Weight2=abs(Width-1-j);
					++Weight2;
					Mul3=abs(Width2-1-j);
					Mul4=j;
					up1=*(lpOverlay_Pointer+j*4);
					up2=*(lpOverlay_Pointer+j*4+1);
					up3=*(lpOverlay_Pointer+j*4+2);
					up4=*(lpOverlay_Pointer+j*4+3);

					down1=*(lpOverlay_Pointer+Pitch*Height+j*4);
					down2=*(lpOverlay_Pointer+Pitch*Height+j*4+1);
					down3=*(lpOverlay_Pointer+Pitch*Height+j*4+2);
					down4=*(lpOverlay_Pointer+Pitch*Height+j*4+3);
					lpCurrent_Pointer=lpOverlay_Pointer+i*Pitch+j*4;

					ipo1=(((((Mul1*up1)+(Mul2*down1))*Weight1/(Mul1+Mul2))+(((Mul3*left1)+Mul4*right1)*Weight2/(Mul3+Mul4)))/(Weight1+Weight2));
					ipo2=(((((Mul1*up2)+(Mul2*down2))*Weight1/(Mul1+Mul2))+(((Mul3*left2)+Mul4*right2)*Weight2/(Mul3+Mul4)))/(Weight1+Weight2));
					ipo3=(((((Mul1*up3)+(Mul2*down3))*Weight1/(Mul1+Mul2))+(((Mul3*left3)+Mul4*right3)*Weight2/(Mul3+Mul4)))/(Weight1+Weight2));
					ipo4=(((((Mul1*up4)+(Mul2*down4))*Weight1/(Mul1+Mul2))+(((Mul3*left4)+Mul4*right4)*Weight2/(Mul3+Mul4)))/(Weight1+Weight2));

					if (i>Height/3 && i<2*Height/3)
					{
						ipo1=(2*ipo1+up1+down1)/4;
						ipo2=(2*ipo2+up2+down2)/4;
						ipo3=(2*ipo3+up3+down3)/4;
						ipo4=(2*ipo4+up4+down4)/4;

					}



					*lpCurrent_Pointer=(BYTE)ipo1;
					*(lpCurrent_Pointer+1)=(BYTE)ipo2;
					*(lpCurrent_Pointer+2)=(BYTE)ipo3;
					*(lpCurrent_Pointer+3)=(BYTE)ipo4;
				}
			}

if(gUseSmoothing)
{
	_asm
        {
            // set edi to top left
            mov edi, lpOverlay
            mov ebx, Pitch
            mov eax, Top
            imul eax, ebx
            add edi, eax
            // loop over height
            mov eax, Height
            LOOP_SMOOTH_OUTER:
            // loop over width
            mov edx, edi
            mov ecx, Width
            LOOP_SMOOTH_INNER:
			add edx, ebx //2 samples one line below
			movq mm4, qword ptr[edx-8]
			movq mm5, qword ptr[edx+8]
			pavgb mm4, mm5
			sub edx, ebx
            sub edx, ebx //2 samples one line above
			movq mm1, qword ptr[edx-8]
			movq mm2, qword ptr[edx+8]
			pavgb mm1, mm2
			pavgb mm1, mm4
			add edx, ebx
            movq mm0, qword ptr[edx]
			pavgb mm0,mm1
            movq qword ptr[edx], mm0
            add edx, 8
            dec ecx
            jnz LOOP_SMOOTH_INNER
            add edi, ebx
            dec eax
            jnz LOOP_SMOOTH_OUTER
        }
}
        }
        break;
    // weighted average mode supplied by
    // Jochen Trenner
    case MODE_WEIGHTED_ASM:
		{
			int Width_third=Width/3;
			int Height_third=Height/3;
			int Height_2third=Height/3*2;
        _asm
        {
            mov edi, lpOverlay
            mov ebx, Pitch
            mov eax, Top
            imul eax, ebx
            add edi, eax
            mov eax, Height
            LOOP_JT1_OUTER:
            mov edx, edi
            movq mm0, qword ptr[edx]
			add edx, Width
			add edx, Width
			add edx, Width
			add edx, Width
			add edx, Width
			add edx, Width
			add edx, Width
			add edx, Width
            movq mm1, qword ptr[edx]
			sub edx, Width
			sub edx, Width
			sub edx, Width
			sub edx, Width
			sub edx, Width
			sub edx, Width
			sub edx, Width
			sub edx, Width
			movq mm2, mm0
			pavgb mm2, mm1
            mov ecx, Width_third

            LOOP_JT1_LEFT:
			push eax
			push ecx
			mov ecx, Height
			sub ecx, eax
			imul ecx, ebx
			sub edx, ecx
			movq mm3, qword ptr[edx]
			add edx, ecx
			imul eax, ebx
			add edx, eax
			movq mm4, qword ptr[edx]
			sub edx, eax
			pop ecx
			pop eax
			cmp eax, Height_2third
			jge LA
			cmp eax, Height_third
			jge LM
			movq mm5, mm4
			jmp LEFT_AVG
            LA:			
            movq mm5, mm3
			jmp LEFT_AVG
            LM:
            pavgb mm4, mm3
			movq mm5, mm4
            LEFT_AVG:	
            pavgb mm5, mm0
            movq qword ptr[edx], mm5
            add edx, 8
            dec ecx
            jnz LOOP_JT1_LEFT

            mov ecx, Width_third
            LOOP_JT1_MIDDLE:
			push eax
			push ecx
			mov ecx, Height
			sub ecx, eax
			imul ecx, ebx
			sub edx, ecx
			movq mm3, qword ptr[edx]
			add edx, ecx
			imul eax, ebx
			add edx, eax
			movq mm4, qword ptr[edx]
			sub edx, eax
			pop ecx
			pop eax
			cmp eax, Height_2third
			jge MA
			cmp eax, Height_third
			jge MM
			movq mm5, mm4
			jmp MIDDLE_AVG
            MA:
            movq mm5, mm3
			jmp MIDDLE_AVG
            MM:
            pavgb mm4, mm3
			movq mm5, mm4
            MIDDLE_AVG:	
            pavgb mm5, mm2
			movq qword ptr[edx], mm5
            add edx, 8
            dec ecx
            jnz LOOP_JT1_MIDDLE

            mov ecx, Width_third
            
            LOOP_JT1_RIGHT:
			push eax
			push ecx
			mov ecx, Height
			sub ecx, eax
			imul ecx, ebx
			sub edx, ecx
			movq mm3, qword ptr[edx]
			add edx, ecx
			imul eax, ebx
			add edx, eax
			movq mm4, qword ptr[edx]
			sub edx, eax
			pop ecx
			pop eax
			cmp eax, Height_2third
			jge RA
			cmp eax, Height_third
			jge RM
			movq mm5, mm4
			jmp RIGHT_AVG
            RA:			
            movq mm5, mm3
			jmp RIGHT_AVG
            RM:			
            pavgb mm4, mm3
			movq mm5, mm4
            RIGHT_AVG:	
            pavgb mm5, mm1
            movq qword ptr[edx], mm5
            add edx, 8
            dec ecx
            jnz LOOP_JT1_RIGHT

            add edi, ebx
            dec eax
            jnz LOOP_JT1_OUTER
        }
if(gUseSmoothing)
{
	_asm
        {
            // set edi to top left
            mov edi, lpOverlay
            mov ebx, Pitch
            mov eax, Top
            imul eax, ebx
            add edi, eax
            // loop over height
            mov eax, Height
            LOOP_SMOOTH_OUTER_ASM:
            // loop over width
            mov edx, edi
            mov ecx, Width
            LOOP_SMOOTH_INNER_ASM:
			add edx, ebx //2 samples one line below
			movq mm4, qword ptr[edx-8]
			movq mm5, qword ptr[edx+8]
			pavgb mm4, mm5
			sub edx, ebx
            sub edx, ebx //2 samples one line above
			movq mm1, qword ptr[edx-8]
			movq mm2, qword ptr[edx+8]
			pavgb mm1, mm2
			pavgb mm1, mm4
			add edx, ebx
            movq mm0, qword ptr[edx]
			pavgb mm0,mm1
            movq qword ptr[edx], mm0
            add edx, 8
            dec ecx
            jnz LOOP_SMOOTH_INNER_ASM
            add edi, ebx
            dec eax
            jnz LOOP_SMOOTH_OUTER_ASM
        }
}
        break;
		}
    case MODE_GREY:
    default:
        _asm
        {
            // set up mm0 as mid grey
            movq mm0, qwGrey
            
            // set edi to top left
            mov edi, lpOverlay
            mov ebx, Pitch
            mov eax, Top
            imul eax, ebx
            add edi, eax
            // loop over height
            mov eax, Height
            LOOP_GREY_OUTER:
            // loop over width
            mov edx, edi
            mov ecx, Width
            LOOP_GREY_INNER:
            // set area to grey
            movq qword ptr[edx], mm0
            add edx, 8
            dec ecx
            jnz LOOP_GREY_INNER
            add edi, ebx
            dec eax
            jnz LOOP_GREY_OUTER
        }
        break;
    }
    _asm
    {
        emms
    }
    return 1000;
}

BOOL Top_OnChange(long NewValue)
{
    Top = NewValue;
    return TRUE;   
}

BOOL Left_OnChange(long NewValue)
{
    Left = NewValue;
    return TRUE;   
}

BOOL Width_OnChange(long NewValue)
{
    Width = NewValue;
    return TRUE;   
}

BOOL Height_OnChange(long NewValue)
{
    Height = NewValue;
    return TRUE;   
}

void LinearCorrStart(void)
{
    Top_OnChange(Top);
    Left_OnChange(Left);
}

////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
/////////////////////////////////////////////////////////////////////////////

FILTER_METHOD LogoKillMethod;

SETTING FLT_LogoKillSettings[FLT_LOGOKILL_SETTING_LASTONE] =
{
    {
        "Top", SLIDER, 0, &Top,
        20, 0, 575, 1, 1,
        NULL,
        "LogoKillFilter", "Top", Top_OnChange,
    },
    {
        "Left", SLIDER, 0, &Left,
        20, 0, 191, 1, 1,
        NULL,
        "LogoKillFilter", "Left", Left_OnChange,
    },
    {
        "Width", SLIDER, 0, &Width,
        20, 1, 191, 1, 1,
        NULL,
        "LogoKillFilter", "Width", Width_OnChange,
    },
    {
        "Height", SLIDER, 0, &Height,
        50, 2, 575, 1, 1,
        NULL,
        "LogoKillFilter", "Height", Height_OnChange,
    },
    {
        "Mode", ITEMFROMLIST, 0, (long*)&Mode,
        MODE_GREY, MODE_GREY, MODE_LASTONE -1, 1, 1,
        ModeList,
        "LogoKillFilter", "Mode", NULL,
    },
    {
        "Max", SLIDER, 0, &Max,
        128, 0, 255, 1, 1,
        NULL,
        "LogoKillFilter", "Max", NULL,
    },
    {
        "Logo Kill Filter", ONOFF, 0, &(LogoKillMethod.bActive),
        FALSE, 0, 1, 1, 1,
        NULL,
        "LogoKillFilter", "UseLogoKillFilter", NULL,
    },
    {
        "Smoothing", ONOFF, 0, &gUseSmoothing,
        TRUE, 0, 1, 1, 1,
        NULL,
        "LogoKillFilter", "UseSmoothing", NULL,
    },
};

FILTER_METHOD LogoKillMethod =
{
    sizeof(FILTER_METHOD),
    FILTER_CURRENT_VERSION,
    DEINTERLACE_INFO_CURRENT_VERSION,
    "Logo Killer Filter",
    "&Logo Killer (experimental)",
    FALSE,
    FALSE,
    LogoKiller, 
    0,
    TRUE,
    NULL,
    NULL,
    NULL,
    FLT_LOGOKILL_SETTING_LASTONE,
    FLT_LogoKillSettings,
    WM_FLT_LOGOKILL_GETVALUE - WM_APP,
    TRUE,
    1,
    IDH_LOGO_KILLER,
};


__declspec(dllexport) FILTER_METHOD* GetFilterPluginInfo(long CpuFeatureFlags)
{
    return &LogoKillMethod;
}

BOOL WINAPI _DllMainCRTStartup(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
    return TRUE;
}

