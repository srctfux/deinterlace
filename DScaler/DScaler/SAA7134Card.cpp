/////////////////////////////////////////////////////////////////////////////
// $Id: SAA7134Card.cpp,v 1.11 2002-10-08 19:35:45 atnak Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2002 Atsushi Nakagawa.  All rights reserved.
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
//
// This software was based on v4l2 device driver for philips
// saa7134 based TV cards.  Those portions are
// Copyright (c) 2001,02 Gerd Knorr <kraxel@bytesex.org> [SuSE Labs]
//
// This software was based on BT848Card.cpp.  Those portions are
// Copyright (c) 2001 John Adcock.
//
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 09 Sep 2002   Atsushi Nakagawa      Initial Release
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.10  2002/10/08 12:22:47  atnak
// added software card reset in ResetHardware(), etc
//
// Revision 1.9  2002/10/06 12:14:52  atnak
// cleaned up SetPageTable(...)
//
// Revision 1.8  2002/10/04 23:40:46  atnak
// proper support for audio channels mono,stereo,lang1,lang2 added
//
// Revision 1.7  2002/10/04 13:24:46  atnak
// Audio mux select through GPIO added (for 7130 cards)
//
// Revision 1.6  2002/10/03 23:36:23  atnak
// Various changes (major): VideoStandard, AudioStandard, CSAA7134Common, cleanups, tweaks etc,
//
// Revision 1.5  2002/09/15 14:28:07  atnak
// Tweaked VBI and VDelay settings
//
// Revision 1.4  2002/09/14 19:40:48  atnak
// various changes
//
// Revision 1.3  2002/09/10 12:14:35  atnak
// Some changes to eAudioStandard stuff
//
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "..\DScalerRes\resource.h"
#include "resource.h"
#include "SAA7134Card.h"
#include "SAA7134_Defines.h"
#include "SAA7134Common.h"
#include "SAA7134I2CBus.h"
#include "Audio.h"
#include "DebugLog.h"
#include "CPU.h"
/// \todo remove need for this
#include "ProgramList.h"
/// \todo remove need for this
#include "OutThreads.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CSAA7134Card::CSAA7134Card(CHardwareDriver* pDriver) :
    CPCICard(pDriver),
    m_CardType(TVCARD_UNKNOWN),
    m_Tuner(NULL),
    m_PreparedRegions(0x00),
    m_VideoStandard(VIDEOSTANDARD_INVALID),
    m_AudioStandard(AUDIOSTANDARD_BG_DUAL_FM)
{
    m_I2CInitialized = false;
    m_I2CBus = new CSAA7134I2CBus(this);
}


CSAA7134Card::~CSAA7134Card()
{
    // disable peripheral devices
    WriteByte(SAA7134_SPECIAL_MODE,0);

    WriteDword(SAA7134_IRQ1, 0UL);
    WriteDword(SAA7134_IRQ2, 0UL);
    // Completely zeroing this conflicts with Lifeview's software
    MaskDataDword(SAA7134_MAIN_CTRL, 0, 0xFF);

    delete m_I2CBus;
    delete m_Tuner;

    ClosePCICard();
}


int CSAA7134Card::RegionID2Channel(eRegionID RegionID)
{
    switch (RegionID)
    {
    case REGIONID_VIDEO_A: return 0;
    case REGIONID_VIDEO_B: return 1;
    case REGIONID_VBI_A: return 2;
    case REGIONID_VBI_B: return 3;
    default:
        // NEVER_GET_HERE;
        break;
    }
    return 0;
}


BYTE CSAA7134Card::TaskID2TaskMask(eTaskID TaskID)
{
    switch (TaskID)
    {
    case TASKID_A: return SAA7134_TASK_A_MASK;
    case TASKID_B: return SAA7134_TASK_B_MASK;
    default:
        // NEVER_GET_HERE;
        break;
    }
    return 0;
}


void CSAA7134Card::ResetHardware()
{
    WriteByte(SAA7134_REGION_ENABLE, 0x00);
    WriteByte(SAA7134_REGION_ENABLE, SAA7134_REGION_ENABLE_SWRST);
    WriteByte(SAA7134_REGION_ENABLE, 0x00);

    // LOG(0, "Initial registery dump");
    // DumpRegisters();

    WriteByte(SAA7134_SOURCE_TIMING, 0x04);
    WriteByte(SAA7134_SOURCE_TIMING_HIBYTE, 0x20);

    WriteByte(SAA7134_START_GREEN, 0x00);
    WriteByte(SAA7134_START_BLUE, 0x00);
    WriteByte(SAA7134_START_RED, 0x00);

    for (int i = 0; i < 0x0F; i++)
    {
        WriteByte(SAA7134_GREEN_PATH(i), (i+1)<<4);
        WriteByte(SAA7134_BLUE_PATH(i), (i+1)<<4);
        WriteByte(SAA7134_RED_PATH(i), (i+1)<<4);
    }
    WriteByte(SAA7134_GREEN_PATH(0x0F), 0xFF);
    WriteByte(SAA7134_BLUE_PATH(0x0F), 0xFF);
    WriteByte(SAA7134_RED_PATH(0x0F), 0xFF);

    // RAM FIFO config ???
    WriteDword(SAA7134_FIFO_SIZE, 0x08070503);
    WriteDword(SAA7134_THRESHOULD, 0x02020202);

    // Enable audio and video processing
    WriteDword(SAA7134_MAIN_CTRL,
            SAA7134_MAIN_CTRL_VPLLE |
            SAA7134_MAIN_CTRL_APLLE |
            SAA7134_MAIN_CTRL_EXOSC |
            SAA7134_MAIN_CTRL_EVFE1 |
            SAA7134_MAIN_CTRL_EVFE2 |
            SAA7134_MAIN_CTRL_ESFE  |
            SAA7134_MAIN_CTRL_EBADC |
            SAA7134_MAIN_CTRL_EBDAC);

    // Disable all interrupts
    WriteDword(SAA7134_IRQ1, 0UL);
    WriteDword(SAA7134_IRQ2, 0UL);

    WriteByte(SAA7134_SPECIAL_MODE, 0x00);

    InitAudio();

    SetTypicalSettings();
    SetupTasks();
}


