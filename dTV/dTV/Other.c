/////////////////////////////////////////////////////////////////////////////
// other.h
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 John Adcock.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////
//
//	This file is subject to the terms of the GNU General Public License as
//	published by the Free Software Foundation.  A copy of this license is
//	included with this software distribution in the file COPYING.  If you
//	do not have a copy, you may obtain a copy by writing to the Free
//	Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//
//	This software is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details
/////////////////////////////////////////////////////////////////////////////
//
// This software was based on Multidec 5.6 Those portions are
// Copyright (C) 1999/2000 Espresso (echter_espresso@hotmail.com)
//
// This was in turn based on  Linux code by:
//
// BT-Parts
//
// Copyright (C) 1996,97,98 Ralph  Metzler (rjkm@thp.uni-koeln.de)
//                         & Marcus Metzler (mocm@thp.uni-koeln.de)
// msp34XX
//
// Copyright (C) 1997,1998 Gerd Knorr <kraxel@goldbach.in-berlin.de>
//
// Copyright (C) 1996,97,98 Ralph  Metzler (rjkm@thp.uni-koeln.de)
//                         & Marcus Metzler (mocm@thp.uni-koeln.de)
// msp34XX
//
// Copyright (C) 1997,1998 Gerd Knorr <kraxel@goldbach.in-berlin.de>
//
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 24 Jul 2000   John Adcock           Original Release
//                                     Translated most code from German
//                                     Combined Header files
//                                     Cut out all decoding
//                                     Cut out digital hardware stuff
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "other.h"
#include "dTV.h"
#include "bt848.h"
#include "OutThreads.h"
#include "VBI_VideoText.h"

LPDIRECTDRAWSURFACE     lpDDSurface = NULL;
// OverLay
LPDIRECTDRAWSURFACE     lpDDOverlay = NULL;
LPDIRECTDRAWSURFACE     lpDDOverlayBack = NULL;
BYTE* lpOverlay = NULL;
BYTE* lpOverlayBack = NULL;
long OverlayPitch = 0;
BOOL Can_ColorKey=FALSE;
DWORD DestSizeAlign;
DWORD SrcSizeAlign;
COLORREF OverlayColor = RGB(255, 0, 255);
int Back_Buffers;		// Make new user parm, TRB 10/28/00
void ExitDD(void)
{
	if (lpDD != NULL)
	{
		if (lpDDOverlay != NULL)
		{
			Overlay_Update(NULL, NULL, DDOVER_HIDE, FALSE);
			IDirectDrawSurface_Release(lpDDOverlay);
		}
		lpDDOverlay = NULL;
		if (lpDDSurface != NULL)
		{
			IDirectDrawSurface_Release(lpDDSurface);
		}
		lpDDSurface = NULL;
		IDirectDraw_Release(lpDD);
		lpDD = NULL;
	}
}

void Overlay_Clean()
{
	int nPixel;
	int nLine;
	HRESULT ddrval;
	DDSURFACEDESC ddsd;

	if (lpDDOverlay != NULL)
	{
		memset(&ddsd, 0x00, sizeof(ddsd));
		ddsd.dwSize = sizeof(ddsd);

		ddrval = IDirectDrawSurface_Lock(lpDDOverlay, NULL, &ddsd, DDLOCK_WAIT, NULL);

		for (nLine = 0; nLine < (signed) ddsd.dwHeight; nLine++)
		{
			for (nPixel = 0; nPixel < (signed) ddsd.dwWidth * 2; nPixel += 4)
			{
				*((int *) ddsd.lpSurface + (nLine * ddsd.lPitch + nPixel) / 4) = 0x80008000;
			}
		}
		ddrval = IDirectDrawSurface_Unlock(lpDDOverlay, ddsd.lpSurface);
	}
	if (lpDDOverlayBack != NULL)
	{
		memset(&ddsd, 0x00, sizeof(ddsd));
		ddsd.dwSize = sizeof(ddsd);

		ddrval = IDirectDrawSurface_Lock(lpDDOverlayBack, NULL, &ddsd, DDLOCK_WAIT, NULL);

		for (nLine = 0; nLine < (signed) ddsd.dwHeight; nLine++)
		{
			for (nPixel = 0; nPixel < (signed) ddsd.dwWidth * 2; nPixel += 4)
			{
				*((int *) ddsd.lpSurface + (nLine * ddsd.lPitch + nPixel) / 4) = 0x80008000;
			}
		}
		ddrval = IDirectDrawSurface_Unlock(lpDDOverlayBack, ddsd.lpSurface);
	}
}

