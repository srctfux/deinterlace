/////////////////////////////////////////////////////////////////////////////
// deinterlace.c
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
// 24 Jul 2000   John Adcock           Put all my deinterlacing code into this
//                                     file
//
// 09 Nov 2000   Tom Barry		       Added Blended Clipping Deinterlace method
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "deinterlace.h"
#include "globals.h"

long BitShift = 13;
long EdgeDetect = 625;
long JaggieThreshold = 73;
long DiffThreshold = 224;
long TemporalTolerance = 300;
long SpatialTolerance = 600;
long SimilarityThreshold = 25;

/////////////////////////////////////////////////////////////////////////////
// memcpyMMX
// Uses MMX instructions to move memory around
// does as much as we can in 64 byte chunks
// using MMX instructions
// then copies any extra bytes
// assumes there will be at least 64 bytes to copy
// This code was originally from Borg's bTV plugin SDK 
/////////////////////////////////////////////////////////////////////////////
void memcpyMMX(void *Dest, void *Src, size_t nBytes)
{
	size_t nCharsLeft = nBytes & 0x3F;
	__asm
	{
		mov		esi, dword ptr[Src]
		mov		edi, dword ptr[Dest]
		mov		ecx, nBytes
		shr     ecx, 6                      // nBytes / 64
align 8
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
		add		esi, 64
		add		edi, 64
		loop CopyLoop
		mov		ecx, nBytes
		and     ecx, 63
		cmp     ecx, 0
		je EndCopyLoop
align 8
CopyLoop2:
		mov dl, byte ptr[esi] 
		mov byte ptr[edi], dl
		inc esi
		inc edi
		dec ecx
		jne near CopyLoop2
EndCopyLoop:
		emms
	}
}

/////////////////////////////////////////////////////////////////////////////
// memcpyBOBMMX
// Uses MMX instructions to move memory around to two places
// does as much as we can in 64 byte chunks
// using MMX instructions
// then copies any extra bytes
// Dest must be the upper of the two lines
// assumes there will be at least 64 bytes to copy
/////////////////////////////////////////////////////////////////////////////
void memcpyBOBMMX(void *Dest, void *Src, size_t nBytes)
{
	__asm
	{
		mov		esi, dword ptr[Src]
		mov		edi, dword ptr[Dest]
		mov     ebx, edi
		add     ebx, OverlayPitch
		mov		ecx, nBytes
		shr     ecx, 6                      // nBytes / 64
align 8
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
		movq	qword ptr[ebx], mm0
		movq	qword ptr[ebx+8*1], mm1
		movq	qword ptr[ebx+8*2], mm2
		movq	qword ptr[ebx+8*3], mm3
		movq	qword ptr[ebx+8*4], mm4
		movq	qword ptr[ebx+8*5], mm5
		movq	qword ptr[ebx+8*6], mm6
		movq	qword ptr[ebx+8*7], mm7
		add		esi, 64
		add		edi, 64
		add		ebx, 64
		dec ecx
		jne near CopyLoop

		mov		ecx, nBytes
		and     ecx, 63
		cmp     ecx, 0
		je EndCopyLoop
align 8
CopyLoop2:
		mov dl, byte ptr[esi] 
		mov byte ptr[edi], dl
		mov byte ptr[ebx], dl
		inc esi
		inc edi
		inc ebx
		dec ecx
		jne near CopyLoop2
EndCopyLoop:
		emms
	}
}

/////////////////////////////////////////////////////////////////////////////
// VideoDeinterlaceMMX
// First go at this
// this one just averages between the incoming line and the
// average between the two surrounding lines
// this should reduce weave aftifacts while allowing
// some detail through
// this function copies two lines to the destination
// uses MMX instructions
// assumes there will be at least 32 bytes to copy
// Dest: This is the pointer to the middle line
// The algoritm comes from Borg's bTV deinterlace plug-in
// with key1 enabled
/////////////////////////////////////////////////////////////////////////////
void VideoDeinterlaceMMX(void *Dest, void *SrcUp, void *SrcSame, void *SrcDown, size_t nBytes)
{
	const __int64 Mask = 0x7f7f7f7f7f7f7f7f;
	__asm
	{
		mov		esi, dword ptr[SrcUp]
		mov		edx, dword ptr[SrcDown]
		mov		ebx, dword ptr[SrcSame]
		mov		edi, dword ptr[Dest]
		mov		ecx, nBytes
		shr     ecx, 5                      // nBytes / 32
align 8
CopyLoop:
		// read in the upper source line data
		movq  mm0, qword ptr [esi]
		movq  mm1, qword ptr [esi + 8]
		movq  mm2, qword ptr [esi + 16]
		movq  mm3, qword ptr [esi + 24]
		// read in the lower source line data
		movq  mm4, qword ptr [edx]
		movq  mm5, qword ptr [edx + 8]
		movq  mm6, qword ptr [edx + 16]
		movq  mm7, qword ptr [edx + 24]
		// move destination for the source line and
		// just put that straight in
		add edi, OverlayPitch
		movq  qword ptr [edi], mm4
		movq  qword ptr [edi + 8], mm5
		movq  qword ptr [edi + 16], mm6
		movq  qword ptr [edi + 24], mm7
		sub edi, OverlayPitch
		// divide all data bytes by 2
		// we do this by shifting all bytes
		// left one and anding to 127
		psrlw mm0, 01
		psrlw mm1, 01
		psrlw mm2, 01
		psrlw mm3, 01
		psrlw mm4, 01
		psrlw mm5, 01
		psrlw mm6, 01
		psrlw mm7, 01
		pand  mm0, Mask
		pand  mm1, Mask
		pand  mm2, Mask
		pand  mm3, Mask
		pand  mm4, Mask
		pand  mm5, Mask
		pand  mm6, Mask
		pand  mm7, Mask
		// work out average
		// of upper and lower
		paddw  mm0, mm4
		paddw  mm1, mm5
		paddw  mm2, mm6
		paddw  mm3, mm7
		// get middle line
		movq  mm4, qword ptr [ebx]
		movq  mm5, qword ptr [ebx + 8]
		movq  mm6, qword ptr [ebx + 16]
		movq  mm7, qword ptr [ebx + 24]
		// divide average and middle line by 2
		psrlw mm0, 01
		psrlw mm1, 01
		psrlw mm2, 01
		psrlw mm3, 01
		psrlw mm4, 01
		psrlw mm5, 01
		psrlw mm6, 01
		psrlw mm7, 01
		pand  mm0, Mask
		pand  mm1, Mask
		pand  mm2, Mask
		pand  mm3, Mask
		pand  mm4, Mask
		pand  mm5, Mask
		pand  mm6, Mask
		pand  mm7, Mask
		// add to give average of the middle line 
		// and the average of upper and lower lines
		paddw  mm0, mm4
		paddw  mm1, mm5
		paddw  mm2, mm6
		paddw  mm3, mm7
		// copy the result to the output buffer
		movq  qword ptr [edi], mm0
		movq  qword ptr [edi + 8], mm1
		movq  qword ptr [edi + 16], mm2
		movq  qword ptr [edi + 24], mm3
		// move to next set of 4 qwords
		add esi, 32
		add edi, 32
		add edx, 32
		add ebx, 32
		dec ecx
		jne near CopyLoop

		// see if we have any extra bytes to do
		mov		ecx, nBytes
		and     ecx, 31
		shr     ecx, 2                      // nBytes / 4
		cmp     ecx, 0
		je EndCopyLoop
align 8
CopyLoop2:
		// if we do have some left over then
		// do the same as before but only 4 bytes at a time
		movq  mm0, qword ptr [esi]
		movq  mm4, qword ptr [edx]
		psrlw mm0, 01
		psrlw mm4, 01
		pand  mm0, Mask
		pand  mm4, Mask
		paddw  mm0, mm4
		movq  mm4, qword ptr [ebx]
		psrlw mm0, 01
		psrlw mm4, 01
		pand  mm0, Mask
		pand  mm4, Mask
		paddw  mm0, mm4
		movq  dword ptr [edi], mm0
		add esi, 4
		add edi, 4
		add edx, 4
		add ebx, 4
		dec ecx
		jne near CopyLoop2
EndCopyLoop:
		emms
	}
}