void CSAA7134Card::SetTypicalSettings()
{
    // Typical settings for video decoder (according to saa7134 manual(v0.1))

    WriteByte(SAA7134_INCR_DELAY,               0x08);
    WriteByte(SAA7134_ANALOG_IN_CTRL1,          0xC0);
    WriteByte(SAA7134_ANALOG_IN_CTRL2,          0x00);
    WriteByte(SAA7134_ANALOG_IN_CTRL3,          0x90);
    WriteByte(SAA7134_ANALOG_IN_CTRL4,          0x90);
    WriteByte(SAA7134_HSYNC_START,              0xEB);
    WriteByte(SAA7134_HSYNC_STOP,               0xE0);

    WriteByte(SAA7134_SYNC_CTRL,                0x98);  // standard adjust
    WriteByte(SAA7134_LUMA_CTRL,                0x40);  // standard adjust
    WriteByte(SAA7134_DEC_LUMA_BRIGHT,          0x80);  // bright 128
    WriteByte(SAA7134_DEC_LUMA_CONTRAST,        0x44);  // contrast 68
    // Saturation: Auto=64, PAL=61, SECAM=64, NTSC=67
    WriteByte(SAA7134_DEC_CHROMA_SATURATION,    0x40);  // saturation 64
    WriteByte(SAA7134_DEC_CHROMA_HUE,           0x00);  // hue 0
    WriteByte(SAA7134_CHROMA_CTRL1,             0x8B);  // standard adjust
    WriteByte(SAA7134_CHROMA_GAIN_CTRL,         0x00);  // standard adjust
    WriteByte(SAA7134_CHROMA_CTRL2,             0x00);  // standard adjust
    WriteByte(SAA7134_MODE_DELAY_CTRL,          0x00);
    WriteByte(SAA7134_ANALOG_ADC,               0x41);
    WriteByte(SAA7134_VGATE_START,              0x11);
    WriteByte(SAA7134_VGATE_STOP,               0xFE);
    WriteByte(SAA7134_MISC_VGATE_MSB,           0x18);  // standard adjust
    WriteByte(SAA7134_RAW_DATA_GAIN,            0x40);
    WriteByte(SAA7134_RAW_DATA_OFFSET,          0x80);
}


void CSAA7134Card::SetupTasks()
{
    // 0x00 = YUV2
    WriteByte(SAA7134_OFMT_VIDEO_A, 0x00);
    WriteByte(SAA7134_OFMT_VIDEO_B, 0x00);
    // 0x06 = raw VBI
    WriteByte(SAA7134_OFMT_DATA_A, 0x06);
    WriteByte(SAA7134_OFMT_DATA_B, 0x06);

    // Let Task A get Odd then Even field, follow by Task B
    // getting Odd then Even field. (Odd is DScaler's Even)

    WriteByte(SAA7134_TASK_CONDITIONS(SAA7134_TASK_A_MASK), 0x0E);
    WriteByte(SAA7134_TASK_CONDITIONS(SAA7134_TASK_B_MASK), 0x0E);

    // handle two fields per task
    WriteByte(SAA7134_FIELD_HANDLING(SAA7134_TASK_A_MASK), 0x02);
    WriteByte(SAA7134_FIELD_HANDLING(SAA7134_TASK_B_MASK), 0x02);

    WriteByte(SAA7134_V_FILTER(SAA7134_TASK_A_MASK), 0x00);
    WriteByte(SAA7134_V_FILTER(SAA7134_TASK_B_MASK), 0x00);

    WriteByte(SAA7134_LUMA_CONTRAST(SAA7134_TASK_A_MASK), 0x40);
    WriteByte(SAA7134_LUMA_CONTRAST(SAA7134_TASK_B_MASK), 0x40);

    WriteByte(SAA7134_CHROMA_SATURATION(SAA7134_TASK_A_MASK), 0x40);
    WriteByte(SAA7134_CHROMA_SATURATION(SAA7134_TASK_B_MASK), 0x40);

    WriteByte(SAA7134_LUMA_BRIGHT(SAA7134_TASK_A_MASK), 0x80);
    WriteByte(SAA7134_LUMA_BRIGHT(SAA7134_TASK_B_MASK), 0x80);

    WriteWord(SAA7134_H_PHASE_OFF_LUMA(SAA7134_TASK_A_MASK), 0x00);
    WriteWord(SAA7134_H_PHASE_OFF_LUMA(SAA7134_TASK_B_MASK), 0x00);
    WriteWord(SAA7134_H_PHASE_OFF_CHROMA(SAA7134_TASK_A_MASK), 0x00);
    WriteWord(SAA7134_H_PHASE_OFF_CHROMA(SAA7134_TASK_B_MASK), 0x00);

    WriteByte(SAA7134_VBI_PHASE_OFFSET_LUMA(SAA7134_TASK_A_MASK), 0x00);
    WriteByte(SAA7134_VBI_PHASE_OFFSET_LUMA(SAA7134_TASK_B_MASK), 0x00);
    WriteByte(SAA7134_VBI_PHASE_OFFSET_CHROMA(SAA7134_TASK_A_MASK), 0x00);
    WriteByte(SAA7134_VBI_PHASE_OFFSET_CHROMA(SAA7134_TASK_B_MASK), 0x00);

    WriteByte(SAA7134_DATA_PATH(SAA7134_TASK_A_MASK), 0x03);
    WriteByte(SAA7134_DATA_PATH(SAA7134_TASK_B_MASK), 0x03);

    // deinterlace y offsets ?? no idea what these are
    // Odds: 0x00 default
    // Evens: 0x00 default + yscale(1024) / 0x20;
    // ---
    // We can tweak these to change the top line to even (by giving
    // even an offset of 0x20) but it doesn't change VBI
    WriteByte(SAA7134_V_PHASE_OFFSET0(SAA7134_TASK_A_MASK), 0x00); // Odd
    WriteByte(SAA7134_V_PHASE_OFFSET1(SAA7134_TASK_A_MASK), 0x00); // Even
    WriteByte(SAA7134_V_PHASE_OFFSET2(SAA7134_TASK_A_MASK), 0x00); // Odd
    WriteByte(SAA7134_V_PHASE_OFFSET3(SAA7134_TASK_A_MASK), 0x00); // Even

    WriteByte(SAA7134_V_PHASE_OFFSET0(SAA7134_TASK_B_MASK), 0x00); // Odd
    WriteByte(SAA7134_V_PHASE_OFFSET1(SAA7134_TASK_B_MASK), 0x00); // Even
    WriteByte(SAA7134_V_PHASE_OFFSET2(SAA7134_TASK_B_MASK), 0x00); // Odd
    WriteByte(SAA7134_V_PHASE_OFFSET3(SAA7134_TASK_B_MASK), 0x00); // Even
}