BOOL Overlay_Update(LPRECT pSrcRect, LPRECT pDestRect, DWORD dwFlags, BOOL ColorKey)
{
	HRESULT ddrval;
	DDOVERLAYFX DDOverlayFX;

	if ((lpDD == NULL) || (lpDDSurface == NULL) || (lpDDOverlay == NULL))
	{
		return (FALSE);
	}

	memset(&DDOverlayFX, 0x00, sizeof(DDOverlayFX));
	DDOverlayFX.dwSize = sizeof(DDOverlayFX);

	if (pSrcRect == NULL)
	{
		ddrval = IDirectDrawSurface_UpdateOverlay(lpDDOverlay, NULL, lpDDSurface, NULL, dwFlags, &DDOverlayFX);
		if (ddrval != DD_OK)
		{
			ErrorBox("Error hiding Overlay");
		}
		return (TRUE);
	}

	if (! bIsFullScreen)
		dwFlags |= DDOVER_KEYDESTOVERRIDE;
	DDOverlayFX.dckDestColorkey.dwColorSpaceHighValue = Overlay_ColorMatch(lpDDSurface, OverlayColor);
	DDOverlayFX.dckDestColorkey.dwColorSpaceLowValue = DDOverlayFX.dckDestColorkey.dwColorSpaceHighValue;

	ddrval = IDirectDrawSurface_UpdateOverlay(lpDDOverlay, pSrcRect, lpDDSurface, pDestRect, dwFlags, &DDOverlayFX);
	if (ddrval != DD_OK)
	{
		ErrorBox("Error calling Overlay_Update");
		return (FALSE);
	}

	return TRUE;
}