///////////////////////////////////////////////////////////////////////////////
// DeinterlaceFieldBob
//
// Deinterlaces a field with a tendency to bob rather than weave.  Best for
// high-motion scenes like sports.
//
// The algorithm for this was taken from the 
// Deinterlace - area based Vitual Dub Plug-in by
// Gunnar Thalin
///////////////////////////////////////////////////////////////////////////////
void DeinterlaceFieldBob(short** pOddLines, short** pEvenLines, short** pPrevLines, BYTE* lpCurOverlay, BOOL bIsOdd)
{
	int Line;
	short* YVal1;
	short* YVal2;
	short* YVal3;
	BYTE* Dest;
	
	const __int64 YMask    = 0x00ff00ff00ff00ff;
	const __int64 UVMask    = 0xff00ff00ff00ff00;

	__int64 qwEdgeDetect;
	__int64 qwThreshold;
	const __int64 Mask = 0x7f7f7f7f7f7f7f7f;

	pPrevLines = NULL;	// stop the compiler from whining
	qwEdgeDetect = EdgeDetect;
	qwEdgeDetect += (qwEdgeDetect << 48) + (qwEdgeDetect << 32) + (qwEdgeDetect << 16);
	qwThreshold = JaggieThreshold;
	qwThreshold += (qwThreshold << 48) + (qwThreshold << 32) + (qwThreshold << 16);


	// copy first even line no matter what, and the first odd line if we're
	// processing an odd field.
	memcpyMMX(lpCurOverlay, pEvenLines[0], CurrentX * 2);
	if (bIsOdd)
		memcpyMMX(lpCurOverlay + OverlayPitch, pOddLines[0], CurrentX * 2);

	for (Line = 0; Line < (CurrentY / 2 - 1); ++Line)
	{
		if (bIsOdd)
		{
			YVal1 = pOddLines[Line];
			YVal2 = pEvenLines[Line + 1];
			YVal3 = pOddLines[Line + 1];
			Dest = lpCurOverlay + (Line * 2 + 2) * OverlayPitch;
		}
		else
		{
			YVal1 = pEvenLines[Line];
			YVal2 = pOddLines[Line];
			YVal3 = pEvenLines[Line + 1];
			Dest = lpCurOverlay + (Line * 2 + 1) * OverlayPitch;
		}

		// For ease of reading, the comments below assume that we're operating on an odd
		// field (i.e., that bIsOdd is true).  The exact same processing is done when we
		// operate on an even field, but the roles of the odd and even fields are reversed.
		// It's just too cumbersome to explain the algorithm in terms of "the next odd
		// line if we're doing an odd field, or the next even line if we're doing an
		// even field" etc.  So wherever you see "odd" or "even" below, keep in mind that
		// half the time this function is called, those words' meanings will invert.

		// Copy the odd line to the overlay verbatim.
		memcpyMMX(Dest + OverlayPitch, YVal3, CurrentX * 2);

		_asm
		{
			mov ecx, CurrentX
			mov eax, dword ptr [YVal1]
			mov ebx, dword ptr [YVal2]
			mov edx, dword ptr [YVal3]
			mov edi, dword ptr [Dest]
			shr ecx, 2       // there are CurrentX * 2 / 8 qwords

align 8
DoNext8Bytes:			
			movq mm0, qword ptr[eax] 
			movq mm1, qword ptr[ebx] 
			movq mm2, qword ptr[edx]

			// get intensities in mm3 - 4
			movq mm3, mm0
			movq mm4, mm1
			movq mm5, mm2

			pand mm3, YMask
			pand mm4, YMask
			pand mm5, YMask

			// get average in mm0
			psrlw mm0, 01
			psrlw mm2, 01
			pand  mm0, Mask
			pand  mm2, Mask
			paddw mm0, mm2

			// work out (O1 - E) * (O2 - E) / 2 - EdgeDetect * (O1 - O2) ^ 2 >> 12
			// result will be in mm6

			psrlw mm3, 01
			psrlw mm4, 01
			psrlw mm5, 01

			movq mm6, mm3
			psubw mm6, mm4	//mm6 = O1 - E

			movq mm7, mm5
			psubw mm7, mm4	//mm7 = O2 - E

			pmullw mm6, mm7		// mm0 = (O1 - E) * (O2 - E)

			movq mm7, mm3
			psubw mm7, mm5		// mm7 = (O1 - O2)
			pmullw mm7, mm7		// mm7 = (O1 - O2) ^ 2
			psrlw mm7, 12		// mm7 = (O1 - O2) ^ 2 >> 12
			pmullw mm7, qwEdgeDetect		// mm7  = EdgeDetect * (O1 - O2) ^ 2 >> 12

			psubw mm6, mm7      // mm6 is what we want

			pcmpgtw mm6, qwThreshold

			movq mm7, mm6

			pand mm0, mm6

			pandn mm7, mm1

			por mm7, mm0

			movq qword ptr[edi], mm7

			add eax, 8
			add ebx, 8
			add edx, 8
			add edi, 8
			dec ecx
			jne near DoNext8Bytes
			emms
		}
	}

	// Copy last odd line if we're processing an even field.
	if (! bIsOdd)
		memcpyMMX(lpCurOverlay + (CurrentY - 1) * OverlayPitch, pOddLines[CurrentY / 2 - 1], CurrentX * 2);
}


///////////////////////////////////////////////////////////////////////////////////
// Blended Clipping Deinterlace - Tom Barry 11/09/2000
///////////////////////////////////////////////////////////////////////////////////
// As written, Blended Clipping Deinterlace method is really designed as a testing
// laboratory.  It uses a huge confusing control panel and it has way too many
// controls to be user friendly as a final product.  It does, however, allow
// us to easily experiment with what might work best.  It may be able to provide an 
// example of some of the things that might be possible if we find a few sets of 
// parms that work well for various situations.  It can create both some decent results
// and also some very bad results depending upon both the input video and the
// parameter settings.

// Note that while I've attempted to optimize the assembler code, this method still
// eats a fair amount of CPU time.  In order to run it on my 350 or 450 mhz machines
// I must have all the dTV performance options turned on.

// How it works:

// Deinterlace algorithms face a choice between using the most recently received
// information which is accurate in time and using older field information which
// may be more accurate in space.  Adaptive algorithms make this choice on the fly
// for each field and/or pixel. Blended algorithms try to smoothly blend two or more  
// choices together, avoiding rapidly blinking pixels and some other artifacts. The
// Blended Clipping algorithm attempts to do all of the above.
//
// Assume we are processing an odd field.  Like most other simpler deinterlacing 
// methods, the Blended Clipping method will just use the current field info unchanged
// for the odd lines.  Odd lines are just copied to the video buffer for now though
// this too might be improved in the future.
//
// For the even lines we will create a blend of the Weave pixels (the previous even lines)
// and a Clipped Weave, which is itself a blend.
//
// The Clipped Weave is just the weave pixel unless it's luma (and optionally chroma) is
// outside the high/low range of its vertically adjacent pixels.  If so, the luma (chroma)
// values are clipped to the nearest adjacent value.  This is intended to be a 
// replacement for the various Interpolated BOB implementatons that usually represent
// the current time estimated value for the even lines.  To see the pure value of a
// Clipped Weave, without any other blending, just max out the "Minimum Clip" slider
// on the contol panel.  It still creates some BOB-like artifacts but they are less
// visible and don't seem to have the rapid up/down jitter of BOB implementations.

// Exactly what percentage of the Weave vs. Clipped Weave we use depends upon a number
// number of factors, each controlled by a slider in the Blended Clipping control panel.
// There are also INI parms for these values.  (currentlly 9 sliders, 2 check boxes)

// Most of the parameters should be thought of as unitless fuzzy preference values
// in the range of 0..100. The INI file parm names will be in the [deinterace] section
// will be the same as the global definitions.

// The slider names and descriptions follow.  The INI file/global names follow 
// each description:

// "Minimum Clip" slider: This can be uses to increase the amount of Clip vs Weave
// in the event that none of the other more specific values seem to work and you
// still see too many Weave artifacts ("venetion blinds").  It's best to try everything
// else first.

	int		BlcMinimumClip = -15;				// currently -100 .. 100