/*
 *  SetPageTable
 *
 *  If page tables aren't being used, pPhysical should be 0 and
 *  nPages should contain the size of the memory block that will be
 *  used with this channel.
 *
 *  RegionID is one of: REGIONID_VIDEO_A, REGIONID_VIDEO_B
 *                      REGIONID_VBI_A, REGIONID_VBI_B
 */
void CSAA7134Card::SetPageTable(eRegionID RegionID, DWORD pPhysical, DWORD nPages)
{
    int Channel = RegionID2Channel(RegionID);

    if (pPhysical != 0)
    {
        DWORD Page = pPhysical >> 12;

        // ME must be enabled for page tables to work
        WriteByte(SAA7134_RS_CONTROL_0(Channel), Page & 0xFF);
        WriteByte(SAA7134_RS_CONTROL_1(Channel), Page >> 8 & 0xFF);
        WriteByte(SAA7134_RS_CONTROL_2(Channel), Page >> 16 |
            SAA7134_RS_CONTROL_2_ME | SAA7134_RS_CONTROL_2_BURST_MAX);

        m_DMAChannelMemorySize[Channel] = nPages * 4096;
    }
    else
    {
        WriteByte(SAA7134_RS_CONTROL_0(Channel), 0x00);
        WriteByte(SAA7134_RS_CONTROL_1(Channel), 0x00);
        WriteByte(SAA7134_RS_CONTROL_2(Channel), SAA7134_RS_CONTROL_2_BURST_MAX);

        m_DMAChannelMemorySize[Channel] = nPages;
    }
}


void CSAA7134Card::SetBaseOffsets(eRegionID RegionID,
                                      DWORD dwEvenOffset,
                                      DWORD dwOddOffset,
                                      DWORD dwPitch
                                  )
{
    int Channel = RegionID2Channel(RegionID);

    // WARNING!!  SAA7134 thinks the top line is odd but DScaler expects
    //            it to be even.  Do the conversion in here in SAA7134Card.cpp
    //            so we don't confuse ourselves... (I hope)  Make sure everything
    //            we give back to SAA7134Source.cpp has already been converted.
    //            GetProcessingRegion() and GetIRQEventRegion() and should have
    //            the conversion back.

    // Number bytes to offset into every page
    // Give the even offset as odd and odd offset as even
    WriteDword(SAA7134_RS_BA1(Channel), dwEvenOffset);
    WriteDword(SAA7134_RS_BA2(Channel), dwOddOffset);

    // Number of bytes to spend per line
    WriteDword(SAA7134_RS_PITCH(Channel), dwPitch);
}


void CSAA7134Card::SetBSwapAndWSwap(eRegionID RegionID, BOOL bBSwap, BOOL bWSwap)
{
    int Channel = RegionID2Channel(RegionID);

    WriteByte(SAA7134_RS_CONTROL_3(Channel),
            (bBSwap ? SAA7134_RS_CONTROL_3_BSWAP : 0x00) |
            (bWSwap ? SAA7134_RS_CONTROL_3_WSWAP : 0x00)
        );
}


