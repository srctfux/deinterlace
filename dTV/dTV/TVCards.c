/////////////////////////////////////////////////////////////////////////////
// TVCards.c
/////////////////////////////////////////////////////////////////////////////
// The structures where taken from bttv driver version 7.37
// bttv - Bt848 frame grabber driver
//
// Copyright (C) 1996,97,98 Ralph  Metzler (rjkm@thp.uni-koeln.de)
//                         & Marcus Metzler (mocm@thp.uni-koeln.de)
// (c) 1999,2000 Gerd Knorr <kraxel@goldbach.in-berlin.de>
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
// Date          Developer                Changes
//
// 15 Aug 2000   John Adcock             Added structures from bttv
// 20 Nov 2000   Michael Eskin, Conexant Added support for Conexant and Rockwell Bt878XEVKs
//  5 Dec 2000   Michael Eskin, Conexant Added support for Conexant Foghorn ATSC reference designs 
//                                       and Philips 1236D tuner
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "tvcards.h"
#include "bt848.h"
#include "i2c.h"
#include "OutThreads.h"
#include "FD_50Hz.h"
#include "FD_60Hz.h"

TVCARDID CardType = TVCARD_UNKNOWN;
TVTUNERID TunerType = TUNER_ABSENT;
long ProcessorSpeed = 1;
long TradeOff = 1;

void hauppauge_boot_msp34xx();
void init_PXC200();

