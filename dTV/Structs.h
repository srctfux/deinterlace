/////////////////////////////////////////////////////////////////////////////
// structs,h
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

#ifndef __STRUCTS_H___
#define __STRUCTS_H___

struct TInterCast
{
	unsigned char blocks[28*16];
	unsigned char data[0x100];
	unsigned char *fbuf;
	unsigned int packnum, packtot, packlen, total;
	int datap, ok, esc, ciok;
	unsigned char ci,lastci;  /* ci = continuity index */
	unsigned short dat;
	unsigned int length;
	unsigned int done, flength;
	int pnum;
	char *name;
	int fbufSize;
	char Error[64];
	char fname[512];
	unsigned int BytesSoFar;
};

struct TVDat
{
	int Status;
	int LastKillPos;
    int Interleave;
	int Lenght;
	int Pos;
	int Blockzaehler;
	int CRCError;
	int BlockSoFar;
    unsigned char eLBN;
	unsigned char SOB1;
    unsigned char SOB2;
    unsigned char ADR;
    unsigned char SYS;
    unsigned char LBN;
    unsigned char KEY;
    unsigned char XDATA[1024];   //ARRAY of Byte...
    unsigned char CSL;
    unsigned char CSH;
    int Fehler_Frei;
	char FileName[13];
	unsigned int FileSize;
	FILE *FilePtr;

    int Index;
	char RawName[128];
	unsigned int BlocksOK;
	unsigned int BlocksError;
	char Error[512];
};


struct TVDatBlockz
{
    unsigned char SOB1;
    unsigned char SOB2;
    unsigned char ADR;
    unsigned char SYS;
    unsigned char LBN;
    unsigned char KEY;
    unsigned char XDATA[1024];   //ARRAY of Byte...
    int  pos;           //Aktuelle Position im Bytestream
    int  Lenght;        //Laenge des Datenrumpfes
    int  interleave;    //WH Zdhler...
    unsigned char CSL;
    unsigned char CSH;
    int Fehler_Frei;
};


//Struktur für den Block SOT - Start of Transmission   !!SF
typedef struct SOTREC 
{
	unsigned int  date;   //Datum der Ausstrahlung...
    unsigned short fanz;          //Anzahl der übertragenden Dateien.
 	char           GeneralName[512];  
	char           ExecCommand[512];  
    char           Kommentar[512];    
};

typedef struct TCountries
{
        // Device data
        char Name[128];
};

typedef struct TVTDialog
{
	HWND Dialog;
	int  Page;
	int  SubPage;
	int  FramePos;
	BOOL Large;
	BOOL PageChange;
	unsigned char AsciiBuffer[26][40];
};

struct PIDFilters
{
	char FilterName[5];
	unsigned char FilterId;
	unsigned short PID;
};

typedef struct TProgramm
{
	char Name[30];
	char Anbieter[30];
	char Land[30];
    unsigned long freq;
    char Typ;
    BOOL  Tuner_Auto;
	BOOL  PMT_Auto;
	BOOL  PID_Auto;
    int power;             /* LNB power 0=off/pass through, 1=on */
	int volt;              /* 14/18V (V=0/H=1) */
	int afc;
	int ttk;               /* 22KHz */
	int diseqc;            /* Diseqc input select */
	unsigned int srate;         
	int qam;               
	int fec;   
	int norm;
	unsigned short  tp_id;        
	unsigned short  Video_pid;        
	unsigned short  Audio_pid;
    unsigned short  TeleText_pid;          /* Teletext PID */
	unsigned short  PMT_pid;
    unsigned short  PCR_pid;
	unsigned short  PMC_pid;
	unsigned short  SID_pid;
	unsigned short  AC3_pid;
	unsigned short  EMM_pid;
	unsigned short  ECM_pid;
	unsigned char   TVType; //  == 00 PAL ; 11 == NTSC    
	unsigned char   ServiceTyp;
    unsigned char   CA_ID;
	unsigned short  Temp_Audio;
    unsigned char   Buffer[10];   // For later Use
	unsigned short  Filteranzahl;
    struct PIDFilters Filters[12];
};