void CSAA7134Card::VerifyMemorySize(eRegionID RegionID)
{
    WORD LinesAvailable;
    WORD BytesPerLine;

    // Only Video and VBI memory size checking is implemented
    if (!IsRegionIDVideo(RegionID) && !IsRegionIDVBI(RegionID))
    {
        return;
    }

    BYTE TaskMask = TaskID2TaskMask(RegionID2TaskID(RegionID));

    if (IsRegionIDVBI(RegionID))
    {
        BytesPerLine = ReadWord(SAA7134_VBI_H_LEN(TaskMask));
    }
    else
    {
        // YUV is 2 bytes per pixel
        BytesPerLine = ReadWord(SAA7134_VIDEO_PIXELS(TaskMask)) * 2;
    }

    LinesAvailable = CalculateLinesAvailable(RegionID, BytesPerLine);

    if (IsRegionIDVBI(RegionID))
    {
        if (LinesAvailable < ReadWord(SAA7134_VBI_V_LEN(TaskMask)))
        {
            WriteWord(SAA7134_VBI_H_LEN(TaskMask), LinesAvailable);
        }
    }
    else
    {
        if (LinesAvailable < ReadWord(SAA7134_VIDEO_LINES(TaskMask)))
        {
            WriteWord(SAA7134_VIDEO_LINES(TaskMask), LinesAvailable);
        }
    }
}


WORD CSAA7134Card::CalculateLinesAvailable(eRegionID RegionID, WORD wBytesPerLine)
{
    int Channel = RegionID2Channel(RegionID);

    DWORD Offset1     = ReadDword(SAA7134_RS_BA1(Channel));
    DWORD Offset2     = ReadDword(SAA7134_RS_BA2(Channel));
    DWORD Pitch         = ReadDword(SAA7134_RS_PITCH(Channel));

    DWORD MaxBaseOffset = (Offset2 > Offset1) ? Offset2 : Offset1;
    DWORD MinimumBytesAvailable;

    MinimumBytesAvailable = m_DMAChannelMemorySize[Channel];

    MinimumBytesAvailable -= MaxBaseOffset;
    if (MinimumBytesAvailable < wBytesPerLine)
    {
        return 0;
    }

    return (MinimumBytesAvailable - wBytesPerLine) / Pitch + 1;
}


void CSAA7134Card::SetDMA(eRegionID RegionID, BOOL bState)
{
    BYTE    Region = 0;
    DWORD   IRQs = 0;
    DWORD   Control = 0;

    switch (RegionID)
    {
    case REGIONID_VIDEO_A:
        Region = SAA7134_REGION_ENABLE_VID_ENA;
        Control = SAA7134_MAIN_CTRL_TE0;
        IRQs = SAA7134_IRQ1_INTE_RA0_0 | SAA7134_IRQ1_INTE_RA0_1;
        break;
    case REGIONID_VIDEO_B:
        Region = SAA7134_REGION_ENABLE_VID_ENB;
        Control = SAA7134_MAIN_CTRL_TE1;
        IRQs = SAA7134_IRQ1_INTE_RA0_2 | SAA7134_IRQ1_INTE_RA0_3;
        break;
    case REGIONID_VBI_A:
        Region = SAA7134_REGION_ENABLE_VBI_ENA;
        Control = SAA7134_MAIN_CTRL_TE2;
        IRQs = SAA7134_IRQ1_INTE_RA0_4 | SAA7134_IRQ1_INTE_RA0_5;
        break;

    case REGIONID_VBI_B:
        Region = SAA7134_REGION_ENABLE_VBI_ENB;
        Control = SAA7134_MAIN_CTRL_TE3;
        IRQs = SAA7134_IRQ1_INTE_RA0_6 | SAA7134_IRQ1_INTE_RA0_7;
        break;
    }

    if (bState) {
        m_PreparedRegions |= Region;
        // Don't turn on interrupts because we don't have an ISR!
        // MaskDataDword(SAA7134_IRQ1, IRQs, IRQs);
        MaskDataDword(SAA7134_MAIN_CTRL, Control, Control);
    }
    else {
        m_PreparedRegions &= ~Region;
        AndDataByte(SAA7134_REGION_ENABLE, ~Region);
        MaskDataDword(SAA7134_IRQ1, 0x00, IRQs);
        MaskDataDword(SAA7134_MAIN_CTRL, 0x00, Control);
    }
}


BOOL CSAA7134Card::GetDMA(eRegionID RegionID)
{
    DWORD Mask = 0;

    switch (RegionID) {
    case REGIONID_VIDEO_A: Mask = SAA7134_MAIN_CTRL_TE0; break;
    case REGIONID_VIDEO_B: Mask = SAA7134_MAIN_CTRL_TE1; break;
    case REGIONID_VBI_A: Mask = SAA7134_MAIN_CTRL_TE2; break;
    case REGIONID_VBI_B: Mask = SAA7134_MAIN_CTRL_TE3; break;
    }

    return (ReadDword(SAA7134_MAIN_CTRL) & Mask) > 0;
}


void CSAA7134Card::StartCapture(BOOL bCaptureVBI)
{
    BYTE Region;

    Region = SAA7134_REGION_ENABLE_VID_ENA | SAA7134_REGION_ENABLE_VID_ENB;

    VerifyMemorySize(REGIONID_VIDEO_A);
    VerifyMemorySize(REGIONID_VIDEO_B);

    if (bCaptureVBI)
    {
        CheckVBIAndVideoOverlap(TASKID_A);
        CheckVBIAndVideoOverlap(TASKID_B);

        VerifyMemorySize(REGIONID_VBI_A);
        VerifyMemorySize(REGIONID_VBI_B);

        Region |= SAA7134_REGION_ENABLE_VBI_ENA | SAA7134_REGION_ENABLE_VBI_ENB;
    }

    MaskDataByte(SAA7134_REGION_ENABLE, Region, m_PreparedRegions);
    MaskDataByte(SAA7134_AUDIO_MUTE_CTRL, 0x00, SAA7134_AUDIO_MUTE_CTRL_MUTSOUT);
}