const TVCARDSETUP TVCards[TVCARD_LASTONE] =
{
	{
		"Unknown Card",
		3, 1, 0, 2, 0,
		{ 2, 3, 1, 1, 0, 0, 0, 0},
		{ 0, 0, 0, 0, 0, 0},
		0,
		1,1,1,1,0,0,0,1,
		PLL_NONE,
		TUNER_USER_SETUP
	},
	{
		"MIRO PCTV",
		4, 1, 0, 2, 15,
		{ 2, 3, 1, 1, 0, 0, 0, 0},
		{ 2, 0, 0, 0, 10, 0},
		0,
		1,1,1,1,0,0,0,1,
		PLL_NONE,
		TUNER_AUTODETECT
	},
	{
		"Hauppauge old",
		4, 1, 0, 2, 7,
		{ 2, 3, 1, 1, 0, 0, 0, 0},
		{ 0, 1, 2, 3, 4, 0},
		0,
		1,1,0,1,0,0,0,1,
		PLL_NONE,
		TUNER_USER_SETUP
	},
	{
		"STB",
		3, 1, 0, 2, 7,
		{ 2, 3, 1, 1, 0, 0, 0, 0},
		{ 4, 0, 2, 3, 1, 0},
		0,
		0,1,1,1,1,0,0,1,
		PLL_NONE,
		TUNER_USER_SETUP
	},
	{
		"Intel",
		3, 1, 0, -1, 7,
		{ 2, 3, 1, 1, 0, 0, 0, 0},
		{ 0, 1, 2, 3, 4, 0},
		0,
		1,1,1,1,0,0,0,1,
		PLL_NONE,
		TUNER_USER_SETUP
	},
	{
		"Diamond DTV2000",
		3, 1, 0, 2, 3,
		{ 2, 3, 1, 1, 0, 0, 0, 0},
		{ 0, 1, 0, 1, 3, 0},
		0,
		1,1,1,1,0,0,0,1,
		PLL_NONE,
		TUNER_USER_SETUP
	},
	{
		"AVerMedia TVPhone",
		3, 1, 0, 3, 15,
		{ 2, 3, 1, 1, 0, 0, 0, 0},
		{12, 4, 11, 11, 0, 0},
		0,
		1,1,1,1,0,0,0,1,
		PLL_NONE,
		TUNER_USER_SETUP
	},
	{
		"MATRIX-Vision MV-Delta",
		5, 1, -1, 3, 0,
		{ 2, 3, 1, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0},
		0,
		1,1,1,1,0,0,0,1,
		PLL_NONE,
		TUNER_USER_SETUP
	},
	/* 0x08 */
	{
		"Fly Video II",
		3, 1, 0, 2, 0xc00,
		{ 2, 3, 1, 1, 0, 0, 0, 0},
		{ 0, 0xc00, 0x800, 0x400, 0xc00, 0},
		0,
		1,1,1,1,0,0,0,1,
		PLL_NONE,
		TUNER_USER_SETUP
	},
	{
		"TurboTV",
		3, 1, 0, 2, 3,
		{ 2, 3, 1, 1, 0, 0, 0, 0},
		{ 1, 1, 2, 3, 0, 0},
		0,
		1,1,1,1,0,0,0,1,
		PLL_NONE,
		TUNER_USER_SETUP
	},
	{
		"Standard BT878",
		4, 1, 0, 2, 7,
		{ 2, 0, 1, 1, 0, 0, 0, 0},
		{ 0, 1, 2, 3, 4, 0},
		0,
		1,1,0,1,0,0,0,1,
		PLL_28,
		TUNER_USER_SETUP
	},
	{
		"MIRO PCTV pro",
		3, 1, 0, 2, 65551,
		{ 2, 3, 1, 1, 0, 0, 0, 0},
		{1, 65537, 0, 0, 10, 0},
		0,
		1,1,1,1,0,0,0,1,
		PLL_NONE,
		TUNER_AUTODETECT
	},
	{
		"ADS Technologies Channel Surfer TV",
		3, 1, 2, 2, 15,
		{ 2, 3, 1, 1, 0, 0, 0, 0},
		{ 13, 14, 11, 7, 0, 0},
		0,
		1,1,1,1,0,0,0,1,
		PLL_NONE,
		TUNER_USER_SETUP
	},
	{
		"AVerMedia TVCapture 98",
		3, 4, 0, 2, 15,
		{ 2, 3, 1, 1, 0, 0, 0, 0},
		{ 13, 14, 11, 7, 0, 0},
		0,
		1,1,1,1,0,0,0,1,
		PLL_28,
		TUNER_USER_SETUP
	},
	{
		"Aimslab VHX",
		3, 1, 0, 2, 7,
		{ 2, 3, 1, 1, 0, 0, 0, 0},
		{ 0, 1, 2, 3, 4, 0},
		0,
		1,1,1,1,0,0,0,1,
		PLL_NONE,
		TUNER_USER_SETUP
	},
	{
		"Zoltrix TV-Max",
		3, 1, 0, 2, 15,
		{ 2, 3, 1, 1, 0, 0, 0, 0},
		{0, 0, 1 , 0, 10, 0},
		0,
		1,1,1,1,0,0,0,1,
		PLL_NONE,
		TUNER_USER_SETUP
	},
	/* 0x10 */
	{
		"Pixelview PlayTV (bt878)",
		3, 1, 0, 2, 0x01fe00,
		{ 2, 3, 1, 1, 0, 0, 0, 0},
		{ 0x01c000, 0, 0x018000, 0x014000, 0x002000, 0 },
		0,
		1,1,1,1,0,0,0,1,
		PLL_28,
		TUNER_USER_SETUP
	},
	{
		"Leadtek WinView 601",
		3, 1, 0, 2, 0x8300f8,
		{ 2, 3, 1, 1, 0, 0, 0, 0},
		{ 0x4fa007,0xcfa007,0xcfa007,0xcfa007,0xcfa007,0xcfa007},
		0,
		1,1,1,1,0,0,0,1,
		PLL_NONE,
		TUNER_USER_SETUP
	},
	{
		"AVEC Intercapture",
		3, 2, 0, 2, 0,
		{2, 3, 1, 1, 0, 0, 0, 0},
		{1, 0, 0, 0, 0, 0},
		0,
		1,1,1,1,0,0,0,1,
		PLL_NONE,
		TUNER_USER_SETUP
	},
	{
		"LifeView FlyKit w/o Tuner",
		3, 1, -1, -1, 0x8dff00,
		{ 2, 3, 1, 1, 0, 0, 0, 0},
		{ 0, 0, 0, 0, 0},
		0,
		0,0,0,0,0,0,0,1,
		PLL_NONE,
		TUNER_USER_SETUP
	},

	{
		"CEI Raffles Card",
		3, 3, 0, 2, 0,
		{2, 3, 1, 1, 0, 0, 0, 0},
		{0, 0, 0, 0 ,0, 0},
		0,
		1,1,1,1,0,0,0,1,
		PLL_NONE,
		TUNER_USER_SETUP
	},
	{
		"Lucky Star Image World ConferenceTV",
		3, 1, 0, 2, 0x00fffe07,
		{ 2, 3, 1, 1, 0, 0, 0, 0},
		{ 131072, 1, 1638400, 3, 4, 0},
		0,
		1,1,1,1,0,0,0,1,
		PLL_28,
		TUNER_PHILIPS_PAL_I
	},
	{
		"Phoebe Tv Master + FM",
		3, 1, 0, 2, 0xc00,
		{ 2, 3, 1, 1, 0, 0, 0, 0},
		{0, 1, 0x800, 0x400, 0xc00, 0},
		0,
		1,1,1,1,0,0,0,1,
		PLL_NONE,
		TUNER_USER_SETUP
	},
	{
		"Modular Technology MM205 PCTV, bt878",
		2, 1, 0, -1, 7,
		{ 2, 3 , 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0},
		0,
		1,1,1,1,0,0,0,1,
		PLL_NONE,
		TUNER_USER_SETUP
	},
	/* 0x18 */
	{
		"Askey/Typhoon/Anubis Magic TView CPH051/061 (bt878)",
		3, 1, 0, 2, 0xe00,
		{ 2, 3, 1, 1, 0, 0, 0, 0},
		{0x400, 0x400, 0x400, 0x400, 0},
		0,
		1,1,1,1,0,0,0,1,
		PLL_28,
		TUNER_USER_SETUP
	},
	{
		"Terratec/Vobis TV-Boostar",
		3, 1, 0, 2, 16777215,
		{ 2, 3, 1, 1, 0, 0, 0, 0},
		{131072, 1, 1638400, 3, 4, 0},
		0,
		1,1,1,1,0,0,0,1,
		PLL_NONE,
		TUNER_USER_SETUP
	},
	{
		"Newer Hauppauge WinCam (bt878)",
		4, 1, 0, 3, 7,
		{ 2, 0, 1, 1, 0, 0, 0, 0},
		{ 0, 1, 2, 3, 4, 0},
		0,
		1,1,1,1,0,0,0,1,
		PLL_NONE,
		TUNER_USER_SETUP
	},
	{
		"MAXI TV Video PCI2",
		3, 1, 0, 2, 0xffff,
		{ 2, 3, 1, 1, 0, 0, 0, 0},
		{ 0, 1, 2, 3, 0xc00, 0},
		0,
		1,1,1,1,0,0,0,1,
		PLL_NONE,
		TUNER_PHILIPS_SECAM
	},
	{
		"Terratec TerraTV+",
		3, 1, 0, 2, 0x70000,
		{ 2, 3, 1, 1, 0, 0, 0, 0},
		{ 0x20000, 0x30000, 0x00000, 0x10000, 0x40000, 0x00000},
		0,
		1,1,1,1,0,0,0,1,
		PLL_NONE,
		TUNER_USER_SETUP
	},
	{
		"Imagenation PXC200",
		5, 1, -1, 4, 0,
		{ 2, 3, 1, 0, 0, 0, 0, 0},
		{ 0, 0, 0, 0, 0, 0},
		0,
		1,1,1,1,0,0,0,1,
		PLL_NONE,
		TUNER_USER_SETUP
	},
	{
		"FlyVideo 98",
		3, 1, 0, 2, 0x8dff00,
		{2, 3, 1, 1, 0, 0, 0, 0},
		{ 0, 0x8dff00, 0x8df700, 0x8de700, 0x8dff00, 0 },
		0,
		1,1,1,1,0,0,0,1,
		PLL_NONE,
		TUNER_USER_SETUP
	},
	{
		"iProTV",
		3, 1, 0, 2, 1,
		{ 2, 3, 1, 1, 0, 0, 0, 0},
		{ 1, 0, 0, 0, 0, 0},
		0,
		1,1,1,1,0,0,0,1,
		PLL_NONE,
		TUNER_USER_SETUP
	},
	/* 0x20 */
	{
		"Intel Create and Share PCI",
		4, 1, 0, 2, 7,
		{ 2, 3, 1, 1, 0, 0, 0, 0},
		{ 4, 4, 4, 4, 4, 4},
		0,
		1,1,1,1,0,0,0,1,
		PLL_NONE,
		TUNER_USER_SETUP
	},
	{
		"Terratec TerraTValue",
		3, 1, 0, 2, 0xffff00,
		{ 2, 3, 1, 1, 0, 0, 0, 0},
		{ 0x500, 0, 0x300, 0x900, 0x900, 0},
		0,
		1,1,1,1,0,0,0,1,
		PLL_28,
		TUNER_USER_SETUP
	},
	{
		"Leadtek WinFast 2000",
		3, 1, 0, 2, 0xfff000,
		{ 2, 3, 1, 1, 0, 0, 0, 0},
		{ 0x621000,0x620100,0x621100,0x620000,0xE210000,0x620000},
		0,
		1,1,1,1,1,0,0,1,
		PLL_28,
		TUNER_USER_SETUP
	},
	{
		"Chronos Video Shuttle II",
		3, 3, 0, 2, 0x1800,
		{ 2, 3, 1, 1, 0, 0, 0, 0},
		{ 0, 0, 0x1000, 0x1000, 0x0800, 0},
		0,
		1,1,1,1,0,0,0,1,
		PLL_28,
		TUNER_USER_SETUP
	},
	{
		"Typhoon TView TV/FM Tuner",
		3, 3, 0, 2, 0x1800,
		{ 2, 3, 1, 1, 0, 0, 0, 0},
		{ 0, 0x800, 0, 0, 0x1800, 0 },
		0,
		1,1,1,1,0,0,0,1,
		PLL_28,
		TUNER_USER_SETUP
	},
	{
		"PixelView PlayTV pro",
		3, 1, 0, 2, 0xff,
		{ 2, 3, 1, 1, 0, 0, 0, 0 },
		{ 0x21, 0x20, 0x24, 0x2c, 0x29, 0x29 },
		0,
		0,0,0,0,0,0,0,1,
		PLL_28,
		TUNER_USER_SETUP
	},
	{
		"TView99 CPH063",
		3, 1, 0, 2, 0x551e00,
		{ 2, 3, 1, 1, 0, 0, 0, 0},
		{ 0x551400, 0x551200, 0, 0, 0, 0x551200 },
		0,
		1,1,1,1,0,0,0,1,
		PLL_28,
		TUNER_USER_SETUP
	},
	{
		"Pinnacle PCTV Rave",
		3, 1, 0, 2, 0x03000F,
		{ 2, 3, 1, 1, 0, 0, 0, 0},
		{ 2, 0, 0, 0, 1, 0},
		0,
		1,1,1,1,0,0,0,1,
		PLL_28,
		TUNER_USER_SETUP
	},
	/* 0x28 */
	{
		"STB2",
		3, 1, 0, 2, 7,
		{ 2, 3, 1, 1, 0, 0, 0, 0},
		{ 4, 0, 2, 3, 1, 0},
		0,
		0,1,1,1,0,1,1,1,
		PLL_NONE,
		TUNER_USER_SETUP
	},
	{
		"AVerMedia TVPhone 98",
		3, 4, 0, 2, 4,
		{ 2, 3, 1, 1, 0, 0, 0, 0},
		{ 13, 14, 11, 7, 0, 0},
		0,
		1,1,1,1,0,0,0,1,
		PLL_28,
		TUNER_PHILIPS_PAL
	},
	{
		"ProVideo PV951", /* pic16c54 */
		3, 1, 0, 2, 0,
		{ 2, 3, 1, 1, 0, 0, 0, 0},
		{ 0, 0, 0, 0, 0, 0},
		0,
		0,0,0,0,0,0,0,0,
		PLL_28,
		TUNER_PHILIPS_PAL_I
	},
	{
		"Little OnAir TV",
		3, 1, 0, 2, 0xe00b,
		{2, 3, 1, 1, 0, 0, 0, 0},
		{0xff9ff6, 0xff9ff6, 0xff1ff7, 0, 0xff3ffc, 0},
		0,
		0,0,0,0,0,0,0,0,
		PLL_NONE,
		TUNER_USER_SETUP
	},
	{
		"Sigma TVII-FM",
		2, 1, 0, -1, 3,
		{2, 3, 1, 1, 0, 0, 0, 0},
		{1, 1, 0, 2, 3, 0},
		0,
		0,0,0,0,0,0,0,0,
		PLL_NONE,
		TUNER_USER_SETUP
	},
	{
		"MATRIX-Vision MV-Delta 2",
		5, 1, -1, 3, 0,
		{ 2, 3, 1, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0},
		0,
		0,0,0,0,0,0,0,0,
		PLL_28,
		TUNER_USER_SETUP
	},
	{
		"Zoltrix Genie TV",
		3, 1, 0, 2, 0xbcf03f,
		{ 2, 3, 1, 1, 0, 0, 0, 0},
		{ 0xbc803f, 0, 0xbcb03f, 0, 0xbcb03f, 0},
		0,
		0,0,0,0,0,0,0,0,
		PLL_28,
		TUNER_PHILIPS_PAL
	},
	{
		"Terratec TV/Radio+", /* Radio ?? */
		3, 1, 0, 2, 0x1f0000,
		{ 2, 3, 1, 1, 0, 0, 0, 0},
		{ 0xe2ffff, 0, 0, 0, 0xe0ffff, 0xe2ffff },
		0,
		0,0,0,0,0,0,0,0,
		PLL_35,
		TUNER_PHILIPS_PAL_I
	},
	/* 0x30 */
	{
		"Dynalink Magic TView",
		3, 1, 0, 2, 15,
		{ 2, 3, 1, 1, 0, 0, 0, 0},
		{2, 0, 0, 0, 1, 0},
		0,
		1,1,1,1,0,0,0,1,
		PLL_28,
		TUNER_USER_SETUP
	},
	// MAE 20 Nov 2000 Start of change
	{
		"Conexant Bt878 NTSC XEVK",
		3, 1, 0, 2, 0xFFFEFF,
		{ 2, 3, 1, 1, 0, 0, 0, 0},
		{ 0x001000, 0x001000, 0x000000, 0x000000, 0x003000, 0x000000},
		0,
		1,0,0,0,0,0,0,0,
		PLL_NONE,
		TUNER_PHILIPS_NTSC
	},
	{
		"Rockwell Bt878 NTSC XEVK",
		3, 1, 0, 2, 0xFFFEFF,
		{ 2, 3, 1, 1, 0, 0, 0, 0},
		{ 0x001000, 0x001000, 0x000000, 0x000000, 0x003000, 0x000000},
		0,
		1,0,0,0,0,0,0,0,
		PLL_NONE,
		TUNER_PHILIPS_NTSC
	},
	// MAE 20 Nov 2000 End of change
	// MAE 5 Dec 2000 Start of change
	{
		"Conexant Foghorn NTSC/ATSC-A",
		3, 1, 0, 2, 0xFF00F8,
		{ 2, 3, 1, 1, 0, 0, 0, 0},
		{ 0x000048, 0x000048, 0x000048, 0x000048, 0x000048, 0x000048},
		0,
		1,0,0,0,0,0,0,0,
		PLL_NONE,
		TUNER_PHILIPS_1236D_NTSC_INPUT1
	},
	{
		"Conexant Foghorn NTSC/ATSC-B",
		3, 1, 0, 2, 0xFF00F8,
		{ 2, 3, 1, 1, 0, 0, 0, 0},
		{ 0x000048, 0x000048, 0x000048, 0x000048, 0x000048, 0x000048},
		0,
		1,0,0,0,0,0,0,0,
		PLL_NONE,
		TUNER_PHILIPS_1236D_NTSC_INPUT1
	},
	{
		"Conexant Foghorn NTSC/ATSC-C",
		3, 1, 0, 2, 0xFF00F8,
		{ 2, 3, 1, 1, 0, 0, 0, 0},
		{ 0x000048, 0x000048, 0x000048, 0x000048, 0x000048, 0x000048},
		0,
		1,0,0,0,0,0,0,0,
		PLL_NONE,
		TUNER_PHILIPS_1236D_NTSC_INPUT1
	},
	// MAE 5 Dec 2000 End of change
	{
		"RS BT Card",
		3, 4, 0, 2, 0xfff,
		{ 2, 3, 1, 1, 0, 0, 0, 0},
		{ 13, 14, 11, 7, 0, 0},
		0,
		1,1,1,1,0,0,0,1,
		PLL_28,
		TUNER_ABSENT
	},
};

