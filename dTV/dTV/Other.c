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
#include "DebugLog.h"

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
DWORD PhysicalOverlayColor = RGB(255, 0, 255);
int Back_Buffers = 2;		// Make new user parm, TRB 10/28/00

//-----------------------------------------------------------------------------
// Tells whether or not video overlay is active
BOOL OverlayActive()
{
	return (lpDDOverlay != NULL);
}

//-----------------------------------------------------------------------------
// Blank out video overlay
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

//-----------------------------------------------------------------------------
// Update video overlay with new rectangle
BOOL Overlay_Update(LPRECT pSrcRect, LPRECT pDestRect, DWORD dwFlags, BOOL ColorKey)
{
	HRESULT		ddrval;
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
			// 2001-01-06 John Adcock
			// Now show return code
			char szErrorMsg[200];
			sprintf(szErrorMsg, "Error %x calling UpdateOverlay (Hide)", ddrval);
			ErrorBox(szErrorMsg);
		}
		return (TRUE);
	}

	dwFlags |= DDOVER_KEYDESTOVERRIDE;

	PhysicalOverlayColor = Overlay_ColorMatch(lpDDSurface, OverlayColor);
	if (PhysicalOverlayColor == 0)		// sometimes we glitch and can't get the value
	{
		LOG(" Physical overlay color is zero!  Retrying.");
		PhysicalOverlayColor = Overlay_ColorMatch(lpDDSurface, OverlayColor);
	}
	LOG(" Physical overlay color is %x", PhysicalOverlayColor);

	DDOverlayFX.dckDestColorkey.dwColorSpaceHighValue = PhysicalOverlayColor;
	DDOverlayFX.dckDestColorkey.dwColorSpaceLowValue = PhysicalOverlayColor;

	ddrval = IDirectDrawSurface_UpdateOverlay(lpDDOverlay, pSrcRect, lpDDSurface, pDestRect, dwFlags, &DDOverlayFX);
	if (ddrval != DD_OK)
	{
		if ((pDestRect->top < pDestRect->bottom) && (pDestRect->left < pDestRect->right))
		{
			// 2000-10-29 Added by Mark Rejhon
			// Display error message only if rectangle dimensions are positive.
			// Negative rectangle dimensions are frequently caused by the user
			// resizing the window smaller than the video size.
			// 2001-01-06 John Adcock
			// Now show return code
			char szErrorMsg[200];
			sprintf(szErrorMsg, "Error %x calling UpdateOverlay (Show)", ddrval);
			ErrorBox(szErrorMsg);
		}
		lpDDOverlay = NULL;
		return (FALSE);
	}

	return TRUE;
}

