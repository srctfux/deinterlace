// $Id: DI_GreedyHM.h,v 1.5 2001-08-19 06:26:38 trbarry Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 Tom Barry.  All rights reserved.
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
// 01 Jul 2001   Tom Barry		       Added GreedyH Deinterlace method
//
/////////////////////////////////////////////////////////////////////////////
//
// This member contains the meat of the Greedy (High Motion) deinterlace method
// It is written to not be particularly dependend upon either DScaler or Windows.
// It would be nice to keep it that way if possible as I'd like to also use it to
// port to other environments including maybe Linux, DirectShow filters, batch utilites,
// and maybe VirtualDub or TMPGEnc plug-ins.
//
// I'll add a bigger block of comments here from material I'll post on the list. Basically this
// was made from ideas used in the Blended Clip & Greedy (Low Motion) plug-in's.
//
// Then Edge Enhancement, Median Filtering, Vertical Filtering, Diagonal Jaggie Reduction (DJR ;-) ), 
// n:n pulldown matching, and In-Between Frames were built on that.
//
// !!!  THIS REQUIRES A FAST SSE BOX (Celeron, Athlon, P-III, or P4. !!!
// It will just execute a copy of the old Greedy (Low Motion) if that is not present.
//
//////////////////////////////////////////////////////////////////////////////

extern long GreedyMaxComb;				// max comb we allow past clip
extern long GreedyMotionThreshold;		// ignore changes < this
extern long GreedyMotionSense;			// how rapidly to bob when > threshold
extern long GreedyGoodPullDownLvl;		// Best comb avg / comb avg must be < this for PD
extern long GreedyBadPullDownLvl;		// don't pulldown this field if comb / best avg comb > this 
extern long GreedyEdgeEnhAmt;			// % sharpness to add				
extern long GreedyMedianFilterAmt;		// Don't filter if > this
extern long GreedyLowMotionPdLvl;		// Do pulldown for low motion frames < this

extern BOOL GreedyUsePulldown;			
extern BOOL GreedyUseInBetween;
extern BOOL GreedyUseMedianFilter;
extern BOOL GreedyUseVertFilter;
extern BOOL GreedyUseEdgeEnh;
extern BOOL GreedySSEBox;           
extern UINT GreedyFeatureFlags;         // Save feature flags on setup
typedef struct 
{
    int Comb;					// combs
    int CombChoice;				// val chosen by Greedy Choice
    int Kontrast;				// sum of all abs vertical diff in a field
    int Motion;					// sum of all abs vertical diff in a field
    int Avg;					// avg of last 10 combs (actually just a total)
    int AvgChoice;				// avgs of last 10 chosen combs (actually just a total)
    int Flags;					// a circlular history of last 20 Greedy choice flags
    int Flags2;					// various status flags, mostly for debugging
} GR_PULLDOWN_INFO;

#define PD_VIDEO  1			    // did video deinterlace for this frame
#define PD_PULLDOWN  1 << 1     // did pulldown
#define PD_BAD  1 << 2		    // bad pulldown situation
#define PD_LOW_MOTION  1 << 3   // did pulldown due to low motion
#define PD_MERGED  1 << 4       // made an in between frame
#define PD_32_PULLDOWN  1 << 5  // is 3:2 pulldown
#define PD_22_PULLDOWN 1 << 6   // is 2:2 pulldown
#define PD_ODD        1 << 7    // is Odd Field

// A bunch of shared variables used by all the very similar routines
#define FSFIELDS 4		// number of fields to buffer
#define FSMAXROWS 289	// allow space for max 288 rows/field, plus a spare
#define FSMAXCOLS 1000	// allow space for max 1000 screen cols
#define FSCOLCT FSMAXCOLS * FSFIELDS / 4 // number qwords in row = cols * 4 fields / 4 pixels
#define FSCOLSIZE 32	// bytes to skip for info for next col (of 4 fields and 4 pixels)
#define FSROWSIZE FSMAXCOLS*FSCOLSIZE/4  // bytes to skip to get to info for 2nd row
#define FSSIZE FSFIELDS * FSMAXCOLS * FSMAXROWS / 4   // number qwords in FieldStore array

// Parm data captured from DSCALER info on call
extern short **pLines;					// current input lines, either even or odd
extern short **pOddLines;
extern short **pEvenLines;
extern short **pPrevLines;
extern int	FieldHeight;
extern int	FrameHeight;
extern int LineLength;
extern int OverlayPitch;	
extern BOOL InfoIsOdd;
extern BYTE *lpCurOverlay;

extern __int64 MaxComb;
extern __int64 EdgeThreshold;
extern __int64 EdgeSense;
extern __int64 MedianFilterAmt;
extern __int64 EdgeEnhAmt;
extern __int64 MotionThreshold;
extern __int64 MotionSense;
extern int FsPtr;			// current subscript in FieldStore
extern int FsPtrP;			// prev subscript int FieldStore
extern int FsPtrP2;			// 2 back
extern int FsPtrP3;			// 3 back
extern int FsDelay;				// display is delayed by n fields (1,2,3)
extern __int64 FieldStore[4*FSMAXCOLS*FSMAXROWS/4];
extern __int64* lpFieldStore;

// typedef void (MEMCPY_FUNC)(void* pOutput, void* pInput, size_t nSize);
extern MEMCPY_FUNC* pMemcpy;
extern BOOL DI_GreedyHM();
BOOL FieldStoreCopy(BYTE * dest, __int64 * src, int clen);