const AUTODETECT878 AutoDectect878[] =
{
	{ 0x00011002, TVCARD_HAUPPAUGE878,  "ATI TV Wonder" },
	{ 0x00011461, TVCARD_AVPHONE98,     "AVerMedia TVPhone98" },
	{ 0x00021461, TVCARD_AVERMEDIA98,   "Avermedia TVCapture 98" },
	{ 0x00031461, TVCARD_AVPHONE98,     "AVerMedia TVPhone98" },
	{ 0x00041461, TVCARD_AVPHONE98,     "AVerMedia TVPhone98" },
	{ 0x10b42636, TVCARD_HAUPPAUGE878,  "STB ???" },
	{ 0x1118153b, TVCARD_TERRATVALUE,   "Terratec TV Value" },
	{ 0x1123153b, TVCARD_TERRATVRADIO,  "Terratec TV/Radio+" },
	{ 0x1200bd11, TVCARD_PINNACLERAVE,  "Pinnacle PCTV Rave" },
	{ 0x13eb0070, TVCARD_HAUPPAUGE878,  "Hauppauge WinTV" },
	{ 0x18501851, TVCARD_CHRONOS_VS2,   "Chronos Video Shuttle II" },
	{ 0x18521852, TVCARD_TYPHOON_TVIEW, "Typhoon TView TV/FM Tuner" },
	{ 0x263610b4, TVCARD_STB2,          "STB TV PCI FM, P/N 6000704" },
	{ 0x3000144f, TVCARD_MAGICTVIEW063, "TView 99 (CPH063)" },
	{ 0x300014ff, TVCARD_MAGICTVIEW061, "TView 99 (CPH061)" },
	{ 0x3002144f, TVCARD_MAGICTVIEW061, "Askey Magic TView" },
	{ 0x300214ff, TVCARD_PHOEBE_TVMAS,  "Phoebe TV Master" },
	{ 0x400a15b0, TVCARD_ZOLTRIX_GENIE, "Zoltrix Genie TV" },
	{ 0x6606217d, TVCARD_WINFAST2000,   "Leadtek WinFast TV 2000" },
	// MAE 20 Nov 2000 Start of change
	{ 0x182214F1, TVCARD_CONEXANTNTSCXEVK,  "Conexant Bt878A NTSC XEVK" },
	{ 0x1322127A, TVCARD_ROCKWELLNTSCXEVK,  "Rockwell Bt878A NTSC XEVK" },
	// MAE 20 Nov 2000 End of change
	// MAE 5 Dec 2000 Start of change
	{ 0x013214F1, TVCARD_CONEXANTFOGHORNREVA,  "Conexant Foghorn NTSC/ATSC-A" },
	{ 0x023214F1, TVCARD_CONEXANTFOGHORNREVB,  "Conexant Foghorn NTSC/ATSC-B" },
	{ 0x033214F1, TVCARD_CONEXANTFOGHORNREVC,  "Conexant Foghorn NTSC/ATSC-C" },
	// MAE 5 Dec 2000 End of change

	{ 0, -1, NULL }
};