//-----------------------------------------------------------------------------
// Create new video overlay
BOOL Overlay_Create()
{
	DDSURFACEDESC ddsd;
	DDPIXELFORMAT PixelFormat;
	HRESULT ddrval;
	DDSCAPS caps;

	if (lpDDOverlay) 
	{
		return FALSE;
	}

	// 2000-10-31 Moved by Mark Rejhon
	// Attempt to create primary surface before overlay, in this module,
	// because we may have destroyed the primary surface during a computer 
	// resolution change.
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

	memset(&PixelFormat, 0x00, sizeof(PixelFormat));
	PixelFormat.dwSize = sizeof(DDPIXELFORMAT);
	PixelFormat.dwFlags = DDPF_FOURCC;
	PixelFormat.dwFourCC = MAKEFOURCC('Y', 'U', 'Y', '2');;
	PixelFormat.dwYUVBitCount = 16;

	memset(&ddsd, 0x00, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT | DDSD_BACKBUFFERCOUNT;
	ddsd.ddsCaps.dwCaps = DDSCAPS_OVERLAY | DDSCAPS_VIDEOMEMORY | DDSCAPS_FLIP | DDSCAPS_COMPLEX;

	// create a surface big enough to hold the largest resolution supported
	// this ensures that we can always have enough space to allow
	// mode changes without recreating the overlay
	ddsd.dwWidth = DTV_MAX_WIDTH;
	ddsd.dwHeight = DTV_MAX_HEIGHT;
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

//-----------------------------------------------------------------------------
// Deinitialize video overlay
//
// 2000-10-31 Added by Mark Rejhon
// Provide a way to destroy the video overlay and primary, which should
// be done right before a computer resolution change.
// 
BOOL Overlay_Destroy()
{
	if (lpDD != NULL)
	{
		if (lpDDOverlay != NULL)
		{
			// Destroy the video overlay
			Overlay_Update(NULL, NULL, DDOVER_HIDE, FALSE);
			IDirectDrawSurface_Release(lpDDOverlay);
			lpDDOverlay = NULL;
			lpDDOverlayBack = NULL;

			// Now destroy the primary surface
			if (lpDDSurface != NULL) 
			{
				IDirectDrawSurface_Release(lpDDSurface);
				lpDDSurface = NULL;
			}
			return TRUE;
		}
	}
	return FALSE;
}

//-----------------------------------------------------------------------------
// Initialize DirectDraw
BOOL InitDD(HWND hWnd)
{
	HRESULT ddrval;
	DDCAPS DriverCaps;

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
/*
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
*/
	return TRUE;
}

//-----------------------------------------------------------------------------
// Deinitialize DirectDraw
void ExitDD(void)
{
	if (lpDD != NULL)
	{
		Overlay_Destroy();
		IDirectDraw_Release(lpDD);
		lpDD = NULL;
	}
}

#define LIMIT(x) (((x)<0)?0:((x)>255)?255:(x))
#pragma pack(1)

// A TIFF image-file directory entry.  There are a bunch of
// these in a TIFF file.
struct TiffDirEntry {
	WORD tag;		// Entry type
	WORD type;		// 1=byte, 2=C string, 3=word, 4=dword (we always use dword)
	DWORD count;	// Number of units (of type specified by "type") in value
	DWORD value;
};

// Field data types.
enum TiffDataType {
	Byte = 1,
	String = 2,
	Short = 3,
	Long = 4
};

// A TIFF header with some hardwired fields.
struct TiffHeader {
	char byteOrder[2];
	WORD version;
	DWORD firstDirOffset;

	// TIFF files contain a bunch of extra information, each of which is a
	// tagged "directory" entry.  The entries must be in ascending numerical
	// order.

	WORD numDirEntries;
	struct TiffDirEntry fileType;		// What kind of file this is (tag 254)
	struct TiffDirEntry width;			// Width of image (tag 256)
	struct TiffDirEntry height;			// Height of image (tag 257)
	struct TiffDirEntry bitsPerSample;	// Number of bits per channel per pixel (tag 258)
	struct TiffDirEntry compression;	// Compression settings (tag 259)
	struct TiffDirEntry photometricInterpretation; // What kind of pixel data this is (tag 262)
	struct TiffDirEntry description;	// Image description (tag 270)
	struct TiffDirEntry make;			// "Scanner" maker, aka dTV's URL (tag 271)
	struct TiffDirEntry model;			// "Scanner" model, aka dTV version (tag 272)
	struct TiffDirEntry stripOffset;	// Offset to image data (tag 273)
	struct TiffDirEntry samplesPerPixel; // Number of color channels (tag 277)
	struct TiffDirEntry rowsPerStrip;	// Number of rows in a strip (tag 278)
	struct TiffDirEntry stripByteCounts; // Number of bytes per strip (tag 279)
	struct TiffDirEntry planarConfiguration; // Are channels interleaved? (tag 284)
	DWORD nextDirOffset;

	// We store a few strings in the file; include them in the structure so
	// it's easy to compute their offsets.  Yeah, this wastes a bit of disk
	// space, but an insignificant percentage of the overall file size.
	char descriptionText[80];
	char makeText[40];
	char modelText[16];
	WORD bitCounts[3];
};

#define STRUCT_OFFSET(s,f)  ((int)(((BYTE *) &(s)->f) - (BYTE *)(s)))


//-----------------------------------------------------------------------------
// Fill a TIFF directory entry with information.
static void FillTiffDirEntry(struct TiffDirEntry *entry, WORD tag, DWORD value, enum TiffDataType type)
{
	BYTE bValue;
	WORD wValue;

	entry->tag = tag;
	entry->count = 1;
	entry->type = (int) type;

	switch (type) {
	case Byte:
		bValue = (BYTE) value;
		memcpy(&entry->value, &bValue, 1);
		break;

	case Short:
		wValue = (WORD) value;
		memcpy(&entry->value, &wValue, 2);
		break;

	case String:	// in which case it's a file offset
	case Long:
		entry->value = value;
		break;
	}
}


//-----------------------------------------------------------------------------
// Fill a TIFF header with information about the current image.
static void FillTiffHeader(struct TiffHeader *head, char *description, char *make, char *model)
{
	memset(head, 0, sizeof(struct TiffHeader));

	strcpy(head->byteOrder, "II");		// Intel byte order
	head->version = 42;					// We're TIFF 5.0 compliant, but the version field is unused
	head->firstDirOffset = STRUCT_OFFSET(head, numDirEntries);
	head->numDirEntries = 14;
	head->nextDirOffset = 0;			// No additional directories

	strcpy(head->descriptionText, description);
	strcpy(head->makeText, make);
	strcpy(head->modelText, model);
	head->bitCounts[0] = head->bitCounts[1] = head->bitCounts[2] = 8;

	head->description.tag = 270;
	head->description.type = 2;
	head->description.count = strlen(description) + 1;
	head->description.value = STRUCT_OFFSET(head, descriptionText);

	head->make.tag = 271;
	head->make.type = 2;
	head->make.count = strlen(make) + 1;
	head->make.value = STRUCT_OFFSET(head, makeText);

	head->model.tag = 272;
	head->model.type = 2;
	head->model.count = strlen(model) + 1;
	head->model.value = STRUCT_OFFSET(head, modelText);
	
	head->bitsPerSample.tag = 258;
	head->bitsPerSample.type = Short;
	head->bitsPerSample.count = 3;
	head->bitsPerSample.value = STRUCT_OFFSET(head, bitCounts);

	FillTiffDirEntry(&head->fileType, 254, 0, Long);						// Just the image, no thumbnails
	FillTiffDirEntry(&head->width, 256, CurrentX, Short);
	FillTiffDirEntry(&head->height, 257, CurrentY, Short);
	FillTiffDirEntry(&head->compression, 259, 1, Short);					// No compression
	FillTiffDirEntry(&head->photometricInterpretation, 262, 2, Short);		// RGB image data
	FillTiffDirEntry(&head->stripOffset, 273, sizeof(struct TiffHeader), Long);	// Image comes after header
	FillTiffDirEntry(&head->samplesPerPixel, 277, 3, Short);				// RGB = 3 channels/pixel
	FillTiffDirEntry(&head->rowsPerStrip, 278, CurrentY, Short);			// Whole image is one strip
	FillTiffDirEntry(&head->stripByteCounts, 279, CurrentX * CurrentY * 3, Long);	// Size of image data
	FillTiffDirEntry(&head->planarConfiguration, 284, 1, Short);			// RGB bytes are interleaved
}

//-----------------------------------------------------------------------------
// Save still image snapshot as TIFF format to disk
void SaveStill()
{
	int y, cr, cb, r, g, b, i, j, n = 0;
	FILE *file;
	BYTE rgb[3];
	BYTE* buf;
	char name[13];
	struct stat st;
	struct TiffHeader head;
	DDSURFACEDESC ddsd;
	HRESULT ddrval;
	char description[80];

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
			sprintf(name,"tv%06d.tif",++n) ;
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

		sprintf(description, "dTV image, deinterlace mode %s", DeinterlaceModeName(-1));
		// How do we figure out our version number?!?!
		FillTiffHeader(&head, description, "http://deinterlace.sourceforge.net/", "dTV version 2.x");
		fwrite(&head, sizeof(head), 1, file);

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