// return FS subscripts depending on FsDelay - Note args by reference
BOOL SetFsPtrs(int* L1, int* L2, int* L2P, int* L3, int* CopySrc, BYTE** CopyDest, BYTE** WeaveDest);
BOOL DI_GreedyHF_SSE();   								// fast single pass deint with no options
BOOL DI_GreedyHF_3DNOW();   							// same for 3DNOW
BOOL DI_GreedyHF_MMX();   								// same for MMX
BOOL DI_GreedyHM_NV();									// full deint with no Vertical Filter
BOOL DI_GreedyHM_V();									// full deint with Vertical Filter

#define PDAVGLEN 10									// len of pulldown average, < len of queue
int UpdatePulldown(int Comb, int Kontrast, int Motion);						
BOOL CanDoPulldown();									// check if we should do pulldown, doit
BOOL GetHistData(GR_PULLDOWN_INFO * OHist, int ct);


// Define a few macros for CPU dependent instructions.
// I suspect I don't really understand how the C macro preprocessor works but
// this seems to get the job done.          // TRB 7/01

// BEFORE USING THESE YOU MUST SET:

// #define SSE_TYPE SSE            (or MMX or 3DNOW)

// some macros for pavgb instruction
//      V_PAVGB(mmr1, mmr2, mmr work register, smask) mmr2 may = mmrw if you can trash it

#define V_PAVGB_MMX(mmr1,mmr2,mmrw,smask) __asm \
	{ \
	__asm movq mmrw,mmr2 \
	__asm pand mmrw, smask \
	__asm psrlw mmrw,1 \
	__asm pand mmr1,smask \
	__asm psrlw mmr1,1 \
	__asm paddusb mmr1,mmrw \
	}

#define V_PAVGB_SSE(mmr1,mmr2,mmrw,smask) {pavgb mmr1,mmr2 }
#define V_PAVGB_3DNOW(mmr1,mmr2,mmrw,smask) {pavgusb mmr1,mmr2 }
#define V_PAVGB(mmr1,mmr2,mmrw,smask) V_PAVGB2(mmr1,mmr2,mmrw,smask,SSE_TYPE) 
#define V_PAVGB2(mmr1,mmr2,mmrw,smask,ssetyp) V_PAVGB3(mmr1,mmr2,mmrw,smask,ssetyp) 
#define V_PAVGB3(mmr1,mmr2,mmrw,smask,ssetyp) V_PAVGB_##ssetyp##(mmr1,mmr2,mmrw,smask) 

// some macros for pmaxub instruction
//      V_PMAXUB(mmr1, mmr2)    
#define V_PMAXUB_MMX(mmr1,mmr2)     __asm \
	{ \
    __asm psubusb mmr1,mmr2 \
    __asm paddusb mmr1,mmr2 \
    }

#define V_PMAXUB_SSE(mmr1,mmr2) {pmaxub mmr1,mmr2 }
#define V_PMAXUB_3DNOW(mmr1,mmr2) V_PMAXUB_MMX(mmr1,mmr2)  // use MMX version
#define V_PMAXUB(mmr1,mmr2) V_PMAXUB2(mmr1,mmr2,SSE_TYPE) 
#define V_PMAXUB2(mmr1,mmr2,ssetyp) V_PMAXUB3(mmr1,mmr2,ssetyp) 
#define V_PMAXUB3(mmr1,mmr2,ssetyp) V_PMAXUB_##ssetyp##(mmr1,mmr2) 

// some macros for pminub instruction
//      V_PMINUB(mmr1, mmr2, mmr work register)     mmr2 may NOT = mmrw
#define V_PMINUB_MMX(mmr1,mmr2,mmrw) __asm \
	{ \
    __asm pcmpeqb mmrw,mmrw     \
    __asm psubusb mmrw,mmr2     \
    __asm paddusb mmr1, mmrw     \
    __asm psubusb mmr1, mmrw     \
	}

#define V_PMINUB_SSE(mmr1,mmr2,mmrw) {pminub mmr1,mmr2}
#define V_PMINUB_3DNOW(mmr1,mmr2,mmrw) V_PMINUB_MMX(mmr1,mmr2,mmrw)  // use MMX version
#define V_PMINUB(mmr1,mmr2,mmrw) V_PMINUB2(mmr1,mmr2,mmrw,SSE_TYPE) 
#define V_PMINUB2(mmr1,mmr2,mmrw,ssetyp) V_PMINUB3(mmr1,mmr2,mmrw,ssetyp) 
#define V_PMINUB3(mmr1,mmr2,mmrw,ssetyp) V_PMINUB_##ssetyp##(mmr1,mmr2,mmrw) 

// some macros for movntq instruction
//      V_MOVNTQ(mmr1, mmr2) 
#define V_MOVNTQ_MMX(mmr1,mmr2) {movq mmr1,mmr2}
#define V_MOVNTQ_3DNOW(mmr1,mmr2) {movq mmr1,mmr2 }
#define V_MOVNTQ_SSE(mmr1,mmr2) {movntq mmr1,mmr2 }
#define V_MOVNTQ(mmr1,mmr2) V_MOVNTQ2(mmr1,mmr2,SSE_TYPE) 
#define V_MOVNTQ2(mmr1,mmr2,ssetyp) V_MOVNTQ3(mmr1,mmr2,ssetyp) 
#define V_MOVNTQ3(mmr1,mmr2,ssetyp) V_MOVNTQ_##ssetyp##(mmr1,mmr2)

// end of macros