const TVTUNERSETUP Tuners[TUNER_LASTONE] =
{
	{ "NoTuner", NoTuner, NOTUNER, 	0, 0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 000,},
	{ "Philips PAL_I", Philips, PAL_I, 2244, 7412, 0xa0, 0x90, 0x30, 0x8e, 0xc0, 623, 0x00},
	{ "Philips NTSC", Philips, NTSC, 2516, 7220, 0xA0, 0x90, 0x30, 0x8e, 0xc0, 732, 0x00},
	{ "Philips SECAM", Philips, SECAM, 2592, 7156, 0xA7, 0x97, 0x37, 0x8e, 0xc0, 623, 0x02},
	{ "Philips PAL", Philips, PAL, 2592, 7156, 0xA0, 0x90, 0x30, 0x8e,  0xc0, 623, 0x00},
	{ "Temic PAL", TEMIC, PAL, 2244, 7412, 0x02, 0x04, 0x01, 0x8e, 0xc2, 623, 0x00},
	{ "Temic NTSC", TEMIC, NTSC, 2516, 7412, 0x02, 0x04, 0x01, 0x8e, 0xc2, 732, 0x00},
	{ "Temic PAL_I", TEMIC, PAL_I, 2720, 7200, 0x02, 0x04, 0x01, 0x8e, 0xc2, 623, 0x00},
	{ "Temic 4036 FY5 NTSC", TEMIC, NTSC, 2516, 7412, 0xa0, 0x90, 0x30, 0x8e, 0xc2, 732, 0x00},
	{ "Alps HSBH1", TEMIC, NTSC, 2196, 6164, 0x01, 0x02, 0x08, 0x8e, 0xc2, 732, 0x00},
	{ "Alps TSBE1",TEMIC,PAL, 2196, 6164, 0x01, 0x02, 0x08, 0x8e, 0xc2, 732},
	/* tested (UK UHF) with Modtec MM205 */
	{ "Alps TSBB5", Alps, PAL_I, 2132, 5620, 0x01, 0x02, 0x08, 0x8e, 0xc2, 632, 0x00},
	/* untested - data sheet guess. Only IF differs. */
	{ "Alps TSBE5", Alps, PAL, 2132, 5620, 0x01, 0x02, 0x08, 0x8e, 0xc2, 622, 0x00},
	/* untested - data sheet guess. Only IF differs. */
	{ "Alps TSBC5", Alps, PAL, 2132, 5620, 0x01, 0x02, 0x08, 0x8e, 0xc2, 608, 0x00},
	{ "Temic 4006FH5", TEMIC, PAL_I, 2720, 7200, 0xa0, 0x90, 0x30, 0x8e, 0xc2, 623, 0x00},
	// MAE 5 Dec 2000 Start of change
	{ "Philips 1236D ATSC/NTSC Input 1", Philips, NTSC, 2516, 7220, 0xA3, 0x93, 0x33, 0xCE, 0xc2, 732, 0x00},
	{ "Philips 1236D ATSC/NTSC Input 2", Philips, NTSC, 2516, 7220, 0xA2, 0x92, 0x32, 0xCE, 0xc2, 732, 0x00},
	// MAE 5 Dec 2000 End of change

};