BOOL Overlay_Create()
{
	DDSURFACEDESC ddsd;
	DDPIXELFORMAT PixelFormat;
	HRESULT ddrval;
	DDSCAPS caps;

	memset(&PixelFormat, 0x00, sizeof(PixelFormat));
	PixelFormat.dwSize = sizeof(DDPIXELFORMAT);
	PixelFormat.dwFlags = DDPF_FOURCC;
	PixelFormat.dwFourCC = MAKEFOURCC('Y', 'U', 'Y', '2');;
	PixelFormat.dwYUVBitCount = 16;

	memset(&ddsd, 0x00, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT | DDSD_BACKBUFFERCOUNT;
	ddsd.ddsCaps.dwCaps = DDSCAPS_OVERLAY | DDSCAPS_VIDEOMEMORY | DDSCAPS_FLIP | DDSCAPS_COMPLEX;

	// create a much bigger surface than we need
	// this ensures that we can use the bTV plugin
	ddsd.dwWidth = BTV_VER1_WIDTH;
	ddsd.dwHeight = BTV_VER1_HEIGHT;
	ddsd.dwBackBufferCount = Back_Buffers;

	ddsd.ddpfPixelFormat = PixelFormat;
	ddrval = IDirectDraw_CreateSurface(lpDD, &ddsd, &lpDDOverlay, NULL);
	if (FAILED(ddrval))
	{
		ErrorBox("Can't create Overlay Surface");
		lpDDOverlay = NULL;
		return FALSE;
	}

	ddrval = IDirectDrawSurface_Lock(lpDDOverlay, NULL, &ddsd, DDLOCK_WAIT, NULL);
	if (FAILED(ddrval))
	{
		ErrorBox("Can't Lock Surface");
		ddrval = DDERR_WASSTILLDRAWING;
		return (FALSE);
	}
	ddrval = DDERR_WASSTILLDRAWING;
	OverlayPitch = ddsd.lPitch;
	lpOverlay = ddsd.lpSurface;

	ddrval = IDirectDrawSurface_Unlock(lpDDOverlay, ddsd.lpSurface);
	if (FAILED(ddrval))
	{
		ErrorBox("Can't Unlock Surface");
		return (FALSE);
	}

	caps.dwCaps = DDSCAPS_BACKBUFFER;
	ddrval = IDirectDrawSurface_GetAttachedSurface(lpDDOverlay, &caps, &lpDDOverlayBack);
	if (FAILED(ddrval))
	{
		ErrorBox("Can't create Overlay Back Surface");
		lpDDOverlayBack = NULL;
		return (FALSE);
	}
	else
	{
		ddrval = IDirectDrawSurface_Lock(lpDDOverlayBack, NULL, &ddsd, DDLOCK_WAIT, NULL);
		if (FAILED(ddrval))
		{
			ErrorBox("Can't Lock Back Surface");
			return (FALSE);
		}
		ddrval = DDERR_WASSTILLDRAWING;
		lpOverlayBack = ddsd.lpSurface;
		ddrval = IDirectDrawSurface_Unlock(lpDDOverlayBack, ddsd.lpSurface);
		if (FAILED(ddrval))
		{
			ErrorBox("Can't Unlock Back Surface");
			return (FALSE);
		}
	}

	Overlay_Clean();

	return (TRUE);
}

//-----------------------------------------------------------------------------
// Name: DDColorMatch()
// Desc: Convert a RGB color to a pysical color.
//       We do this by leting GDI SetPixel() do the color matching
//       then we lock the memory and see what it got mapped to.
//-----------------------------------------------------------------------------
DWORD Overlay_ColorMatch(LPDIRECTDRAWSURFACE pdds, COLORREF rgb)
{
    COLORREF rgbT;
    HDC hdc;
    DWORD dw = CLR_INVALID;
    DDSURFACEDESC ddsd;
    HRESULT hres;

    //
    //  Use GDI SetPixel to color match for us
    //
	hres = IDirectDrawSurface_GetDC(pdds, &hdc);
    if (SUCCEEDED(hres))
    {
        rgbT = GetPixel(hdc, 0, 0);     // Save current pixel value
        SetPixel(hdc, 0, 0, rgb);       // Set our value
        IDirectDrawSurface_ReleaseDC(pdds, hdc);
    }
    //
    // Now lock the surface so we can read back the converted color
    //
    ddsd.dwSize = sizeof(ddsd);
    hres = IDirectDrawSurface_Lock(pdds, NULL, &ddsd, DDLOCK_WAIT, NULL);
    if (SUCCEEDED(hres))
    {
        dw = *(DWORD *) ddsd.lpSurface;                 // Get DWORD
        if (ddsd.ddpfPixelFormat.dwRGBBitCount < 32)
		{
            dw &= (1 << ddsd.ddpfPixelFormat.dwRGBBitCount) - 1;  // Mask it to bpp
		}
        IDirectDrawSurface_Unlock(pdds, NULL);
    }
    //
    //  Now put the color that was there back.
    //
	hres = IDirectDrawSurface_GetDC(pdds, &hdc);
    if (SUCCEEDED(hres))
    {
        SetPixel(hdc, 0, 0, rgbT);
        IDirectDrawSurface_ReleaseDC(pdds, hdc);
    }
    return dw;
}


BOOL InitDD(HWND hWnd)
{
	HRESULT ddrval;
	DDCAPS DriverCaps;
	DDSURFACEDESC ddsd;

	if (DirectDrawCreate(NULL, &lpDD, NULL) != DD_OK)
	{
		ErrorBox("DirectDrawCreate failed");
		return (FALSE);
	}

	// can we use Overlay ??
	memset(&DriverCaps, 0x00, sizeof(DriverCaps));
	DriverCaps.dwSize = sizeof(DriverCaps);
	ddrval = IDirectDraw_GetCaps(lpDD, &DriverCaps, NULL);

	if (ddrval == DD_OK)
	{
		if (DriverCaps.dwCaps & DDCAPS_OVERLAY)
		{
			if (!(DriverCaps.dwCaps & DDCAPS_OVERLAYSTRETCH))
			{
				ErrorBox("Can't Strech Overlay");
				return FALSE;
			}

			if (!(DriverCaps.dwCKeyCaps & DDCKEYCAPS_DESTOVERLAY))
			{
				ErrorBox("Can't ColorKey Overlay");
				return FALSE;
			}

			if (DriverCaps.dwCaps & DDCAPS_ALIGNSIZESRC)
			{
				SrcSizeAlign = DriverCaps.dwAlignSizeSrc;
			}
			else
			{
				SrcSizeAlign = 1;
			}

			if (DriverCaps.dwCaps & DDCAPS_ALIGNSIZEDEST)
			{
				DestSizeAlign = DriverCaps.dwAlignSizeDest;
			}
			else
			{
				DestSizeAlign = 1;
			}

		}
		else
		{
			ErrorBox("Can't Use Overlay");
			return (FALSE);
		}
	}

	ddrval = IDirectDraw_SetCooperativeLevel(lpDD, hWnd, DDSCL_NORMAL);

	if (ddrval != DD_OK)
	{
		ErrorBox("SetCooperativeLevel failed");
		return (FALSE);
	}

	memset(&ddsd, 0x00, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS;
	ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

	if (IDirectDraw_CreateSurface(lpDD, &ddsd, &lpDDSurface, NULL) != DD_OK)
	{
		ErrorBox("Error Creating Primary surface");
		return (FALSE);
	}

	ddrval = IDirectDrawSurface_Lock(lpDDSurface, NULL, &ddsd, DDLOCK_WAIT, NULL);
	ddrval = IDirectDrawSurface_Unlock(lpDDSurface, ddsd.lpSurface);
	return TRUE;
}

#define LIMIT(x) (((x)<0)?0:((x)>255)?255:(x))

void SaveStill()
{
	int y, cr, cb, r, g, b, i, j, n = 0;
	FILE *file;
	BYTE rgb[3];
	BYTE* buf;
	char name[13];
	struct stat st;
	DDSURFACEDESC ddsd;
	HRESULT ddrval;

	if (lpDDOverlay != NULL)
	{
		memset(&ddsd, 0x00, sizeof(ddsd));
		ddsd.dwSize = sizeof(ddsd);

		ddrval = IDirectDrawSurface_Lock(lpDDOverlay, NULL, &ddsd, DDLOCK_WAIT, NULL);
		if (FAILED(ddrval))
		{
			ErrorBox("Error Locking Overlay");
			return;
		}

		while (n < 100)
		{
			sprintf(name,"tv%06d.ppm",++n) ;
			if (stat(name, &st))
				break;
		}
		if(n == 100)
		{
			ErrorBox("Could not create a file.  You may have too many captures already.");
			ddrval = IDirectDrawSurface_Unlock(lpDDOverlay, ddsd.lpSurface);
			if (FAILED(ddrval))
			{
				ErrorBox("Error Unlocking Overlay");
				return;
			}
			return;
		}

		file = fopen(name,"wb");
		if (!file)
		{
			ErrorBox("Could not open file in SaveStill");
			ddrval = IDirectDrawSurface_Unlock(lpDDOverlay, ddsd.lpSurface);
			if (FAILED(ddrval))
			{
				ErrorBox("Error Unlocking Overlay");
				return;
			}
			return;
		}
		fprintf(file,"P6\n%d %d\n255\n",CurrentX, CurrentY) ;

		for (i = 0; i < CurrentY; i++ )
		{
			buf = (BYTE*)ddsd.lpSurface + i * ddsd.lPitch;
			for (j = 0; j < CurrentX ; j+=2)
			{
				cb = buf[1] - 128;
				cr = buf[3] - 128;
				y = buf[0] - 16;

				r = ( 76284*y + 104595*cr             )>>16;
				g = ( 76284*y -  53281*cr -  25624*cb )>>16;
				b = ( 76284*y             + 132252*cb )>>16;
				rgb[0] = LIMIT(r);
				rgb[1] = LIMIT(g);
				rgb[2] = LIMIT(b);

				fwrite(rgb,3,1,file) ;

				y = buf[2] - 16;
				r = ( 76284*y + 104595*cr             )>>16;
				g = ( 76284*y -  53281*cr -  25624*cb )>>16;
				b = ( 76284*y             + 132252*cb )>>16;
				rgb[0] = LIMIT(r);
				rgb[1] = LIMIT(g);
				rgb[2] = LIMIT(b);
				fwrite(rgb,3,1,file);

				buf += 4;
			}
		}
		fclose(file);
		ddrval = IDirectDrawSurface_Unlock(lpDDOverlay, ddsd.lpSurface);
		if (FAILED(ddrval))
		{
			ErrorBox("Error Unlocking Overlay");
			return;
		}
	}
	return;
}