void CSAA7134Card::StopCapture()
{
    WriteByte(SAA7134_AUDIO_MUTE_CTRL, 0xFF);
    WriteByte(SAA7134_REGION_ENABLE, 0x00);
}


BOOL CSAA7134Card::IsVideoPresent()
{
    WORD CheckMask = SAA7134_STATUS_VIDEO_HLCK;

    if ((ReadWord(SAA7134_STATUS_VIDEO) & CheckMask) == 0)
    {
        return TRUE;
    }

    return FALSE;
}


/*
 * data comes in:
 *       FIDSCI and TASK set
 * data written out:
 *       VID_A / VID_B and FIDSCO set
 *       (DMA status DONE cleared, status buffer toggled)
 * data out finished:
 *       (IRQ fired & DMA status DONE set)
 */

// Gets the field the card is currently processing
BOOL CSAA7134Card::GetProcessingRegion(eRegionID& RegionID, BOOL& bIsFieldOdd)
{
    WORD Status;

    Status  = ReadWord(SAA7134_SCALER_STATUS);

    // This shouldn't usually happen
    if (Status & (SAA7134_SCALER_STATUS_TRERR |
        SAA7134_SCALER_STATUS_CFERR |
        SAA7134_SCALER_STATUS_LDERR |
        SAA7134_SCALER_STATUS_WASRST))
    {
        return FALSE;
    }

    // FIDSCI XOR FIDSCO should be 0
    if (Status & SAA7134_SCALER_STATUS_D6_D5)
    {
        return FALSE;
    }

    if (Status & SAA7134_SCALER_STATUS_VID_A)
    {
        RegionID = REGIONID_VIDEO_A;
    }
    else if (Status & SAA7134_SCALER_STATUS_VID_B)
    {
        RegionID = REGIONID_VIDEO_B;
    }
    else
    {
        return FALSE;
    }

    if (Status & SAA7134_SCALER_STATUS_FIDSCO)
    {
        bIsFieldOdd = FALSE;
    }
    else
    {
        bIsFieldOdd = TRUE;
    }

    // Everything above is SAA7134 style, here we convert evens
    // to odd and odds to even so SAA7137Source can take even
    // has to top line --instead of odd which SAA7137 uses.
    //   ala. CSAACard::SetBaseOffsets()
    bIsFieldOdd = !bIsFieldOdd;

    return TRUE;
}


/*
 *  I2C Stuff
 */

BYTE CSAA7134Card::GetI2CStatus()
{
    return ReadByte(SAA7134_I2C_ATTR_STATUS) & 0x0F;
}


void CSAA7134Card::SetI2CStatus(BYTE Status)
{
    MaskDataByte(SAA7134_I2C_ATTR_STATUS, Status, 0x0F);
}


void CSAA7134Card::SetI2CCommand(BYTE Command)
{
    MaskDataByte(SAA7134_I2C_ATTR_STATUS, Command, 0xC0);
}


void CSAA7134Card::SetI2CData(BYTE Data)
{
    WriteByte(SAA7134_I2C_DATA, Data);
}


BYTE CSAA7134Card::GetI2CData()
{
    return ReadByte(SAA7134_I2C_DATA);
}


/*
 * ACPI STUFF
 */

// this functions returns 0 if the card is in ACPI state D0 or error
// returns 3 if in D3 state (full off)
int CSAA7134Card::GetACPIStatus()
{
    PCI_COMMON_CONFIG PCI_Config;

    // all new chips should be new enough to have power management
    if(GetPCIConfig(&PCI_Config, m_BusNumber, m_SlotNumber))
    {
        DWORD ACPIStatus = PCI_Config.DeviceSpecific[0x10] & 3;

        LOG(1, "SAA7134 ACPI status: D%d", ACPIStatus);
        return ACPIStatus;
    }

    return 0;
}


// Set ACPIStatus to 0 for D0/full on state. 3 for D3/full off
void CSAA7134Card::SetACPIStatus(int ACPIStatus)
{
    PCI_COMMON_CONFIG PCI_Config;

    if(!GetPCIConfig(&PCI_Config, m_BusNumber, m_SlotNumber))
    {
        return;
    }
    PCI_Config.DeviceSpecific[0x10] &= ~3;
    PCI_Config.DeviceSpecific[0x10] |= ACPIStatus;

    LOG(1, "Attempting to set SAA7134 ACPI status to D%d", ACPIStatus);

    SetPCIConfig(&PCI_Config, m_BusNumber, m_SlotNumber);

    if(ACPIStatus == 0)
    {
        // wait half a second to start the hardware
        ::Sleep(500);
        // reset the chip
        // \todo don't know how to reset
    }
    LOG(1, "Set SAA7134 ACPI status complete");
}


/*
 *
 */