// do any specific card related initilaisation
void Card_Init()
{
	switch(CardType)
	{
	case TVCARD_HAUPPAUGE:
	case TVCARD_HAUPPAUGE878:
		//hauppauge_readee(btv,eeprom_data,0xa0);
        //hauppauge_eeprom(btv);
        hauppauge_boot_msp34xx();
		break;
	case TVCARD_PXC200:
		init_PXC200();
		break;
	default:
		break;
	}
}

/* reset/enable the MSP on some Hauppauge cards */
/* Thanks to Ky�sti M�lkki (kmalkki@cc.hut.fi)! */
void hauppauge_boot_msp34xx()
{
	/* reset/enable the MSP on some Hauppauge cards */
	/* Thanks to Ky�sti M�lkki (kmalkki@cc.hut.fi)! */
	BT848_AndOrDataDword(BT848_GPIO_OUT_EN, 32, ~32);
	BT848_AndOrDataDword(BT848_GPIO_DATA, 0, ~32);
	Sleep(10);
	BT848_AndOrDataDword(BT848_GPIO_DATA, 32, ~32);
}


/* ----------------------------------------------------------------------- */
/*  Imagenation L-Model PXC200 Framegrabber */
/*  This is basically the same procedure as
 *  used by Alessandro Rubini in his pxc200
 *  driver, but using BTTV functions */

