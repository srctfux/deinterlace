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
#include "vt.h"

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

void ExitDD(void)
{
	if (lpDD != NULL)
	{
		if (lpDDOverlay != NULL)
		{
			OverlayUpdate(NULL, NULL, DDOVER_HIDE, FALSE);
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

void Clean_Overlays()
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

BOOL OverlayUpdate(LPRECT pSrcRect, LPRECT pDestRect, DWORD dwFlags, BOOL ColorKey)
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

	dwFlags |= DDOVER_KEYDESTOVERRIDE;
	DDOverlayFX.dckDestColorkey.dwColorSpaceHighValue = RGB(255, 0, 255);
	DDOverlayFX.dckDestColorkey.dwColorSpaceLowValue = RGB(255, 0, 255);

	ddrval = IDirectDrawSurface_UpdateOverlay(lpDDOverlay, pSrcRect, lpDDSurface, pDestRect, dwFlags, &DDOverlayFX);
	if (ddrval != DD_OK)
	{
		ErrorBox("Error calling OverlayUpdate");
		return (FALSE);
	}

	return TRUE;
}

BOOL CreateOverlay()
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
	ddsd.dwBackBufferCount = 1;

	ddsd.ddpfPixelFormat = PixelFormat;
	ddrval = IDirectDraw_CreateSurface(lpDD, &ddsd, &lpDDOverlay, NULL);
	if (FAILED(ddrval))
	{
		ErrorBox("Can't create Overlay Surface");
		lpDDOverlay = NULL;
		return FALSE;
	}

	ddrval = IDirectDrawSurface_Lock(lpDDOverlay, NULL, &ddsd, 0, NULL);
	if (FAILED(ddrval))
	{
		ErrorBox("Can't Lock Surface");
		return (FALSE);
	}
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
		ddrval = IDirectDrawSurface_Lock(lpDDOverlayBack, NULL, &ddsd, 0, NULL);
		if (FAILED(ddrval))
		{
			ErrorBox("Can't Lock Back Surface");
			return (FALSE);
		}
		lpOverlayBack = ddsd.lpSurface;
		ddrval = IDirectDrawSurface_Unlock(lpDDOverlayBack, ddsd.lpSurface);
		if (FAILED(ddrval))
		{
			ErrorBox("Can't Unlock Back Surface");
			return (FALSE);
		}
	}

	return (TRUE);
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

	ddrval = IDirectDrawSurface_Lock(lpDDSurface, NULL, &ddsd, 0, NULL);
	ddrval = IDirectDrawSurface_Unlock(lpDDSurface, ddsd.lpSurface);

	return TRUE;
}