// "Pixel Motion Sensitivity" slider:  This determines how sensitive we are to motion.
// Motion is calculated as the maximum absolute change in luma from the previous field
// in either of the two vertically adjacent pixels.  I was going to use the change in 
// luma of the Weave pixel but that is not current enough to prevent the "flash attacks"
// of venetiaon blinds that can occur with sudden scene changes.  This value is
// calculated separately for each pixel.

	UINT	BlcPixelMotionSense = 17;

// "Recent Motion Sensitivity" slider:  This increases the tendency to use Clip based
// upon an n-period Exponential Moving Average of the recent motion.  Recent motion
// is in turn an arithmetic average of how much each pixel's luma has changed from the
// previous field.  These values are self obtained and maintained.  While this method
// does not attempt to do 3:2 pulldown I believe the motion values could be of assistance
// in the routines that do.  

	int		BlcRecentMotionSense = 0;		// current -100 .. 100)		

// "Motion Average Period" slider:  This sets the period of the moving average for Recent
// Motion Sensitivity.  

// For those of you not recently programming for the stock market:

//		An Exponential Moving Average is a way to keep and update an average without 
//		keeping around a history of all the recent events.  In the stock market,
//		an n Period Exponential Moving Average of variable X is usually defined as:

//			X_new_avg = ( X_old_avg * (n-1) + 2 * X) / (n+1)

	UINT	BlcMotionAvgPeriod = 20;		// currently 1..200

// "Pixel Comb Sensitivity" slider:  This determines how sensitive we are to the current
// comb factor of each pixel.  I used a simplified comb factor C = abs(2*W - H - L)/2,
// which is just the distance of the Weave pixel's luma from its interpolated value.
// This value is calculated separately for each pixel.  This value along with the Pixel
// Motion Sense seem to be the two main things to play with to get good results.  Generally,
// increase one of these if you get Weave artifacts and decrease one if you get BOB artifacts.
  
	UINT	BlcPixelCombSense = 27;

// "Recent Comb Senseitivity" slider:  Operates like the Recent Motion slider but operates
// on the average Comb Factor.

	UINT	BlcRecentCombSense = 0;

// "Comb Average Period" slider: Sets the period of the Comb exponential moving average.
// See the comments on "Motion Average Period".

	UINT	BlcCombAvgPeriod = 20;			// currently 1.200

// "Skip High Comb Frames" slider:  I added this one in the hopes that it could help to
// skip a frame in the event of a sudden flash attack on a rapid scene change or maybe
// help to handle some very poorly mastered anime using ?:? pulldown.  I have not had
// a chance to experiment with it yet.  It will give very ugly results if you set it 
// too high.

	UINT	BlcHighCombSkip = 10;			// larger values skip more

// "Skip Low Motion Frames" slider:  This also is just experimental an probably of low
// value.  The idea here is that any frame with sufficiently low change from the previous
// one is maybe a still frame with some video noise, and could be skipped.  Not for
// normal use.  NOTE - This slider (but not parm) will soon be replaced by the
// Vertical Smoothing slider.

	UINT	BlcLowMotionSkip = 0;			// larger values skip more

// "Vertical Smoothing" slider: Sets a smoothing constant to smooth between the even
// and odd lines.  Not yet implemented, but the INI parm is there.

	UINT    BlcVerticalSmoothing = 0;

// "Use Interpolated BOB instead of Clip" check box.  For those who don't like the
// Clipped Weave, this will change it to an Interpolated Bob.  All other blending and
// processing will still function. (but it probably won't look as good).

	BOOL	BlcUseInterpBob = FALSE;

// "Blend Chroma Value" check box:  Usually the chroma value for the Clipped Weave is
// just taken from the pixel above it.  Checking this box causes the chroma values to 
// also use the clip algoritm.

// Checking this box seems to get richer color detail and a more theater like picture
// but it sometimes seems to create some softness or shimmering on my stock ticker or
// rapidly moving objects with lots of detail like a hockey game.

	BOOL	BlcBlendChroma = TRUE;			// default should maybe be TRUE?

// Finally there is an INI parm, but not a contol to determine whether to even display
// the Blended Clipping controls when that method is selected. If set to false then
// Blended Clipping parms are determined only from the INI file.

	BOOL	BlcShowControls = TRUE;

// Other global values, not user parms:

	UINT	BlcAverageMotions[5][2] = {0};  // reserved
	UINT	BlcTotalAverageMotion = 0;
	UINT	BlcAverageCombs[5][2] = {0};	// reserved
	UINT	BlcTotalAverageComb = 0;
	BOOL	BlcWantsToFlip;


