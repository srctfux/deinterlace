/////////////////////////////////////////////////////////////////////////////
// PlugTest.c
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
// Change Log
//
// Date          Developer             Changes
//
// 10 Apr 2001   John Adcock           Initial Version
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

void memcpyTest(void *Dest, void *Src, size_t nBytes)
{
   memcpy(Dest, Src, nBytes);
}

BOOL FillInfoStruct(DEINTERLACE_INFO* info, char* SnapshotFile)
{
	FILE *file;
   int i = 0;
   int j;

   file = fopen(SnapshotFile,"rb");
	if (!file)
	{
		printf("Could not open file %s\n", SnapshotFile);
		return FALSE;
	}
   if(fread(info, sizeof(DEINTERLACE_INFO), 1, file) != sizeof(DEINTERLACE_INFO))
   {
		printf("Error reading file %s\n", SnapshotFile);
      fclose(file);      
		return FALSE;
   }
   // read in odd fields
   i = 0;
   while(i < MAX_FIELD_HISTORY && info->OddLines[i] != NULL)
   {
      info->OddLines[i] = (short**)malloc(info->FieldHeight * sizeof(short*));
      for(j = 0; j < info->FieldHeight; ++j)
      {
         info->OddLines[i][j] = (short*)malloc(info->LineLength);
         if(fread(info->OddLines[i][j], info->LineLength, 1, file) != info->LineLength)
         {
		      printf("Error reading file %s\n", SnapshotFile);
            fclose(file);      
		      return FALSE;
         }
      }
      i++;
   }

   // read in even fields
   i = 0;
   while(i < MAX_FIELD_HISTORY && info->EvenLines[i] != NULL)
   {
      info->EvenLines[i] = (short**)malloc(info->FieldHeight * sizeof(short*));
      for(j = 0; j < info->FieldHeight; ++j)
      {
         info->EvenLines[i][j] = (short*)malloc(info->LineLength);
         if(fread(info->EvenLines[i][j], info->LineLength, 1, file) != info->LineLength)
         {
		      printf("Error reading file %s\n", SnapshotFile);
            fclose(file);      
		      return FALSE;
         }
      }
      i++;
   }

   info->Overlay = (BYTE*)malloc(info->OverlayPitch * info->FrameHeight);
   info->CpuFeatureFlags = 0;
   info->pMemcpy = memcpyTest;
   fclose(file);      
   return TRUE;
}

void EmptyInfoStruct(DEINTERLACE_INFO* info)
{
   int i, j;

   i = 0;
   while(i < MAX_FIELD_HISTORY && info->OddLines[i] != NULL)
   {
      for(j = 0; j < info->FieldHeight; ++j)
      {
         free(info->OddLines[i][j]);
      }
      free(info->OddLines[i]);
      i++;
   }

   i = 0;
   while(i < MAX_FIELD_HISTORY && info->EvenLines[i] != NULL)
   {
      for(j = 0; j < info->FieldHeight; ++j)
      {
         free(info->EvenLines[i][j]);
      }
      free(info->EvenLines[i]);
      i++;
   }

   free(info->Overlay);
}


BOOL LoadDeintPlugin(LPCSTR szFileName, DEINTERLACE_METHOD* DeintMethod)
{
	GETDEINTERLACEPLUGININFO* pfnGetDeinterlacePluginInfo;
	DEINTERLACE_METHOD* pMethod;
	HMODULE hPlugInMod;

	hPlugInMod = LoadLibrary(szFileName);
	if(hPlugInMod == NULL)
	{
		return FALSE;
	}
	
	pfnGetDeinterlacePluginInfo = (GETDEINTERLACEPLUGININFO*)GetProcAddress(hPlugInMod, "GetDeinterlacePluginInfo");
	if(pfnGetDeinterlacePluginInfo == NULL)
	{
		return FALSE;
	}

	pMethod = pfnGetDeinterlacePluginInfo(0);
	if(pMethod != NULL)
	{
		DeintMethod = pMethod;
		pMethod->hModule = hPlugInMod;
      if(pMethod->pfnPluginStart != NULL)
      {
         pMethod->pfnPluginStart(0, NULL);
      }
      if(pMethod->pfnPluginSwitchTo != NULL)
      {
         pMethod->pfnPluginSwitchTo(NULL, NULL);
      }
	   return TRUE;
	}
   else
   {
   	return FALSE;
   }
}