BOOL APIENTRY CSAA7134Card::ChipSettingProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
    CSAA7134Card* pThis = NULL;
    char szCardId[9] = "n/a     ";
    char szVendorId[9] = "n/a ";
    char szDeviceId[9] = "n/a ";
    DWORD dwCardId(0);

    switch (message)
    {
    case WM_INITDIALOG:
        pThis = (CSAA7134Card*)lParam; 

        SetDlgItemText(hDlg, IDC_BT_CHIP_TYPE, pThis->GetChipType());

        sprintf(szVendorId,"%04X", pThis->GetVendorId());
        SetDlgItemText(hDlg, IDC_BT_VENDOR_ID, szVendorId);

        sprintf(szDeviceId,"%04X", pThis->GetDeviceId());
        SetDlgItemText(hDlg, IDC_BT_DEVICE_ID, szDeviceId);

        SetDlgItemText(hDlg, IDC_TUNER_TYPE, pThis->GetTunerType());

        if (pThis->m_DeviceId == 0x7134)
        {
            SetDlgItemText(hDlg, IDC_AUDIO_DECODER_TYPE, "SAA7134 Onchip");
        }

        dwCardId = pThis->GetSubSystemId();
        if(dwCardId != 0 && dwCardId != 0xffffffff)
        {
            sprintf(szCardId, "x%08X", dwCardId);
            SetDlgItemText(hDlg, IDC_AUTODECTECTID, szCardId);
        }
        SetDlgItemText(hDlg, IDC_TEXT18, "YUV2");
        
        LPCSTR pCPUTypeString;
        if (CpuFeatureFlags & FEATURE_SSE2)
        {
            pCPUTypeString = "SSE2";
        }
        else if (CpuFeatureFlags & FEATURE_SSE)
        {
            pCPUTypeString = "SSE";
        }
        else if (CpuFeatureFlags & FEATURE_MMXEXT)
        {
            pCPUTypeString = "MMXEXT";
        }
        else if (CpuFeatureFlags & FEATURE_3DNOWEXT)
        {
            pCPUTypeString = "3DNOWEXT";
        }
        else if (CpuFeatureFlags & FEATURE_3DNOW)
        {
            pCPUTypeString = "3DNOW";
        }
        else
        {
            pCPUTypeString = "MMX";
        }
        SetDlgItemText(hDlg, IDC_CPU_TYPE, pCPUTypeString);
        break;

    case WM_COMMAND:
        if ((LOWORD(wParam) == IDOK) || (LOWORD(wParam) == IDCANCEL))
        {
            EndDialog(hDlg, TRUE);
        }
        break;
    }

    return (FALSE);
}


LPCSTR CSAA7134Card::GetChipType()
{
    switch (m_DeviceId)
    {
    case 0x7134:
        return "SAA7134";
    }
    return "n/a";
}


void CSAA7134Card::SetCardType(int CardType)
{
    if(m_CardType != CardType)
    {
        m_CardType = (eTVCardId)CardType;

        // perform card specific init
        if(m_TVCards[m_CardType].pInitCardFunction != NULL)
        {
            // call correct function
            // this funny syntax is the only one that works
            // if you want help understanding what is going on
            // I suggest you read http://www.newty.de/
            (*this.*m_TVCards[m_CardType].pInitCardFunction)();
        }
    }
}


eTVCardId CSAA7134Card::GetCardType()
{
    return m_CardType;
}


LPCSTR CSAA7134Card::GetTunerType()
{
    return m_TunerType;
}


/*
 *  UNUSED STUFF
 */

// \CURRENTLY UNUSED
// Gets the field the card just finished processing (if IRQs work)
BOOL CSAA7134Card::GetIRQEventRegion(eRegionID& RegionID, BOOL& bIsFieldOdd)
{
    DWORD Report;
    DWORD Status;

    Report = ReadDword(SAA7134_IRQ_REPORT);
    Status = ReadDword(SAA7134_IRQ_STATUS);
    WriteDword(SAA7134_IRQ_REPORT, Report);

    if (Report == 0)
    {
        return FALSE;
    }

    BOOL bIsTaskA = (Status & 0x20);

    if (Status & 0x40)
    {
        RegionID = bIsTaskA ? REGIONID_VBI_A : REGIONID_VBI_B;
    }
    else
    {
        RegionID = bIsTaskA ? REGIONID_VIDEO_A : REGIONID_VIDEO_B;
    }

    bIsFieldOdd = (Status & 0x10);

    // Everything above is SAA7134 style, here we convert evens
    // to odd and odds to even so SAA7137Source can take even
    // has to top line --instead of odd which SAA7137 uses.
    //   ala. CSAACard::SetBaseOffsets()
    bIsFieldOdd = !bIsFieldOdd;

    return TRUE;
}


// \CURRENTLY UNUSED
// Unused - v4l2 uses this if i2s_rate exists on the card
void CSAA7134Card::EnableI2SAudioOutput(WORD wRate)
{
    // set rate
    MaskDataByte(SAA7134_SIF_SAMPLE_FREQ, wRate == 32000 ? 0x01 : 0x03, 0x03);

    // enable I2S output -- no idea
    WriteByte(SAA7134_DSP_OUTPUT_SELECT,    0x80);
    WriteByte(SAA7134_I2S_OUTPUT_SELECT,    0x80);
    WriteByte(SAA7134_I2S_OUTPUT_FORMAT,    0x01);
    WriteByte(SAA7134_I2S_OUTPUT_LEVEL,     0x00);  
    WriteByte(SAA7134_I2S_AUDIO_OUTPUT,     0x01);
}