void BlendedClipping(short** pOddLines, short** pEvenLines, 
		short** pPrevLines, BYTE* lpCurOverlay, BOOL bIsOdd)
{
	int Line;
	int	LoopCtr;
	int OddPtr;
	long X;
	short* L1;					// ptr to Line1, of 3
	short* L2;					// ptr to Line2, the weave line
	short* L3;					// ptr to Line3
	short* LP1;					// ptr to prev Line1
	short* LP3;					// ptr to prev Line3
	BYTE* Dest;
	const __int64 YMask		= 0x00ff00ff00ff00ff;	// to keep only luma
	const __int64 UVMask    = 0xff00ff00ff00ff00;	// to keep only chroma
	const __int64 ShiftMask = 0xfefffefffefffeff;	// to avoid shifting chroma to luma
	const __int64 SomeOnes  = 0x0001000100010001;	
	__int64 i;
	__int64 MinClip;
	__int64 MinClipMinus;
	__int64 PixelMotionSense;
	__int64 PixelCombSense;
	__int64 L1Mask;					// determines blended chroma vs. chroma from line 1
	__int64 LCMask;					// determines whether we use clip or blend chroma
	__int64 BobMask;				// determines whether we bob or blend chroma
	__int64 MotionAvgL;				// work sum for one line
	__int64 CombAvgL;				// "
	__int64 MotionAvg;				// total sum/avg
	__int64 CombAvg;				// "
	
	union 
	{
		__int64 Wqword;			
		__int32 Wlong[2];
	} W;

	// Set up and scale our user parms
	MotionAvg = 0;
	CombAvg = 0;

	if (BlcBlendChroma)
	{
		if (BlcUseInterpBob)		// use Luma, chroma from interp bob 
		{
			L1Mask = 0;
			LCMask = 0;
			BobMask = 0xfefffefffefffeff;
		}
		else						// use Luma, chroma from Clip
		{
			L1Mask = 0;
			LCMask = 0xffffffffffffffff;
			BobMask = 0;
		}
	}
	else
	{
		if (BlcUseInterpBob)		// use Luma from bob, chroma from L1
		{
			L1Mask = UVMask;
			LCMask = 0;
			BobMask = YMask;
		}
		else						// use Luma from Clip, chroma from L1
		{
			L1Mask = UVMask;
			LCMask = YMask;
			BobMask = 0;
		}
	}

// The effects of the recent average motion, recent average comb, and their relative
// weighting parameters can be precalculated only once per frame and rolled into the
// working value of the user constant parm, BlcMinimumClip, so we don't have to do
// calculations on these for every pixel.  Only the greater of recent motion or comb is used.

// A certain amount of fudging is needed for these values in the next few lines since it
// is desirable the effects center about realistic actual values of avg motion & comb.  
// A random sampling of the values in both avg motion and avg comb show values usually
// in the 1000-4000 range, with occasional jumps to much higher values.  Since the user
// parms are 0-100 and we want output values 0-65535 we will assume a max usual value for each
// average of 4500 and so divide by ((100*4500)/65535)) = 7 for now. (up to 1/5 for new Comb)

// Note the motion and comb average values have been scaled up by 256 in the averaging rtn, so
// the typical value of 2000 means an average change of about 8 in the 8 bit luma values.
// Both BlcMinimumClip and BlcRecentMotionSense may now have negative values but since
// we are using saturated arithmatic those are set in a separate field.
	X = __max(BlcRecentMotionSense,0);
	X = (X * BlcTotalAverageMotion / 7) 
				+ (BlcRecentCombSense * BlcTotalAverageComb) / 5;
	i = __max((BlcMinimumClip * 65535 / 100), 0);
	i = __min( (X + i), 65535);				// scale to range of 0-65535
	MinClip = i << 48 | i << 32 | i << 16 | i;
	
	X = __max( (-BlcRecentMotionSense * BlcTotalAverageMotion / 10), 0)
		+ __max( (-BlcMinimumClip), 0);
	i = __min(X, 255);
	MinClipMinus = i << 48 | i << 32 | i << 16 | i;

// Set up our two parms that are actually evaluated for each pixel
	i = BlcPixelMotionSense * 257/100;		// scale to range of 0-257
	PixelMotionSense = i << 48 | i << 32 | i << 16 | i;    // only 32 bits?>>>>
	
	i = BlcPixelCombSense * 257/100;		// scale to range of 0-257
	PixelCombSense = i << 48 | i << 32 | i << 16 | i;    // only 32 bits?>>>>
	
	OddPtr = (bIsOdd) ? 1 : 0;
	
// copy first even line no matter what, and the first odd line if we're
// processing an odd field.
	memcpyMMX(lpCurOverlay, pEvenLines[0], CurrentX * 2);	// DL0
	if (bIsOdd)
		memcpyMMX(lpCurOverlay + OverlayPitch, pOddLines[0], CurrentX * 2);  // DL1

	for (Line = 0; Line < (CurrentY / 2 - 1); ++Line)
	{
		LoopCtr = CurrentX / 4;				// there are ActiveX * 2 / 8 qwords per line
		MotionAvgL = 0;
		CombAvgL = 0;

		if (bIsOdd)
		{
			L1 = pOddLines[Line];		
			L2 = pEvenLines[Line + 1];	
			L3 = pOddLines[Line + 1];	
			LP1 = pPrevLines[Line];			// prev Odd lines
			LP3 = pPrevLines[Line+1];		
			Dest = lpCurOverlay + (Line * 2 + 2) * OverlayPitch;	// DL2
		}
		else
		{
			L1 = pEvenLines[Line];		
			L2 = pOddLines[Line];		
			L3 = pEvenLines[Line + 1];   
			LP1 = pPrevLines[Line];			// prev even lines
			LP3 = pPrevLines[Line+1];		
			Dest = lpCurOverlay + (Line * 2 + 1) * OverlayPitch;	// DL1
		}
		memcpyMMX(Dest + OverlayPitch, L3, CurrentX * 2);

// For ease of reading, the comments below assume that we're operating on an odd
// field (i.e., that bIsOdd is true).  The exact same processing is done when we
// operate on an even field, but the roles of the odd and even fields are reversed.
// It's just too cumbersome to explain the algorithm in terms of "the next odd
// line if we're doing an odd field, or the next even line if we're doing an
// even field" etc.  So wherever you see "odd" or "even" below, keep in mind that
// half the time this function is called, those words' meanings will invert.

		_asm
		{
			mov eax, dword ptr [L1]		
			mov ebx, dword ptr [L2]		
			mov edx, dword ptr [L3]		
			mov esi, dword ptr [LP1]		
			mov ecx, dword ptr [LP3]   
			mov edi, dword ptr [Dest]       // DL2 if Odd or DL1 if Even 
			movq mm7,YMask					// useful constant within loop 
			
			align 8
DoNext8Bytes:			
			movq mm0, qword ptr[eax]		// L1
			movq mm1, qword ptr[ebx]		// L2
			movq mm2, qword ptr[edx]		// L3

// OK, now we've moved in DL3 and Dl1 (earlier).  We have to make something for DL2
// What we will do is use L2 unless it is > Max(L1,L3) or < Min(L2,L3). If so clip to the closer.
// Note we are clipping all of Y,U, and V values here, not just masking Luma

			movq mm4, mm0					// L1
			movq mm6, mm2					// L3
			psubusb mm4, mm2				// - L3, with saturation
			paddusb mm4, mm2                // now = Max(L1,L3)

			pcmpeqb mm3, mm3					// all ffffffff
			psubusb mm3, mm0				// - L1 
			paddusb mm6, mm3				// add may sat at fff..
			psubusb mm6, mm3				// now = Min(L1,L3)
			
			movq mm5,mm1					// work copy of L2,the weave value
			psubusb mm5, mm6				// L2 - Min
			paddusb mm5, mm6				// now = Max(L2,Min(L1,L3)

			pcmpeqb mm3, mm3				// all ffffffff
			psubusb mm3, mm5				// - Max(L2,Min(L1,L3) 
			paddusb mm4, mm3				// add may sat at FFF..
			psubusb mm4, mm3				// now = Min( Max(L2, Min(L1,L3), L2 )=L2 clipped

// We have created the clipped value but we may not want to use it, depending on user
// parm BlcUseInterpBoB.  Make the bob value too.
			
			movq	mm5, mm0				// L1
			pand	mm5, ShiftMask			// "
			psrlw	mm5, 1
			movq	mm6, mm2				// L3
			pand	mm6, ShiftMask			// "
			psrlw   mm6, 1
			paddb   mm5, mm6				// interpolated bob here is just the average

// Now use our preset flag fields to select which we want

			movq	mm6, mm5				// copy of Bob
			pand	mm6, BobMask			// may mask out nothing, chroma, or all
			pand    mm4, LCMask				// may mask out nothing, chroma, or all
			por		mm4, mm6				// our choice, with or without chroma

// Now is a good time to calculate the Comb Factor.  A simple version is just the
// distance between L2 and the bob (interpolated middle) value now in mm5.
/*  try a different way for now, maybe change back later - TRB 11/14/00
			movq	mm6, mm1				// L2
			psubusb mm6, mm5				// L2 - bob, with sat
			psubusb	mm5, mm1				// bob - L2
			por		mm5, mm6				// abs diff (bob - L2)
			pand	mm5, mm7				// keep only luma
			movq	mm6, mm5				// save a copy for pixel comb sense calc
			paddusw mm5, CombAvgL			// bump our hist average
			movq	CombAvgL, mm5			// and save again
			pmullw  mm6, PixelCombSense     // mul by user factor, keep only low 16 bits
*/

// Instead let's let the Comb Factor just be the difference between L2 and the clipped
// value.  It will be zero for any pixel lying in the range where it does not get clipped.
// This avoids penalizing pixels that just happens to be in a high vertical contrast area.
// Doing this gives an adjustment similar to the use of the EdgeDetect value in the
// original Video Delinterlace routine.
			movq	mm6, mm1				// L2
			movq    mm5, mm4                // our clipped value, call it LC
			psubusb mm6, mm5				// L2 - LC, with sat
			psubusb	mm5, mm1				// LC - L2
			por		mm5, mm6				// abs diff (LC - L2)
			pand	mm5, mm7				// keep only luma
			movq	mm6, mm5				// save a copy for pixel comb sense calc
			paddusw mm5, CombAvgL			// bump our hist average
			movq	CombAvgL, mm5			// and save again
			psubusb mm6, MinClipMinus       // possibly forgive small values
			pmullw  mm6, PixelCombSense     // mul by user factor, keep only low 16 bits
			paddusw mm6, mm6                // try making it bigger
			paddusw mm6, mm6				// again
			paddusw mm6, mm6				// again
			paddusw mm6, mm6				// again
			paddusw mm6, mm6				// again
			paddusw mm6, mm6				// again

// Let's see how much L1 or L3 have changed since the last frame.  If L1 or L3 has  
// changed a lot (we take the greater) then L2 (the weave pixel) probably has also.
// Note that just measuring the change in L2 directly would not be current enough
// in time to avoid the 'flash attack' of weaves during quick scene changes.
// Calc first for L3 change but sum only L1 change in our saved totals for the average.
// L2 is destroyed here, no longer available.

			movq	mm5, qword ptr[ecx]		// LP3, prev L3
			movq	mm3, mm2				// work copy of L3
			psubusb mm3, mm5				// L3 - LP3
			psubusb mm5, mm2				// LP3 - L3
			por		mm5, mm3				// abs(L3 - LP3)
			pand	mm5, mm7				// Ymask, keep luma

			movq	mm3, mm0				// L1, another copy
			movq	mm2, qword ptr[esi]		// LP1, the previous value of L1
			psubusb mm3, mm2				// L1 - LP1, unsigned & sat.
			psubusb mm2, mm0				// LP1 - L1, unsigned & sat.
			por		mm2, mm3				// abs(L1-LP1)
			pand	mm2, mm7				// Ymask, keep luma

			movq	mm3, MotionAvgL			// good time to update our average totals
			paddusw mm3, mm2
			movq	MotionAvgL, mm3

			psubusb mm2, mm5
			paddusb mm2, mm5				// max of abs(L1-LP1) and abs(L3-LP3)
			psubusb mm2, MinClipMinus		// but maybe ignore some small changes

			pmullw	mm2, PixelMotionSense	// mul by user factor, keep only low 16 bits		
			paddusw mm2, mm6				// combine with our pixel comb
			paddusw mm2, mm2				// let's dbl them both for greater sensitivity

// Now turn the motion & comb factors in mm2 into a 2 blending factors that sum to 256

			paddusw mm2,MinClip				// add user and history factors to bias upward
			pcmpeqw mm3,mm3					// set all fff... = 65536
			psubusw mm3,mm2                 // get (more or less) 64k-mm2
			paddusw mm2, SomeOnes			// adjust so they total 64K?
			psrlw   mm2,8					// clip factor
			psrlw   mm3,8					// weave factor, both sum to +-256 

// We still have the clipped (or bob) value in mm4.  Let's call it LC below.
// mm2 and mm3 should now have factors for how much clip & weave respecively, sum=256
			movq	mm5,mm4					// save copy of clipped val, LC
			pand    mm4,mm7					// Ymask, keep luma from clipped val, LC
			pmullw  mm4,mm2					// clip fact * Clip luma
			pand    mm1,mm7					// Ymask, keep luma from weave val, L2
			pmullw  mm1,mm3                 // weave fact * weave luma
			paddusw mm4,mm1					
			psrlw   mm4,8					// back to 8 bit luma

// combine luma results with chroma and store 4 pixels
			pand	mm5, UVMask				// maybe keep Chroma from LC, or 0
			pand	mm0, L1Mask				// Maybe chroma comes from here
			por		mm4, mm0
			por		mm4, mm5				// combine them and we've got it
			movq qword ptr[edi], mm4        // and that is our final answer

// bump ptrs and loop
			lea		eax,[eax+8]				
			lea		ebx,[ebx+8]
			lea		ecx,[ecx+8]
			lea		edx,[edx+8]
			lea		edi,[edi+8]			
			lea		esi,[esi+8]
			dec		LoopCtr
			jnz		DoNext8Bytes

// done with one line but have to roll some totals that might otherwise overflow
			movq	mm0, CombAvgL
			pmaddwd mm0, SomeOnes
			paddd	mm0, CombAvg
			movq    CombAvg, mm0
			
			movq	mm0, MotionAvgL
			pmaddwd mm0, SomeOnes
			paddd	mm0, MotionAvg
			movq    MotionAvg, mm0

			emms
		}
	}

	// Copy last odd line if we're processing an even field.
	if (! bIsOdd)
		memcpyMMX(lpCurOverlay + (CurrentY - 1) * OverlayPitch, pOddLines[CurrentY / 2 - 1], CurrentX * 2);

// We will keep moving averages of the Motion and Comb factors.  For extra precision the 
// values will be kept scaled up by 256.  See comments on rtn header about averages.

	BlcWantsToFlip = TRUE;			// assume we do 

	W.Wqword = MotionAvg;
	X = 256 * (W.Wlong[0] + W.Wlong[1]) / ( (CurrentY / 2 - 1) * CurrentX );

	// Do we want to skip the frame because motion is too small? Check before updating avg.
	if (BlcTotalAverageMotion > 0  
			&& (100 * X / BlcTotalAverageMotion) < BlcLowMotionSkip)
	{
		BlcWantsToFlip = FALSE;
	}
	
	BlcTotalAverageMotion = (BlcTotalAverageMotion * (BlcMotionAvgPeriod - 1) + 2 * X)
			/ (BlcMotionAvgPeriod + 1);

	W.Wqword = CombAvg;
	X = 256 * (W.Wlong[0] + W.Wlong[1]) / ( (CurrentY / 2 - 1) * CurrentX );

	if (BlcTotalAverageComb > 0       // Skip a very high comb frame?
			&& (100 * X / BlcTotalAverageComb) > 10 * (100 - BlcHighCombSkip))
	{
		BlcWantsToFlip = FALSE;
	}
	
	BlcTotalAverageComb = (BlcTotalAverageComb * (BlcCombAvgPeriod - 1) + 2 * X)
			/ (BlcCombAvgPeriod + 1);


}