void init_PXC200()
{
	const BYTE vals[] =
	{
		0x08, 0x09, 0x0a, 0x0b, 0x0d, 0x0d,
		0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
		0x00
	};
	int i;

	/* Initialise GPIO-connevted stuff */
	BT848_WriteWord(BT848_GPIO_OUT_EN, 1<<13); /* Reset pin only */
	BT848_WriteWord(BT848_GPIO_DATA, 0);
	Sleep(30);
	BT848_WriteWord(BT848_GPIO_DATA, 1<<13);
	/* GPIO inputs are pulled up, so no need to drive
	 * reset pin any longer */
	BT848_WriteWord(BT848_GPIO_OUT_EN, 0);

	/*  we could/should try and reset/control the AD pots? but
	    right now  we simply  turned off the crushing.  Without
	    this the AGC drifts drifts
	    remember the EN is reverse logic -->
	    setting BT848_ADC_AGC_EN disable the AGC
	    tboult@eecs.lehigh.edu
	*/
	BT848_WriteByte(BT848_ADC, BT848_ADC_RESERVED|BT848_ADC_AGC_EN);

	/*	Initialise MAX517 DAC */
	I2CBus_Lock();
	I2CBus_Write(0x5E, 0, 0x80, 1);

	/*	Initialise 12C508 PIC */
	/*	The I2CWrite and I2CRead commmands are actually to the
	 *	same chips - but the R/W bit is included in the address
	 *	argument so the numbers are different */
	for (i = 0; i < sizeof(vals)/sizeof(int); i++)
	{
		I2CBus_Write(0x1E, vals[i], 0, 1);
		I2CBus_Read(0x1F);
	}
	I2CBus_Unlock();
}

int Card_AutoDetectTuner(TVCARDID CardId)
{
	TVTUNERID Tuner = TUNER_ABSENT;
	switch(CardId)
	{
	case TVCARD_MIRO:
	case TVCARD_MIROPRO:
		Tuner = ((BT848_ReadWord(BT848_GPIO_DATA)>>10)-1)&7;
		break;
	default:
		break;
	}
	return Tuner;
}

TVCARDID Card_AutoDetect()
{
	// look for normal eeprom address
	if(I2CBus_AddDevice(I2C_HAUPEE))
	{
		DWORD Id = BT848_GetSubSystemID();
		if (Id != 0 && Id != 0xffffffff)
		{
			int i;
			for (i = 0; AutoDectect878[i].ID != 0; i++)
			{
				if (AutoDectect878[i].ID  == Id)
				{
					return AutoDectect878[i].CardId;
				}
			}
		}
	}

	// look for STB eeprom address
	if(I2CBus_AddDevice(I2C_STBEE))
	{
		return TVCARD_STB;
	}

	return TVCARD_UNKNOWN;
}

const TVCARDSETUP* GetCardSetup()
{
	return TVCards + CardType;
}

const TVTUNERSETUP* GetTunerSetup()
{
	if(TunerType >= 0)
	{
		return Tuners + TunerType;
	}
	else
	{
		return NULL;
	}
}

void TVCard_FirstTimeSetupHardware(HINSTANCE hInst, HWND hWnd)
{
	// try to detect the card
	CardType = Card_AutoDetect();
	Card_AutoDetectTuner(CardType);

	// then display the hardware setup dialog
	DialogBox(hInst, "SELECTCARD", hWnd, (DLGPROC) SelectCardProc);
}

void TVCard_ChangeDefault(SETTING* pSetting, long Default)
{
	pSetting->Default = Default;
	*pSetting->pValue = Default;
}