struct TPacket30
{
	// Type 0 data

	struct
	{
		unsigned nMag:3;
		unsigned nPage:8;
		unsigned:5; // Unused
		WORD nSubcode;
	} HomePage;
	WORD NetId;
	struct
	{
		char Offset;
		DWORD JulianDay;
		BYTE Hour;
		BYTE Min;
		BYTE Sec;
	} UTC;
	char Unknown[5];
	char Identifier[21];

	// Type 2 data
	
	struct
	{
		unsigned LCI:2;
		unsigned LUF:1;
		unsigned PRF:1;
		unsigned PCS:2;
		unsigned MI:1;
		unsigned day:5;
		unsigned month:4;
		unsigned hour:5;
		unsigned minute:6;
		unsigned:5;
		WORD CNI;
		WORD PTY;
	} PDC;
};

typedef struct TVTPage
{
	WORD wCtrl;
	BOOL Fill;
	BYTE Frame[25][40];
    BYTE LineUpdate[25];
	BYTE bUpdated;
};

typedef struct TVT
{
    unsigned short SubCount;
	struct TVTPage *SubPage;
};

struct TTVSetting
{
	WORD wCropWidth;
	WORD wCropHeight;
	WORD wTotalWidth;
	BYTE bDelayA;
	BYTE bDelayB;
	BYTE bIForm;
	DWORD dwXsfNum;
	DWORD dwXsfDen;
	WORD wHDelayx1;
	WORD wHActivex1;
	WORD wVDelay;
	WORD wCropOffset;
};

struct TTunerType
{
	WORD thresh1; /* frequency Range for UHF,VHF-L, VHF_H */   
	WORD thresh2;  
	BYTE VHF_L;
	BYTE VHF_H;
	BYTE UHF;
	BYTE config; 
	BYTE I2C;
	WORD IFPCoff;
};



typedef struct TChannels
{
	char Name[128];
	int MinChannel;
	int MaxChannel;
	unsigned long freq[512];
};


typedef unsigned long PHYS;

struct TMixerAccess
{
    int SoundSystem;
    int Destination;
    int Connection;
    int Control;
};

struct TMixerValues
{
    int Kanal1;
    int Kanal2;
    int Kanal3;
    int Kanal4;
};


struct TMixerLoad 
{
	struct TMixerAccess MixerAccess;
	struct TMixerValues MixerValues;
};

struct TMixerControls
{
	int AnzahlControls;
	MIXERCONTROL          *MixerControl;
	MIXERCONTROLDETAILS   *MixerDetail;
};

struct TBL
{
	short FeldId;
	unsigned short x;
	unsigned short s;
	unsigned short r;
};

struct TMixerConnections
{
	int AnzahlConnections;
	MIXERLINE *MixerConnections;
	struct TMixerControls *To_Control;
};

struct TMixerLines 
{
	int AnzahlLines;
	MIXERLINE *MixerLine;
	struct TMixerConnections *To_Connection;
};

struct TSoundSystem
{
	int DeviceAnzahl;
	MIXERCAPS *MixerDev;
	struct TMixerLines *To_Lines;
};


typedef struct OTLNB 
{
	BOOL Use;
	BOOL Power;
	BOOL Switch22khz;
	unsigned int MinFreq;
	unsigned int MaxFreq;
	unsigned int LofLow;
	unsigned int LofHigh;
	unsigned int SwitchFreq;
};

struct TLNB
{
	BOOL Diseq;
	struct OTLNB Anschluss[4];
};

struct TUT
{
	char Zeile[240];
    short Fg;
	short Bg;
};

struct fmt_char
{
    unsigned char ch;
	unsigned char fg;
	unsigned char bg;
	unsigned char attr;
};

struct fmt_page
{
    unsigned int dbl;
	unsigned int hid;
    struct fmt_char data[25][40];
};

#endif