// \CURRENTLY UNUSED
// Unused, (v4l2 checks video_out existance in saa7134-cards.c)
void CSAA7134Card::EnableCCIR656VideoOut()
{
    // enable video output for CCIR656
    WriteByte(SAA7134_VIDEO_PORT_CTRL0, 0x00);
    WriteByte(SAA7134_VIDEO_PORT_CTRL1, 0xB1);
    WriteByte(SAA7134_VIDEO_PORT_CTRL2, 0x00);
    WriteByte(SAA7134_VIDEO_PORT_CTRL3, 0xA1);
    WriteByte(SAA7134_VIDEO_PORT_CTRL4, 0x00);
    WriteByte(SAA7134_VIDEO_PORT_CTRL5, 0x04);
    WriteByte(SAA7134_VIDEO_PORT_CTRL6, 0x06);
    WriteByte(SAA7134_VIDEO_PORT_CTRL7, 0x00);
    WriteByte(SAA7134_VIDEO_PORT_CTRL8, 0x00);
}



/*
 *  TEMPORALLY DEBUGGING STUFF
 */

// \TEMP DEBUG
void CSAA7134Card::DumpRegisters()
{
    DWORD   Data;
    char    HexString[64];

    for (int i = 0x000; i < 0x400; i += 16)
    {
        *HexString = '\0';

        Data = ReadDword(i);
        sprintf(HexString, "%02x%02x %02x%02x",
            Data & 0xFF, (Data >> 8) & 0xFF, (Data >> 16) & 0xFF, (Data >> 24) & 0xFF);

        Data = ReadDword(i + 4);
        sprintf(HexString, "%s %02x%02x %02x%02x", HexString,
            Data & 0xFF, (Data >> 8) & 0xFF, (Data >> 16) & 0xFF, (Data >> 24) & 0xFF);

        Data = ReadDword(i + 8);
        sprintf(HexString, "%s|%02x%02x %02x%02x", HexString,
            Data & 0xFF, (Data >> 8) & 0xFF, (Data >> 16) & 0xFF, (Data >> 24) & 0xFF);
        
        Data = ReadDword(i + 12);
        sprintf(HexString, "%s %02x%02x %02x%02x", HexString,
            Data & 0xFF, (Data >> 8) & 0xFF, (Data >> 16) & 0xFF, (Data >> 24) & 0xFF);

        LOG(0, "%03lX: %s", i, HexString);
    }
}


// \TEMP DEBUG
// Don't know what this is for, came from v4l2 saa7134 code
void CSAA7134Card::StatGPIO()
{
    MaskDataDword(SAA7134_GPIO_GPMODE, 0UL, SAA7134_GPIO_GPMODE_GPRESCN);
    MaskDataDword(SAA7134_GPIO_GPMODE, SAA7134_GPIO_GPMODE_GPRESCN,
        SAA7134_GPIO_GPMODE_GPRESCN);

    DWORD Mode = ReadDword(SAA7134_GPIO_GPMODE) & 0x0FFFFFFF;
    DWORD Status = ReadDword(SAA7134_GPIO_GPSTATUS) & 0x0FFFFFFF;
    LOG(0, "debug: gpio: mode=0x%07lx in=0x%07lx out=0x%07lx\n", Mode,
            (~Mode) & Status, Mode & Status);
}


// \TEMP DEBUG
void CSAA7134Card::CheckRegisters(DWORD* AOdd, DWORD* AEven, DWORD* BOdd, DWORD* BEven)
{
    static WORD OldScalerStatus = 0;
    static DWORD OldIRQStatus = 0;
    static DWORD OldDMAStatus = 0;
    static DWORD OldAOdd = 0;
    static DWORD OldBOdd = 0;
    static DWORD OldAEven = 0;
    static DWORD OldBEven = 0;

    WORD ScalerStatus  = ReadWord(SAA7134_SCALER_STATUS);
    DWORD IRQStatus = ReadDword(SAA7134_IRQ_STATUS);
    DWORD DMAStatus = (ReadDword(SAA7134_DMA_STATUS) >> 20) & 0x1F;

    if (*AOdd != OldAOdd)
    {
        OldAOdd = *AOdd;

        LOG(0, "A Odd CHANGED");
    }

    if (*BOdd != OldBOdd)
    {
        OldBOdd = *BOdd;

        LOG(0, "B Odd CHANGED");
    }

    if (*AEven != OldAEven)
    {
        OldAEven = *AEven;

        LOG(0, "A Even CHANGED");
    }

    if (*BEven != OldBEven)
    {
        OldBEven = *BEven;

        LOG(0, "B Even CHANGED");
    }

    if (DMAStatus != OldDMAStatus)
    {
        OldDMAStatus = DMAStatus;

        LOG(0, "DMA Status: %02x", DMAStatus);
        //LOG(0, "DMA Status: %x", (DMAStatus >> 20) & 0x0F);
    }

    if (IRQStatus != OldIRQStatus)
    {
        OldIRQStatus = IRQStatus;

        LOG(0, "IRQ Status: %s,%s,%s,%ld",
               (IRQStatus & 0x40) ? "vbi"  : "video",
               (IRQStatus & 0x20) ? "b"    : "a",
               (IRQStatus & 0x10) ? "odd"  : "even",
               (IRQStatus & 0x0f));
    }

    if (ScalerStatus != OldScalerStatus)
    {
        OldScalerStatus = ScalerStatus;

        LOG(0, "Scaler Status: %s%s%s%s%s%s%s%s%s%s%s%s",
            (ScalerStatus & SAA7134_SCALER_STATUS_VID_A) ? "VID_A " : "",
            (ScalerStatus & SAA7134_SCALER_STATUS_VBI_A) ? "VBI_A " : "",
            (ScalerStatus & SAA7134_SCALER_STATUS_VID_B) ? "VID_B " : "",
            (ScalerStatus & SAA7134_SCALER_STATUS_VBI_B) ? "VBI_B " : "",
            (ScalerStatus & SAA7134_SCALER_STATUS_TRERR) ? "TRERR " : "",
            (ScalerStatus & SAA7134_SCALER_STATUS_CFERR) ? "CFERR " : "",
            (ScalerStatus & SAA7134_SCALER_STATUS_LDERR) ? "LDERR " : "",
            (ScalerStatus & SAA7134_SCALER_STATUS_WASRST) ? "WASRST " : "",
            (ScalerStatus & SAA7134_SCALER_STATUS_FIDSCI) ? "FIDSCI " : "",
            (ScalerStatus & SAA7134_SCALER_STATUS_FIDSCO) ? "FIDSCO " : "",
            (ScalerStatus & SAA7134_SCALER_STATUS_D6_D5) ? "D6_D5 " : "",
            (ScalerStatus & SAA7134_SCALER_STATUS_TASK) ? "TASK " : ""
            );
    }
}