void ChangeTVSettingsBasedOnTuner()
{
	// default the TVTYPE dependant on the Tuner selected
	// should be OK most of the time
	if(TunerType != TUNER_ABSENT)
	{
		switch(Tuners[TunerType].Type)
		{
		case PAL:
		case PAL_I:
			TVCard_ChangeDefault(BT848_GetSetting(TVFORMAT), FORMAT_PAL_BDGHI);
			break;
		case SECAM:
			TVCard_ChangeDefault(BT848_GetSetting(TVFORMAT), FORMAT_SECAM);
			break;
		case NTSC:
		default:
			TVCard_ChangeDefault(BT848_GetSetting(TVFORMAT), FORMAT_NTSC);
			break;
		}
	}
}

	
void ChangeDefaultsBasedOnHardware()
{
	// default the TVTYPE dependant on the Tuner selected
	// should be OK most of the time
	if(TunerType != TUNER_ABSENT)
	{
		switch(Tuners[TunerType].Type)
		{
		case PAL:
		case PAL_I:
			TVCard_ChangeDefault(BT848_GetSetting(TVFORMAT), FORMAT_PAL_BDGHI);
			break;
		case SECAM:
			TVCard_ChangeDefault(BT848_GetSetting(TVFORMAT), FORMAT_SECAM);
			break;
		case NTSC:
		default:
			TVCard_ChangeDefault(BT848_GetSetting(TVFORMAT), FORMAT_NTSC);
			break;
		}
	}
	// now do defaults based on the processor speed selected
	if(ProcessorSpeed == 1 && TradeOff == 0)
	{
		// User has selected 300-500 MHz and low judder
		TVCard_ChangeDefault(OutThreads_GetSetting(HURRYWHENLATE), FALSE);
		TVCard_ChangeDefault(OutThreads_GetSetting(WAITFORFLIP), TRUE);
		TVCard_ChangeDefault(OutThreads_GetSetting(DOACCURATEFLIPS), FALSE);
		TVCard_ChangeDefault(OutThreads_GetSetting(AUTODETECT), TRUE);
		TVCard_ChangeDefault(OutThreads_GetSetting(PULLDOWNMODE), GREEDY);
		TVCard_ChangeDefault(FD60_GetSetting(NTSCFILMFALLBACKMODE), GREEDY);
		TVCard_ChangeDefault(FD50_GetSetting(PALFILMFALLBACKMODE), GREEDY);
		TVCard_ChangeDefault(BT848_GetSetting(CURRENTX), 720);
	}
	else if(ProcessorSpeed == 1 && TradeOff == 1)
	{
		// User has selected 300-500 MHz and best picture
		TVCard_ChangeDefault(OutThreads_GetSetting(HURRYWHENLATE), TRUE);
		TVCard_ChangeDefault(OutThreads_GetSetting(WAITFORFLIP), FALSE);
		TVCard_ChangeDefault(OutThreads_GetSetting(DOACCURATEFLIPS), FALSE);
		TVCard_ChangeDefault(OutThreads_GetSetting(AUTODETECT), TRUE);
		TVCard_ChangeDefault(OutThreads_GetSetting(PULLDOWNMODE), GREEDY);
		TVCard_ChangeDefault(FD60_GetSetting(NTSCFILMFALLBACKMODE), GREEDY);
		TVCard_ChangeDefault(FD50_GetSetting(PALFILMFALLBACKMODE), GREEDY);
		TVCard_ChangeDefault(BT848_GetSetting(CURRENTX), 720);
	}
	else if(ProcessorSpeed == 2 && TradeOff == 0)
	{
		// User has selected below 300 MHz and low judder
		TVCard_ChangeDefault(OutThreads_GetSetting(HURRYWHENLATE), FALSE);
		TVCard_ChangeDefault(OutThreads_GetSetting(WAITFORFLIP), TRUE);
		TVCard_ChangeDefault(OutThreads_GetSetting(DOACCURATEFLIPS), FALSE);
		TVCard_ChangeDefault(OutThreads_GetSetting(AUTODETECT), FALSE);
		TVCard_ChangeDefault(OutThreads_GetSetting(PULLDOWNMODE), GREEDY);
		TVCard_ChangeDefault(FD60_GetSetting(NTSCFILMFALLBACKMODE), GREEDY);
		TVCard_ChangeDefault(FD50_GetSetting(PALFILMFALLBACKMODE), GREEDY);
		TVCard_ChangeDefault(BT848_GetSetting(CURRENTX), 640);
	}
	else if(ProcessorSpeed == 2 && TradeOff == 1)
	{
		// User has selected below 300 MHz and best picture
		TVCard_ChangeDefault(OutThreads_GetSetting(HURRYWHENLATE), TRUE);
		TVCard_ChangeDefault(OutThreads_GetSetting(WAITFORFLIP), FALSE);
		TVCard_ChangeDefault(OutThreads_GetSetting(DOACCURATEFLIPS), FALSE);
		TVCard_ChangeDefault(OutThreads_GetSetting(AUTODETECT), FALSE);
		TVCard_ChangeDefault(OutThreads_GetSetting(PULLDOWNMODE), GREEDY);
		TVCard_ChangeDefault(FD60_GetSetting(NTSCFILMFALLBACKMODE), GREEDY);
		TVCard_ChangeDefault(FD50_GetSetting(PALFILMFALLBACKMODE), GREEDY);
		TVCard_ChangeDefault(BT848_GetSetting(CURRENTX), 640);
	}
	else
	{
		// user has fast processor use best defaults
		TVCard_ChangeDefault(OutThreads_GetSetting(HURRYWHENLATE), FALSE);
		TVCard_ChangeDefault(OutThreads_GetSetting(WAITFORFLIP), TRUE);
		TVCard_ChangeDefault(OutThreads_GetSetting(DOACCURATEFLIPS), FALSE);
		TVCard_ChangeDefault(OutThreads_GetSetting(AUTODETECT), TRUE);
		TVCard_ChangeDefault(OutThreads_GetSetting(PULLDOWNMODE), ADAPTIVE);
		TVCard_ChangeDefault(FD60_GetSetting(NTSCFILMFALLBACKMODE), ADAPTIVE);
		TVCard_ChangeDefault(FD50_GetSetting(PALFILMFALLBACKMODE), GREEDY);
		TVCard_ChangeDefault(BT848_GetSetting(CURRENTX), 720);
	}
}