///////////////////////////////////////////////////////////////////////////////
// Deinterlace the latest field, attempting to weave wherever it won't cause
// visible artifacts.
//
// The algorithm is described in comments below.
//
void DeinterlaceFieldTwoFrame(short** pOddLines,
					          short** pEvenLines,
					          short** pPrevOdd,
					          short** pPrevEven,
					          BYTE* lpCurOverlay,
					          BOOL bIsOdd)
{
	int Line;
	short* YVal0;
	short* YVal1;
	short* YVal2;
	short* OVal0;
	short* OVal1;
	short* OVal2;
	BYTE* OldSI;
	BYTE* OldSP;
	BYTE* Dest;

	const __int64 YMask    = 0x00ff00ff00ff00ff;
	const __int64 UVMask    = 0xff00ff00ff00ff00;

	__int64 qwSpatialTolerance;
	__int64 qwTemporalTolerance;
	__int64 qwAllOnes = 0xffffffffffffffff;
	__int64 qwBobbedPixels;
	const __int64 Mask = 0x7f7f7f7f7f7f7f7f;

	qwSpatialTolerance = SpatialTolerance / 4;		// divide by 4 because of squaring behavior, see below
	qwSpatialTolerance += (qwSpatialTolerance << 48) + (qwSpatialTolerance << 32) + (qwSpatialTolerance << 16);
	qwTemporalTolerance = TemporalTolerance / 4;
	qwTemporalTolerance += (qwTemporalTolerance << 48) + (qwTemporalTolerance << 32) + (qwTemporalTolerance << 16);

	// copy first even line no matter what, and the first odd line if we're
	// processing an even field.
	memcpyMMX(lpCurOverlay, pEvenLines[0], CurrentX * 2);
	if (! bIsOdd)
		memcpyMMX(lpCurOverlay + OverlayPitch, pOddLines[0], CurrentX * 2);

	for (Line = 0; Line < (CurrentY / 2 - 1); ++Line)
	{
		if (bIsOdd)
		{
			YVal0 = pOddLines[Line];
			YVal1 = pEvenLines[Line + 1];
			YVal2 = pOddLines[Line + 1];
			OVal0 = pPrevOdd[Line];
			OVal1 = pPrevEven[Line + 1];
			OVal2 = pPrevOdd[Line + 1];
			Dest = lpCurOverlay + (Line * 2 + 3) * OverlayPitch;
		}
		else
		{
			YVal0 = pEvenLines[Line];
			YVal1 = pOddLines[Line];
			YVal2 = pEvenLines[Line + 1];
			OVal0 = pPrevEven[Line];
			OVal1 = pPrevOdd[Line];
			OVal2 = pPrevEven[Line + 1];
			Dest = lpCurOverlay + (Line * 2 + 2) * OverlayPitch;
		}

		// Always use the most recent data verbatim.  By definition it's correct (it'd
		// be shown on an interlaced display) and our job is to fill in the spaces
		// between the new lines.
		memcpyMMX(Dest, YVal2, CurrentX * 2);
		Dest -= OverlayPitch;

		_asm
		{
			// We'll be using a couple registers that have meaning in the C code, so
			// save them.
			mov dword ptr[OldSI], esi
			mov dword ptr[OldSP], esp

			// Figure out what to do with the scanline above the one we just copied.
			// We use the following notation for the top, middle, and bottom pixels
			// of concern:
			//
			// Field 1 | Field 2 | Field 3 | Field 4 |
			//         |   T0    |         |   T1    | scanline we copied in last iteration
			//   M0    |         |    M1   |         | intermediate scanline from alternate field
			//         |   B0    |         |   B1    | scanline we just copied
			//
			// We will weave if any of the following is true:
			//   - M1 is similar to either B1 or T1.  This indicates that no weave
			//     artifacts would be visible.
			//   - T1 and B1 and M1 are old.  In that case any weave artifact that
			//     appears isn't due to fast motion, since it was there in the previous
			//     frame too.
			//
			// Pixels are processed 4 at a time using MMX instructions.
			//
			// SQUARING NOTE:
			// We square luminance differences to amplify the effects of large
			// differences, which makes it easier to set thresholds.  Unfortunately,
			// we can't compare the square of difference directly against a threshold,
			// thanks to the lack of an MMX unsigned compare instruction.  The
			// problem is that if we had two pixels with luminance 0 and 255,
			// the difference squared would be 65025, which is a negative
			// 16-bit signed value and would thus compare less than a threshold.
			// We get around this by dividing all the luminance values by two before
			// squaring them; this results in an effective maximum luminance
			// difference of 127, whose square (16129) is safely comparable.

			mov ecx, CurrentX
			mov eax, dword ptr [YVal0]		// eax = T1
			mov ebx, dword ptr [YVal1]		// ebx = M1
			mov esp, dword ptr [YVal2]		// esp = B1
			mov edx, dword ptr [OVal0]		// edx = T0
			mov esi, dword ptr [OVal1]		// esi = M0
			shr ecx, 2						// there are CurrentX * 2 / 8 qwords

align 8
DoNext8Bytes:
			mov edi, dword ptr [OVal2]		// edi = B0
			movq mm0, qword ptr[eax]		// mm0 = T1
			movq mm1, qword ptr[esp]		// mm1 = B1
			movq mm2, qword ptr[ebx]		// mm2 = M1

			// Average T1 and B1 so we can do interpolated bobbing if we bob onto T1.
			movq mm7, mm1					// mm7 = B1
			// (Note: This _emit sequence is the "pavgb mm7,mm0" instruction, which
			// doesn't work on all processors.)
			// _emit 0x0f
			// _emit 0xe0
			// _emit 0xf8
			movq mm5, mm0					// mm5 = T1
			psrlw mm7, 1					// mm7 = B1 / 2
			pand mm7, Mask					// mask off lower bits
			psrlw mm5, 1					// mm5 = T1 / 2
			pand mm5, Mask					// mask off lower bits
			paddw mm7, mm5					// mm7 = (T1 + B1) / 2
			movq qwBobbedPixels, mm7

			// Now that we've averaged them, we no longer care about the chroma
			// values of T1 and B1 (all our comparisons are luminance-only).
			pand mm0, YMask					// mm0 = luminance(T1)
			pand mm1, YMask					// mm1 = luminance(B1)

			// Find out whether M1 is new.  "New" means the square of the
			// luminance difference between M1 and M0 is less than the temporal
			// tolerance.
			//
			movq mm7, mm2					// mm7 = M1
			movq mm4, qword ptr[esi]		// mm4 = M0
			pand mm7, YMask					// mm7 = luminance(M1)
			movq mm6, mm7					// mm6 = luminance(M1)     used below
			pand mm4, YMask					// mm4 = luminance(M0)
			psubsw mm7, mm4					// mm7 = M1 - M0
			psraw mm7, 1					// mm7 = M1 - M0 (see SQUARING NOTE above)
			pmullw mm7, mm7					// mm7 = (M1 - M0) ^ 2
			pcmpgtw mm7, qwTemporalTolerance // mm7 = 0xffff where (M1 - M0) ^ 2 > threshold, 0x0000 otherwise

			// Find out how different T1 and M1 are.
			movq mm3, mm0					// mm3 = T1
			psubsw mm3, mm6					// mm3 = T1 - M1
			psraw mm3, 1					// mm3 = T1 - M1 (see SQUARING NOTE above)
			pmullw mm3, mm3					// mm3 = (T1 - M1) ^ 2
			pcmpgtw mm3, qwSpatialTolerance	// mm3 = 0xffff where (T1 - M1) ^ 2 > threshold, 0x0000 otherwise

			// Find out how different B1 and M1 are.
			movq mm4, mm1					// mm4 = B1
			psubsw mm4, mm6					// mm4 = B1 - M1
			psraw mm4, 1					// mm4 = B1 - M1 (see SQUARING NOTE above)
			pmullw mm4, mm4					// mm4 = (B1 - M1) ^ 2
			pcmpgtw mm4, qwSpatialTolerance	// mm4 = 0xffff where (B1 - M1) ^ 2 > threshold, 0x0000 otherwise

			// We care about cases where M1 is different from both T1 and B1.
			pand mm3, mm4					// mm3 = 0xffff where M1 is different from T1 and B1, 0x0000 otherwise

			// Find out whether T1 is new.
			movq mm4, mm0					// mm4 = T1
			movq mm5, qword ptr[edx]		// mm5 = T0
			pand mm5, YMask					// mm5 = luminance(T0)
			psubsw mm4, mm5					// mm4 = T1 - T0
			psraw mm4, 1					// mm4 = T1 - T0 (see SQUARING NOTE above)
			pmullw mm4, mm4					// mm4 = (T1 - T0) ^ 2 / 4
			pcmpgtw mm4, qwTemporalTolerance // mm4 = 0xffff where (T1 - T0) ^ 2 > threshold, 0x0000 otherwise

			// Find out whether B1 is new.
			movq mm5, mm1					// mm5 = B1
			movq mm6, qword ptr[edi]		// mm6 = B0
			pand mm6, YMask					// mm6 = luminance(B0)
			psubsw mm5, mm6					// mm5 = B1 - B0
			psraw mm5, 1					// mm5 = B1 - B0 (see SQUARING NOTE above)
			pmullw mm5, mm5					// mm5 = (B1 - B0) ^ 2
			pcmpgtw mm5, qwTemporalTolerance // mm5 = 0xffff where (B1 - B0) ^ 2 > threshold, 0x0000 otherwise

			// We care about cases where M1 is old and either T1 or B1 is old.
			por mm4, mm5					// mm4 = 0xffff where T1 or B1 is new
			por mm4, mm7					// mm4 = 0xffff where T1 or B1 or M1 is new
			movq mm6, qwAllOnes				// mm6 = 0xffffffffffffffff
			pxor mm4, mm6					// mm4 = 0xffff where T1 and B1 and M1 are old

			// Pick up the interpolated (T1+B1)/2 pixels.
			movq mm1, qwBobbedPixels		// mm1 = (T1 + B1) / 2

			// At this point:
			//  mm1 = (T1+B1)/2
			//  mm2 = M1
			//  mm3 = mask, 0xffff where M1 is different from both T1 and B1
			//  mm4 = mask, 0xffff where T1 and B1 and M1 are old
			//  mm6 = 0xffffffffffffffff
			//
			// Now figure out where we're going to weave and where we're going to bob.
			// We'll weave if all pixels are old or M1 isn't different from both its
			// neighbors.
			pxor mm3, mm6					// mm3 = 0xffff where M1 is the same as either T1 or B1
			por mm3, mm4					// mm3 = 0xffff where M1 and T1 and B1 are old or M1 = T1 or B1
			pand mm2, mm3					// mm2 = woven data where T1 or B1 isn't new or they're different
			pandn mm3, mm1					// mm3 = bobbed data where T1 or B1 is new and they're similar
			por mm3, mm2					// mm3 = finished pixels

			// Shuffle some registers around since there aren't enough of them
			// to hold all our pointers at once.
			add edi, 8
			mov dword ptr[OVal2], edi
			mov edi, dword ptr[Dest]

			// Put the pixels in place.
			movq qword ptr[edi], mm3

			// Advance to the next set of pixels.
			add eax, 8
			add ebx, 8
			add edx, 8
			add esi, 8
			add esp, 8
			add edi, 8
			mov dword ptr[Dest], edi
			dec ecx
			jne near DoNext8Bytes

			emms

			mov esi, dword ptr[OldSI]
			mov esp, dword ptr[OldSP]
		}
	}

	// Copy last odd line if we're processing an odd field.
	if (bIsOdd)
		memcpyMMX(lpCurOverlay + (CurrentY - 1) * OverlayPitch, pOddLines[CurrentY / 2 - 1], CurrentX * 2);
}