// \TEMP DEBUG
BYTE CSAA7134Card::DirectGetByte(DWORD dwAddress)
{
    return ReadByte(dwAddress);
}


// \TEMP DEBUG
void CSAA7134Card::DirectSetBit(DWORD dwAddress, int nBit, BOOL bSet)
{
    if (bSet)
    {
        OrDataByte(dwAddress, (1<<nBit));
        if (dwAddress >= 0x040 || dwAddress < 0x080)
        {
            OrDataByte(dwAddress + 0x040, (1<<nBit));
        }
    }
    else
    {
        AndDataByte(dwAddress, ~(1<<nBit));
        if (dwAddress >= 0x040 || dwAddress < 0x080)
        {
            AndDataByte(dwAddress + 0x040, ~(1<<nBit));
        }
    }
}



/* might need this for future debugging
    static WORD OldStatus = 0;
    Status = ReadWord(SAA7134_STATUS_VIDEO);

    if (Status != OldStatus)
    {
        OldStatus = Status;
        LOG(0, "Video Status: %s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
            (Status & SAA7134_STATUS_VIDEO_DCSTD0) ? "DCSTD0 " : "",
            (Status & SAA7134_STATUS_VIDEO_DCSCT1) ? "DCSCT1 " : "",
            (Status & SAA7134_STATUS_VIDEO_WIPA) ? "WIPA " : "",
            (Status & SAA7134_STATUS_VIDEO_GLIMB) ? "GLIMB " : "",
            (Status & SAA7134_STATUS_VIDEO_GLIMT) ? "GLIMT " : "",
            (Status & SAA7134_STATUS_VIDEO_SLTCA) ? "SLTCA " : "",
            (Status & SAA7134_STATUS_VIDEO_HLCK) ? "HLCK " : "",
            (Status & SAA7134_STATUS_VIDEO_RDCAP) ? "RDCAP " : "",
            (Status & SAA7134_STATUS_VIDEO_COPRO) ? "COPRO " : "",
            (Status & SAA7134_STATUS_VIDEO_COLSTR) ? "COLSTR " : "",
            (Status & SAA7134_STATUS_VIDEO_TYPE3) ? "TYPE3 " : "",
            (Status & SAA7134_STATUS_VIDEO_FIDT) ? "FIDT " : "",
            (Status & SAA7134_STATUS_VIDEO_HLVLN) ? "HLVLN " : "",
            (Status & SAA7134_STATUS_VIDEO_INTL) ? "INTL " : "",
            (Status & (1<<7)) ? "Unknown1 " : "",
            (Status & (1<<12)) ? "Unknown2" : ""
            );
    }
*/
/*
    static WORD OldStatus = 0;
    Status = ReadWord(SAA7134_SCALER_STATUS);

    if (Status != OldStatus)
    {
        OldStatus = Status;
        LOG(0, "Scaler Status: %s%s%s%s%s%s%s%s%s%s%s%s",
            (Status & SAA7134_SCALER_STATUS_VID_A) ? "VID_A " : "",
            (Status & SAA7134_SCALER_STATUS_VBI_A) ? "VBI_A " : "",
            (Status & SAA7134_SCALER_STATUS_VID_B) ? "VID_B " : "",
            (Status & SAA7134_SCALER_STATUS_VBI_B) ? "VBI_B " : "",
            (Status & SAA7134_SCALER_STATUS_TRERR) ? "TRERR " : "",
            (Status & SAA7134_SCALER_STATUS_CFERR) ? "CFERR " : "",
            (Status & SAA7134_SCALER_STATUS_LDERR) ? "LDERR " : "",
            (Status & SAA7134_SCALER_STATUS_WASRST) ? "WASRST " : "",
            (Status & SAA7134_SCALER_STATUS_FIDSCI) ? "FIDSCI " : "",
            (Status & SAA7134_SCALER_STATUS_FIDSCO) ? "FIDSCO " : "",
            (Status & SAA7134_SCALER_STATUS_D6_D5) ? "D6_D5 " : "",
            (Status & SAA7134_SCALER_STATUS_TASK) ? "TASK " : ""
            );
    }
*/


