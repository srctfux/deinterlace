/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 John Adcock.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////
//
//	This file is subject to the terms of the GNU General Public License as
//	published by the Free Software Foundation.  A copy of this license is
//	included with this software distribution in the file COPYING.txt.  If you
//	do not have a copy, you may obtain a copy by writing to the Free
//	Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//
//	This software is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details
//
/////////////////////////////////////////////////////////////////////////////
// Plugin.c
/////////////////////////////////////////////////////////////////////////////
// Visual plugin to bTV

#include <windows.h>
#include "Plugin.h"

#pragma warning(disable: 4799)		// disable 'no EMMS instruction' warning


/////////////////////////////////////////////////////////////////////////////
// CopyLine
// Uses MMX instructions to move memory around
/////////////////////////////////////////////////////////////////////////////
void CopyLine(unsigned short *Src, unsigned short *Dest)
{
	__asm
	{
		mov		esi, dword ptr[Src]
		mov		edi, dword ptr[Dest]
		mov		ecx, FIELD_W/32				// FIELD_W / (8 Regs * 4 PixelsPerReg)

align 16
CopyLoop:
		movq	mm0, qword ptr[esi]
		movq	mm1, qword ptr[esi+8*1]
		movq	mm2, qword ptr[esi+8*2]
		movq	mm3, qword ptr[esi+8*3]
		movq	mm4, qword ptr[esi+8*4]
		movq	mm5, qword ptr[esi+8*5]
		movq	mm6, qword ptr[esi+8*6]
		movq	mm7, qword ptr[esi+8*7]
		movq	qword ptr[edi], mm0
		movq	qword ptr[edi+8*1], mm1
		movq	qword ptr[edi+8*2], mm2
		movq	qword ptr[edi+8*3], mm3
		movq	qword ptr[edi+8*4], mm4
		movq	qword ptr[edi+8*5], mm5
		movq	qword ptr[edi+8*6], mm6
		movq	qword ptr[edi+8*7], mm7
		add		esi, 64						// 8 Regs * 4 PixelsPerReg * 2 BytesPerPixel
		add		edi, 64

		dec		ecx
		jne		near CopyLoop
		emms
	}
}

/////////////////////////////////////////////////////////////////////////////
// plugin_DoField
// Note we are using flipping so we only want to let the screen show when we 
// have a full frame
/////////////////////////////////////////////////////////////////////////////
__declspec(dllexport)
long plugin_DoField(PLUGINPARAMS *pp)
{
	int y;

	// if key1 button is pressed then flip after even
	// this seems to work for Video and TV sources
	if(pp->key1)
	{
		if(pp->odd)
		{
			for (y = 0; y < FIELD_H; y++)
			{
				CopyLine(pp->src[y], pp->destodd[y]);
			}
			return 0;
		}
		else
		{
			for (y = 0; y < FIELD_H; y++)
			{
				CopyLine(pp->src[y], pp->desteven[y]);
			}
			return RET_FLIPME;
		}
	}
	else
	// if key1 button is not pressed then flip after odd
	// this seems to work for DVD and Laserdisc sources
	{
		if(pp->odd)
		{
			for (y = 0; y < FIELD_H; y++)
			{
				CopyLine(pp->src[y], pp->destodd[y]);
			}
			return RET_FLIPME;
		}
		else
		{
			for (y = 0; y < FIELD_H; y++)
			{
				CopyLine(pp->src[y], pp->desteven[y]);
			}
			return 0;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// plugin_DoField
// Say we want to use page flipping
/////////////////////////////////////////////////////////////////////////////
__declspec(dllexport)
int plugin_GetProps(PLUGINPROPS *pp, int lang)
{
	pp->shortname = "PALMovie";
	pp->longname = "PALMovie v1.0 (c) by John Adcock in 2000";
	pp->flags = PLUGINF_HAVECONFIG + PLUGINF_WANTSFLIPPING;
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// plugin_Config
// This is called when user selects the plugin regardless it's active or not...
/////////////////////////////////////////////////////////////////////////////
__declspec(dllexport)
void plugin_Config(HWND w, int lang)
{
	MessageBox(w, "Make sure page flipping is turned on.\n\n"
					"Use Key1 to choose which field to flip on.\n"
					"Key1 off seems to work for DVD and LaserDiscs\n"
					"Key1 on seems to work for Video and TV\n\n"
					"(c) John Adcock 2000.\n\n"
					"PALMovie comes with ABSOLUTELY NO WARRANTY.\n"
					"This software in licenced under the terms of the GNU GPL\n"
					"for details see http://www.fsf.net/copyleft/gpl.html\n\n"
					"The Sourceforge project page for this plug-in is \n"
					"http://deinterlace.sourceforge.net/",
					"PALMovie config", MB_OK);
}

/////////////////////////////////////////////////////////////////////////////
// plugin_Initialize
// This is called when user selects the plugin regardless it's active or not...
/////////////////////////////////////////////////////////////////////////////
__declspec(dllexport)
void plugin_Initialize()
{
}

/////////////////////////////////////////////////////////////////////////////
// plugin_UnInitialize
// This is called when user changes some other plugin regardless it's active or not...
/////////////////////////////////////////////////////////////////////////////
__declspec(dllexport)
void plugin_UnInitialize()
{
}