///////////////////////////////////////////////////////////////////////////////
// Deinterlace the latest field, with a tendency to weave rather than bob.
// Good for high detail on low-movement scenes.
//
// The algorithm is described in comments below.
//
void DeinterlaceFieldWeave(short** pOddLines, short** pEvenLines, short** pPrevLines, BYTE* lpCurOverlay, BOOL bIsOdd)
{
	int Line;
	short* YVal1;
	short* YVal2;
	short* YVal3;
	short* YVal4;
	BYTE* OldStack;
	BYTE* Dest;

	const __int64 YMask    = 0x00ff00ff00ff00ff;
	const __int64 UVMask    = 0xff00ff00ff00ff00;

	__int64 qwSpatialTolerance;
	__int64 qwTemporalTolerance;
	__int64 qwThreshold;
	const __int64 Mask = 0xfefefefefefefefe;

	qwSpatialTolerance = SpatialTolerance;
	qwSpatialTolerance += (qwSpatialTolerance << 48) + (qwSpatialTolerance << 32) + (qwSpatialTolerance << 16);
	qwTemporalTolerance = TemporalTolerance;
	qwTemporalTolerance += (qwTemporalTolerance << 48) + (qwTemporalTolerance << 32) + (qwTemporalTolerance << 16);
	qwThreshold = SimilarityThreshold;
	qwThreshold += (qwThreshold << 48) + (qwThreshold << 32) + (qwThreshold << 16);

	// copy first even line no matter what, and the first odd line if we're
	// processing an even field.
	memcpyMMX(lpCurOverlay, pEvenLines[0], CurrentX * 2);
	if (! bIsOdd)
		memcpyMMX(lpCurOverlay + OverlayPitch, pOddLines[0], CurrentX * 2);

	for (Line = 0; Line < (CurrentY / 2 - 1); ++Line)
	{
		if (bIsOdd)
		{
			YVal1 = pEvenLines[Line];
			YVal2 = pOddLines[Line];
			YVal3 = pEvenLines[Line + 1];
			YVal4 = pPrevLines[Line];
			Dest = lpCurOverlay + (Line * 2 + 1) * OverlayPitch;
		}
		else
		{
			YVal1 = pOddLines[Line];
			YVal2 = pEvenLines[Line + 1];
			YVal3 = pOddLines[Line + 1];
			YVal4 = pPrevLines[Line + 1];
			Dest = lpCurOverlay + (Line * 2 + 2) * OverlayPitch;
		}

		// For ease of reading, the comments below assume that we're operating on an odd
		// field (i.e., that bIsOdd is true).  The exact same processing is done when we
		// operate on an even field, but the roles of the odd and even fields are reversed.
		// It's just too cumbersome to explain the algorithm in terms of "the next odd
		// line if we're doing an odd field, or the next even line if we're doing an
		// even field" etc.  So wherever you see "odd" or "even" below, keep in mind that
		// half the time this function is called, those words' meanings will invert.

		// Copy the even scanline below this one to the overlay buffer, since we'll be
		// adapting the current scanline to the even lines surrounding it.  The scanline
		// above has already been copied by the previous pass through the loop.
		memcpyMMX(Dest + OverlayPitch, YVal3, CurrentX * 2);

		_asm
		{
			mov dword ptr[OldStack], esi

			mov ecx, CurrentX
			mov eax, dword ptr [YVal1]
			mov ebx, dword ptr [YVal2]
			mov edx, dword ptr [YVal3]
			mov esi, dword ptr [YVal4]
			mov edi, dword ptr [Dest]
			shr ecx, 2       // there are ActiveX * 2 / 8 qwords

align 8
DoNext8Bytes:			
			movq mm0, qword ptr[eax]		// mm0 = E1
			movq mm1, qword ptr[ebx]		// mm1 = O
			movq mm2, qword ptr[edx]		// mm2 = E2

			movq mm3, mm0					// mm3 = intensity(E1)
			movq mm4, mm1					// mm4 = intensity(O)
			movq mm6, mm2					// mm6 = intensity(E2)
			pand mm3, YMask
			pand mm4, YMask
			pand mm6, YMask

			// Average E1 and E2 for interpolated bobbing.
#if 0
			// The following SSE instruction doesn't work on older CPUs, but is faster for
			// newer ones.  Comment it out until we have CPU type detection.
			// This is the hex encoding for "pavg mm0,mm2" which VC++'s assembler doesn't
			// seem to recognize.
			_emit 0x0F						// mm0 = avg(E1, E2)
			_emit 0xE0
			_emit 0xC2
#else
			pand mm0, Mask					// mm0 = E1 with lower chroma bit stripped off
			psrlw mm0, 1					// mm0 = E1 / 2
			pand mm2, Mask					// mm2 = E2 with lower chroma bit stripped off
			psrlw mm2, 1					// mm2 = E2 / 2
			paddb mm0, mm2					// mm2 = (E1 + E2) / 2
#endif

			// The meat of the work is done here.  We want to see whether this pixel is
			// close in luminosity to ANY of: its top neighbor, its bottom neighbor,
			// or its predecessor.  To do this without branching, we use MMX's
			// saturation feature, which gives us Z(x) = x if x>=0, or 0 if x<0.
			//
			// The formula we're computing here is
			//		Z(ST - (E1 - O) ^ 2) + Z(ST - (E2 - O) ^ 2) + Z(TT - (Oold - O) ^ 2)
			// where ST is spatial tolerance and TT is temporal tolerance.  The idea
			// is that if a pixel is similar to none of its neighbors, the resulting
			// value will be pretty low, probably zero.  A high value therefore indicates
			// that the pixel had a similar neighbor.  The pixel in the same position
			// in the field before last (Oold) is considered a neighbor since we want
			// to be able to display 1-pixel-high horizontal lines.

			movq mm7, qwSpatialTolerance
			movq mm5, mm3					// mm5 = E1
			psubsw mm5, mm4					// mm5 = E1 - O
			psraw mm5, 1
			pmullw mm5, mm5					// mm5 = (E1 - O) ^ 2
			psubusw mm7, mm5				// mm7 = ST - (E1 - O) ^ 2, or 0 if that's negative

			movq mm3, qwSpatialTolerance
			movq mm5, mm6					// mm5 = E2
			psubsw mm5, mm4					// mm5 = E2 - O
			psraw mm5, 1
			pmullw mm5, mm5					// mm5 = (E2 - O) ^ 2
			psubusw mm3, mm5				// mm0 = ST - (E2 - O) ^ 2, or 0 if that's negative
			paddusw mm7, mm3				// mm7 = (ST - (E1 - O) ^ 2) + (ST - (E2 - O) ^ 2)

			movq mm3, qwTemporalTolerance
			movq mm5, qword ptr[esi]		// mm5 = Oold
			pand mm5, YMask
			psubsw mm5, mm4					// mm5 = Oold - O
			psraw mm5, 1 // XXX
			pmullw mm5, mm5					// mm5 = (Oold - O) ^ 2
			psubusw mm3, mm5				// mm0 = TT - (Oold - O) ^ 2, or 0 if that's negative
			paddusw mm7, mm3				// mm7 = our magic number

			// Now compare the similarity totals against our threshold.  The pcmpgtw
			// instruction will populate the target register with a bunch of mask bits,
			// filling words where the comparison is true with 1s and ones where it's
			// false with 0s.  A few ANDs and NOTs and an OR later, we have bobbed
			// values for pixels under the similarity threshold and weaved ones for
			// pixels over the threshold.

			pcmpgtw mm7, qwThreshold		// mm7 = 0xffff where we're greater than the threshold, 0 elsewhere
			movq mm6, mm7					// mm6 = 0xffff where we're greater than the threshold, 0 elsewhere
			pand mm7, mm1					// mm7 = weaved data where we're greater than the threshold, 0 elsewhere
			pandn mm6, mm0					// mm6 = bobbed data where we're not greater than the threshold, 0 elsewhere
			por mm7, mm6					// mm7 = bobbed and weaved data

			movq qword ptr[edi], mm7
			add eax, 8
			add ebx, 8
			add edx, 8
			add edi, 8
			add esi, 8
			dec ecx
			jne near DoNext8Bytes
			emms

			mov esi, dword ptr[OldStack]
		}
	}

	// Copy last odd line if we're processing an odd field.
	if (bIsOdd)
		memcpyMMX(lpCurOverlay + (CurrentY - 1) * OverlayPitch, pOddLines[CurrentY / 2 - 1], CurrentX * 2);
}