BOOL APIENTRY SelectCardProc(HWND hDlg, UINT message, UINT wParam, LONG lParam)
{
	int i;
	static long OrigProcessorSpeed;
	static long OrigTradeOff;
	static long OrigTuner;

	switch (message)
	{
	case WM_INITDIALOG:
		SendMessage(GetDlgItem(hDlg, IDC_CARDSSELECT), CB_RESETCONTENT, 0, 0);
		for(i = 0; i < TVCARD_LASTONE; i++)
		{
			SendMessage(GetDlgItem(hDlg, IDC_CARDSSELECT), CB_ADDSTRING, 0, (LONG)TVCards[i].szName);
		}
		SendMessage(GetDlgItem(hDlg, IDC_CARDSSELECT), CB_SETCURSEL, CardType, 0);

		SendMessage(GetDlgItem(hDlg, IDC_TUNERSELECT), CB_RESETCONTENT, 0, 0);
		for(i = 0; i < TUNER_LASTONE; i++)
		{
			SendMessage(GetDlgItem(hDlg, IDC_TUNERSELECT), CB_ADDSTRING, 0, (LONG)Tuners[i].szName);
		}
		SendMessage(GetDlgItem(hDlg, IDC_TUNERSELECT), CB_SETCURSEL, TunerType, 0);

		SendMessage(GetDlgItem(hDlg, IDC_PROCESSOR_SPEED), CB_ADDSTRING, 0, (LONG)"Above 500 MHz");
		SendMessage(GetDlgItem(hDlg, IDC_PROCESSOR_SPEED), CB_ADDSTRING, 0, (LONG)"300 - 500 MHz");
		SendMessage(GetDlgItem(hDlg, IDC_PROCESSOR_SPEED), CB_ADDSTRING, 0, (LONG)"Below 300 MHz");
		SendMessage(GetDlgItem(hDlg, IDC_PROCESSOR_SPEED), CB_SETCURSEL, ProcessorSpeed, 0);
		SendMessage(GetDlgItem(hDlg, IDC_TRADEOFF), CB_ADDSTRING, 0, (LONG)"Show all frames - Lowest judder");
		SendMessage(GetDlgItem(hDlg, IDC_TRADEOFF), CB_ADDSTRING, 0, (LONG)"Best picture quality");
		SendMessage(GetDlgItem(hDlg, IDC_TRADEOFF), CB_SETCURSEL, TradeOff, 0);
		OrigProcessorSpeed = ProcessorSpeed;
		OrigTradeOff = TradeOff;
		OrigTuner = TunerType;
		SetFocus(hDlg);
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			TunerType = SendMessage(GetDlgItem(hDlg, IDC_TUNERSELECT), CB_GETCURSEL, 0, 0);
			CardType = SendMessage(GetDlgItem(hDlg, IDC_CARDSSELECT), CB_GETCURSEL, 0, 0);
			ProcessorSpeed = SendMessage(GetDlgItem(hDlg, IDC_PROCESSOR_SPEED), CB_GETCURSEL, 0, 0);
			TradeOff = SendMessage(GetDlgItem(hDlg, IDC_TRADEOFF), CB_GETCURSEL, 0, 0);
			if(OrigProcessorSpeed != ProcessorSpeed || 
				OrigTradeOff != TradeOff)
			{
				ChangeDefaultsBasedOnHardware();
			}
			if(OrigTuner != TunerType)
			{
				ChangeTVSettingsBasedOnTuner();
			}
			EndDialog(hDlg, TRUE);
			break;
		case IDCANCEL:
			EndDialog(hDlg, TRUE);
			break;
		case IDC_CARDSSELECT:
			i = SendMessage(GetDlgItem(hDlg, IDC_CARDSSELECT), CB_GETCURSEL, 0, 0);
			if(TVCards[i].TunerId == TUNER_USER_SETUP)
			{
				SendMessage(GetDlgItem(hDlg, IDC_TUNERSELECT), CB_SETCURSEL, TUNER_ABSENT, 0);
			}
			else if(TVCards[i].TunerId == TUNER_AUTODETECT)
			{
				SendMessage(GetDlgItem(hDlg, IDC_TUNERSELECT), CB_SETCURSEL, Card_AutoDetectTuner(i), 0);
			}
			else
			{
				SendMessage(GetDlgItem(hDlg, IDC_TUNERSELECT), CB_SETCURSEL, TVCards[i].TunerId, 0);
			}
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
	return (FALSE);
}

/////////////////////////////////////////////////////////////////////////////
// Start of Settings related code
/////////////////////////////////////////////////////////////////////////////

SETTING TVCardSettings[TVCARD_SETTING_LASTONE] =
{
	{
		"Card Type", NUMBER, 0, &CardType,
		 TVCARD_UNKNOWN, TVCARD_UNKNOWN, TVCARD_LASTONE - 1, 0, NULL,
		"Hardware", "CardType", NULL,
	},
	{
		"Tuner Type", NUMBER, 0, &TunerType,
		 TUNER_ABSENT, TUNER_ABSENT, TUNER_LASTONE - 1, 0, NULL,
		"Hardware", "TunerType", NULL,
	},
	{
		"Processor Speed", NUMBER, 0, &ProcessorSpeed,
		 1, 0, 2, 0, NULL,
		"Hardware", "ProcessorSpeed", NULL,
	},
	{
		"Trade Off", NUMBER, 0, &TradeOff,
		 1, 0, 1, 0, NULL,
		"Hardware", "TradeOff", NULL,
	},
};


SETTING* TVCard_GetSetting(TVCARD_SETTING Setting)
{
	if(Setting > -1 && Setting < TVCARD_SETTING_LASTONE)
	{
		return &(TVCardSettings[Setting]);
	}
	else
	{
		return NULL;
	}
}

void TVCard_ReadSettingsFromIni()
{
	int i;
	for(i = 0; i < TVCARD_SETTING_LASTONE; i++)
	{
		Setting_ReadFromIni(&(TVCardSettings[i]));
	}
	ChangeDefaultsBasedOnHardware();
}

void TVCard_WriteSettingsToIni()
{
	int i;
	for(i = 0; i < TVCARD_SETTING_LASTONE; i++)
	{
		Setting_WriteToIni(&(TVCardSettings[i]));
	}
}

void TVCard_SetMenu(HMENU hMenu)
{
	EnableMenuItem(hMenu, IDM_CHANNELPLUS, (TunerType != TUNER_ABSENT)?MF_ENABLED:MF_GRAYED);
	EnableMenuItem(hMenu, IDM_CHANNELMINUS, (TunerType != TUNER_ABSENT)?MF_ENABLED:MF_GRAYED);
	EnableMenuItem(hMenu, IDM_ANALOGSCAN, (TunerType != TUNER_ABSENT)?MF_ENABLED:MF_GRAYED);
	EnableMenuItem(hMenu, IDM_SOURCE_TUNER, (TunerType != TUNER_ABSENT)?MF_ENABLED:MF_GRAYED);

	if(TVCards[CardType].SVideoInput == -1)
	{
		EnableMenuItem(hMenu, IDM_SOURCE_SVIDEO, MF_GRAYED);
		EnableMenuItem(hMenu, IDM_SOURCE_OTHER1, (TVCards[CardType].nVideoInputs > 2)?MF_ENABLED:MF_GRAYED);
		EnableMenuItem(hMenu, IDM_SOURCE_OTHER2, (TVCards[CardType].nVideoInputs > 3)?MF_ENABLED:MF_GRAYED);
		EnableMenuItem(hMenu, IDM_SOURCE_COMPVIASVIDEO, MF_GRAYED);
	}
	else
	{
		EnableMenuItem(hMenu, IDM_SOURCE_SVIDEO, MF_ENABLED);
		EnableMenuItem(hMenu, IDM_SOURCE_OTHER1, (TVCards[CardType].nVideoInputs > 3)?MF_ENABLED:MF_GRAYED);
		EnableMenuItem(hMenu, IDM_SOURCE_OTHER2, (TVCards[CardType].nVideoInputs > 4)?MF_ENABLED:MF_GRAYED);
		EnableMenuItem(hMenu, IDM_SOURCE_COMPVIASVIDEO, MF_ENABLED);
	}
	EnableMenuItem(hMenu, IDM_SOURCE_CCIR656, (CardType == TVCARD_RS_BT)?MF_ENABLED:MF_GRAYED);
}
