/////////////////////////////////////////////////////////////////////////////
// Other.h
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

#ifndef __OTHER_H___
#define __OTHER_H___

#include "defines.h"
#include "structs.h"
#include "globals.h"

BOOL APIENTRY ChipSettingProc(HWND hDlg, UINT message, UINT wParam, LONG lParam);

void ExitDD(void);
BOOL InitDD(HWND hWnd);
BOOL OverlayUpdate(LPRECT pSrcRect, LPRECT pDestRect, DWORD dwFlags, BOOL ColorKey);
BOOL CreateOverlay();
void  Black_Surface();
void Black_Overlays();

BOOL LockMemory(LPVOID pLinear, DWORD dwSize);
BOOL UnlockMemory(LPVOID pLinear, DWORD dwSize);
BOOL Init_TV_Karte(HWND hWnd);
BOOL Init_Tuner(int TunerNr);
BOOL Init_BT_HardWare(HWND hWnd);
PHYS RiscLogToPhys(DWORD * pLog);
void MakeVBITable(int nIndex, int VBI_Lines);

void Set_PLL(int PLL);

BOOL SetGeometry(int width, int height, int Type, int ColourFormat);
BOOL SetAudioSource(int nChannel);
void FillRiscJumps(int nFlags);
void SetRiscJumpsDecode(int nFlags);
BOOL SetGeoSize(int wWidth, int wHeight);
BOOL SetBrightness(unsigned char bBrightness);
BOOL SetHue(char bHue);
BOOL SetContrast(int wContrast);
BOOL SetSaturationU(int wData);
BOOL SetSaturationV(int wData);
BOOL SetVideoSource(int nInput);
BOOL SetColourFormat(int nColourFormat);
void SetDMA(BOOL bState);
void MakeVideoTableForDisplay();
void MakeVideoTableForDirectDraw(PMemStruct dma, int Pitch, int PosOffset);

BOOL Audio_WriteMSP(BYTE bSubAddr, int wAddr, int wData);
BOOL Audio_SetVolume(int nVolume);
BOOL Audio_SetBalance(char nBalance);
BOOL Audio_SetBass(char nBass);
BOOL Audio_SetTreble(char nTreble);
BOOL Audio_SetLoudness(BYTE nLoudness);
BOOL Audio_SetSpatial(char nSpatial);
BOOL Audio_SetSuperBass(BOOL bSuperBass);
void Audio_SetToneControl(BOOL nMode);
BOOL Audio_SetEqualizer(int nIndex, char nLevel);

int Audio_GetStereoMode(BOOL bHardwareMode);
void Load_Country_Settings();
void Load_Country_Specific_Settings(int LPos);
BOOL VideoPresent();

void WinBis_daten_ausgabe(void);
void WinBis_push_data(unsigned char *dat);
void winbis_decoder(unsigned char *dat);

//Bus
BOOL I2CBus_AddDevice(BYTE I2C_Port);
BOOL I2CBus_Lock();
BOOL I2CBus_Unlock();
void I2CBus_Start();
void I2CBus_Stop();
void I2CBus_One();
void I2CBus_Zero();
BOOL I2CBus_Ack();
BOOL I2CBus_SendByte(BYTE nData, int nWaitForAck);
BYTE I2CBus_ReadByte(BOOL bLast);
BYTE I2CBus_Read(BYTE nAddr);
BOOL I2CBus_Write(BYTE nAddr, BYTE nData1, BYTE nData2, BOOL bSendBoth);
void I2CBus_wait(int us);
void I2C_SetLine(BOOL bCtrl, BOOL bData);
BOOL I2C_GetLine();
BYTE I2C_Read(BYTE nAddr);
BOOL I2C_Write(BYTE nAddr, BYTE nData1, BYTE nData2, BOOL bSendBoth);
BOOL Tuner_SetFrequency(int TunerTyp, int wFrequency);
BOOL Tuner_SetChannel(int nChannel, BOOL bNotify);
BOOL Tuner_Scan();

BOOL Init_Audio(BYTE DRead, BYTE DWrite);
void Audio_Autodetect();

BOOL MSP_Reset();
BOOL MSP_Version();
void MSP_Set_MajorMinor_Mode(int MajorMode, int MinorMode);
void MSP_SetMode(int type);
void MSP_SetStereo(int MajorMode, int MinorMode, int mode);
void MSPWatch_Mode();

void SAA7146_SetColourFormat(int nColourFormat);
void video_setmode(int v);
void set_up_grabbing();

BOOL LoadDeviceDriver( const TCHAR * Name, const TCHAR * Path, HANDLE * lphDevice,BOOL Install);
BOOL InstallDriver( IN SC_HANDLE SchSCManager, IN LPCTSTR DriverName, IN LPCTSTR ServiceExe );
BOOL UnloadDeviceDriver( const TCHAR * Name,BOOL DRemove );
BOOL StartDriver( IN SC_HANDLE SchSCManager, IN LPCTSTR DriverName );
BOOL StopDriver( IN SC_HANDLE SchSCManager, IN LPCTSTR DriverName );
BOOL RemoveDriver( IN SC_HANDLE SchSCManager, IN LPCTSTR DriverName );

int Get_Tuner_Status( void );
int Get_CI_Status( void );

#define GetBit(val,bit,mask) (BYTE)(((val)>>(bit))&(mask))

void Get_Thread_Status();
void InterCast_Init();
void InterCast_Exit();
BOOL Init_BT_Kernel_Memory( void );
void Reset_BT_HardWare();
BOOL Init_Memory();
BOOL SetGeoSize(int wWidth, int wHeight);
BOOL SetAudioSource(int nChannel);
BOOL SetVideoSource(int nInput);
BOOL Tuner_Scan();
BOOL SetColourFormat(int nColourFormat);
void Load_Country_Settings();
void Load_Country_Specific_Settings(int LPos);
BOOL Init_Tuner(int TunerNr );
void Winbis_Exit( void );
void Init_Winbis( void );

BOOL APIENTRY IFormSettingProc(HWND hDlg,UINT message,UINT wParam,LONG lParam);
BOOL APIENTRY PLLSettingProc(HWND hDlg,UINT message,UINT wParam,LONG lParam);
BOOL APIENTRY TunerSettingProc(HWND hDlg,UINT message,UINT wParam,LONG lParam);
BOOL APIENTRY RecordPlay(HWND hDlg,UINT message,UINT wParam,LONG lParam);
BOOL APIENTRY CardSettingProc(HWND hDlg,UINT message,UINT wParam,LONG lParam);
void Free_Display_DMA(int NR);
void MSP_Print_Mode();

#endif