///////////////////////////////////////////////////////////////////////////////
// GetCombFactor
//
// This routine basically calculates how close the pixels in pLines2
// are the interpelated pixels between pLines1
// this idea was taken from the VirtualDub CVideoTelecineRemover class
// at the moment it is the correct algoritm outlined in the comments
// not the one used in that program
// I only do this on the Y component as I assume that any noticable combing
// will be visible in the black and white image
// the relative sizes of the returns from this function will be used to 
// determine the best ordering of the fields
// This function only works on the area displayed so will perform better if any
// VBI lines are off screen
// the BitShift value is used to filter out noise and quantization error
///////////////////////////////////////////////////////////////////////////////
long GetCombFactor(short** pLines1, short** pLines2)
{
	int Line;
	long LineFactor;
	long CombFactor = 0;
	short* YVal1;
	short* YVal2;
	short* YVal3;
	long ActiveX = CurrentX - 2 * InitialOverscan;
	const __int64 YMask    = 0x00ff00ff00ff00ff;
	const __int64 qwOnes = 0x0001000100010001;
	__int64 wBitShift    = BitShift;

	__int64 qwEdgeDetect;
	__int64 qwThreshold;
	const __int64 Mask = 0x7f7f7f7f7f7f7f7f;

	qwEdgeDetect = EdgeDetect;
	qwEdgeDetect += (qwEdgeDetect << 48) + (qwEdgeDetect << 32) + (qwEdgeDetect << 16);
	qwThreshold = JaggieThreshold;
	qwThreshold += (qwThreshold << 48) + (qwThreshold << 32) + (qwThreshold << 16);

	for (Line = 100; Line < ((CurrentY - 100) / 2); ++Line)
	{
		YVal1 = pLines1[Line] + InitialOverscan;
		YVal2 = pLines2[Line] + InitialOverscan;
		YVal3 = pLines1[Line + 1] + InitialOverscan;
		_asm
		{
			mov ecx, ActiveX
			mov eax,dword ptr [YVal1]
			mov ebx,dword ptr [YVal2]
			mov edx,dword ptr [YVal3]
			shr ecx, 2       // there are ActiveX * 2 / 8 qwords
		    movq mm1, YMask
			pxor mm0, mm0    // mm0 = 0
align 8
Next8Bytes:
			movq mm3, qword ptr[eax] 
			movq mm4, qword ptr[ebx] 
			movq mm5, qword ptr[edx]

			pand mm3, YMask
			pand mm4, YMask
			pand mm5, YMask

			// work out (O1 - E) * (O2 - E) - EdgeDetect * (O1 - O2) ^ 2 >> 12
			// result will be in mm6

			psrlw mm3, 01
			psrlw mm4, 01
			psrlw mm5, 01

			movq mm6, mm3
			psubw mm6, mm4		//mm6 = O1 - E

			movq mm7, mm5
			psubw mm7, mm4		//mm7 = O2 - E

			pmullw mm6, mm7		// mm0 = (O1 - E) * (O2 - E)

			movq mm7, mm3
			psubw mm7, mm5		// mm7 = (O1 - O2)
			pmullw mm7, mm7		// mm7 = (O1 - O2) ^ 2
			psrlw mm7, 12		// mm7 = (O1 - O2) ^ 2 >> 12
			pmullw mm7, qwEdgeDetect		// mm1  = EdgeDetect * (O1 - O2) ^ 2 >> 12

			psubw mm6, mm7      // mm6 is what we want

			pcmpgtw mm6, qwThreshold

			pand mm6, qwOnes

			paddw mm0, mm6

			add eax, 8
			add ebx, 8
			add edx, 8

			dec ecx
			jne near Next8Bytes

			movd eax, mm0
			psrlq mm0,32
			movd ecx, mm0
			add ecx, eax
			mov dword ptr[LineFactor], ecx
			emms
		}
		CombFactor += (LineFactor & 0xFFFF);
		CombFactor += (LineFactor >> 16);
	}
	return CombFactor;
}