void UnloadDeintPlugin(DEINTERLACE_METHOD* DeintMethod)
{
   if(DeintMethod->pfnPluginExit != NULL)
   {
      DeintMethod->pfnPluginExit();
   }
   FreeLibrary(DeintMethod->hModule);
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
static void FillTiffHeader(struct TiffHeader *head, char *description, char *make, char *model, DEINTERLACE_INFO* info)
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
	FillTiffDirEntry(&head->width, 256, info->FrameWidth, Short);
	FillTiffDirEntry(&head->height, 257, info->FrameHeight, Short);
	FillTiffDirEntry(&head->compression, 259, 1, Short);					// No compression
	FillTiffDirEntry(&head->photometricInterpretation, 262, 2, Short);		// RGB image data
	FillTiffDirEntry(&head->stripOffset, 273, sizeof(struct TiffHeader), Long);	// Image comes after header
	FillTiffDirEntry(&head->samplesPerPixel, 277, 3, Short);				// RGB = 3 channels/pixel
	FillTiffDirEntry(&head->rowsPerStrip, 278, info->FrameHeight, Short);			// Whole image is one strip
	FillTiffDirEntry(&head->stripByteCounts, 279, info->FrameWidth * info->FrameHeight * 3, Long);	// Size of image data
	FillTiffDirEntry(&head->planarConfiguration, 284, 1, Short);			// RGB bytes are interleaved
}

//-----------------------------------------------------------------------------
// Save still image snapshot as TIFF format to disk
BOOL MakeTifFile(DEINTERLACE_INFO* info, char* TifFile)
{
	int y, cr, cb, r, g, b, i, j, n = 0;
	FILE *file;
	BYTE rgb[3];
	BYTE* buf;
	struct TiffHeader head;
	char description[] = "dTV image";

	file = fopen(TifFile,"wb");
	if (!file)
	{
		printf("Could not open file %s", TifFile);
		return FALSE;
	}

	FillTiffHeader(&head, description, "http://deinterlace.sourceforge.net/", "PlugTest", info);
	fwrite(&head, sizeof(head), 1, file);

	for (i = 0; i < info->FrameHeight; i++)
	{
		buf = (BYTE*)info->Overlay + i * info->OverlayPitch;
		for (j = 0; j < info->FrameWidth ; j+=2)
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
	return TRUE;
}

int ProcessSnapShot(char* SnapshotFile, char* PluginFile, char* TifFile)
{
   DEINTERLACE_INFO info;
   DEINTERLACE_METHOD* DeintMethod = NULL;
   if(!FillInfoStruct(&info, SnapshotFile))
   {
   	return 1;
   }

   if(!LoadDeintPlugin(PluginFile, DeintMethod))
   {
   	return 1;
   }
   
   DeintMethod->pfnAlgorithm(&info);

   if(!MakeTifFile(&info, TifFile))
   {
   	return 1;
   }

   EmptyInfoStruct(&info);
   return 0;
}


int main(int argc, char* argv[])
{
  	printf("PlugTest (c) 2001 John Adcock\n\n");
   printf("PlugTest comes with ABSOLUTELY NO WARRANTY");
   printf("This is free software, and you are welcome");
   printf("to redistribute it under certain conditions.");
   printf("See http://www.gnu.org/copyleft/gpl.html for details.");
   
   if(argc != 4)
   {
   	printf("\nUsage: PlugTest dTVSnapFile PlugInDll OutputTifFile\n");
   	return 1;
   }
	return ProcessSnapShot(argv[1], argv[2], argv[3]);
}