///////////////////////////////////////////////////////////////////////////////
// CompareFields
//
// This routine basically calculates how close the pixels in pLines2
// are to the pixels in pLines1
// this is my attempt to implement Mark Rejhon's 3:2 pulldown code
// we will use this to dect the times when we get three fields in a row from
// the same frame
// the result is the total average diffrence between the Y components of each pixel
// This function only works on the area displayed so will perform better if any
// VBI lines are off screen
// the BitShift value is used to filter out noise and quantization error
///////////////////////////////////////////////////////////////////////////////
long CompareFields(short** pLines1, short** pLines2, RECT *rect)
{
	int Line;
	long LineFactor;
	long DiffFactor = 0;
	short* YVal1;
	short* YVal2;
	long ActiveX = rect->right - rect->left;
	const __int64 YMask    = 0x00ff00ff00ff00ff;
	__int64 wBitShift    = BitShift;

	for (Line = rect->top / 2; Line < rect->bottom / 2; ++Line)
	{
		YVal1 = pLines1[Line] + (rect->left & ~1);
		YVal2 = pLines2[Line] + (rect->left & ~1);
		_asm
		{
			mov ecx, ActiveX
			mov eax,dword ptr [YVal1]
			mov ebx,dword ptr [YVal2]
			shr ecx, 2		 // there are ActiveX * 2 / 8 qwords
		    movq mm1, YMask
			movq mm7, wBitShift
			pxor mm0, mm0    // mm0 = 0  this is running total
align 8
Next8Bytes:
			movq mm4, qword ptr[eax] 
			movq mm5, qword ptr[ebx] 
			pand mm5, mm1    // get only Y compoment
			pand mm4, mm1    // get only Y compoment

			psubw mm4, mm5   // mm4 = Y1 - Y2
			pmaddwd mm4, mm4 // mm4 = (Y1 - Y2) ^ 2
			psrld mm4, mm7   // divide mm4 by 2 ^ Bitshift
			paddd mm0, mm4   // keep total in mm0

			add eax, 8
			add ebx, 8
			
			dec ecx
			jne near Next8Bytes

			movd eax, mm0
			psrlq mm0,32
			movd ecx, mm0
			add ecx, eax
			mov dword ptr[LineFactor], ecx
			emms
		}
		DiffFactor += (long)sqrt(LineFactor);
	}
	return DiffFactor;
}

long CompareFields2(short** pLines1, short** pLines2)
{
	int Line;
	long LineFactor;
	long CombFactor = 0;
	short* YVal1;
	short* YVal2;
	long ActiveX = CurrentX - 2 * InitialOverscan;
	const __int64 YMask    = 0x00ff00ff00ff00ff;
	const __int64 qwOnes = 0x0001000100010001;
	__int64 wBitShift    = BitShift;

	__int64 qwThreshold;

	qwThreshold = DiffThreshold;
	qwThreshold += (qwThreshold << 48) + (qwThreshold << 32) + (qwThreshold << 16);

	for (Line = 100; Line < ((CurrentY - 100) / 2); ++Line)
	{
		YVal1 = pLines1[Line] + InitialOverscan;
		YVal2 = pLines2[Line] + InitialOverscan;
		_asm
		{
			mov ecx, ActiveX
			mov eax,dword ptr [YVal1]
			mov ebx,dword ptr [YVal2]
			shr ecx, 2       // there are ActiveX * 2 / 8 qwords
		    movq mm1, YMask
			pxor mm0, mm0    // mm0 = 0
align 8
Next8Bytes:
			movq mm3, qword ptr[eax] 
			movq mm4, qword ptr[ebx] 

			pand mm3, YMask
			pand mm4, YMask

			psubw mm3, mm4

			pmullw mm3, mm3

			pcmpgtw mm3, qwThreshold

			pand mm3, qwOnes

			paddw mm0, mm3

			add eax, 8
			add ebx, 8

			dec ecx
			jne near Next8Bytes

			movd eax, mm0
			psrlq mm0,32
			movd ecx, mm0
			add ecx, eax
			mov dword ptr[LineFactor], ecx
			emms
		}
		CombFactor += (LineFactor & 0xFFFF);
		CombFactor += (LineFactor >> 16);
	}
	return CombFactor;
}


