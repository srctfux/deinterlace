/*
 *    Copyright (C) 2003 Gabest
 *    http://www.gabest.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 *  Note: This interface was defined for the matroska container format
 *  originally, but can be implemented for other formats as well.
 *
 */

#pragma once

// 30323449-0000-0010-8000-00AA00389B71  'I420' == MEDIASUBTYPE_I420
DEFINE_GUID(MEDIASUBTYPE_I420,
0x30323449, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);

#define WAVE_FORMAT_DOLBY_AC3 0x2000
// {00002000-0000-0010-8000-00aa00389b71}
DEFINE_GUID(MEDIASUBTYPE_WAVE_DOLBY_AC3,
WAVE_FORMAT_DOLBY_AC3, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);

#define WAVE_FORMAT_DVD_DTS 0x2001
// {00002001-0000-0010-8000-00aa00389b71}
DEFINE_GUID(MEDIASUBTYPE_WAVE_DTS,
WAVE_FORMAT_DVD_DTS, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);

// Be compatible with 3ivx
#define WAVE_FORMAT_AAC 0x00FF
// {000000FF-0000-0010-8000-00AA00389B71}
DEFINE_GUID(MEDIASUBTYPE_AAC,
WAVE_FORMAT_AAC, 0x000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);

// ... and also compatible with nero
// btw, older nero parsers use a lower-case fourcc, newer upper-case (why can't it just offer both?)
// {4134504D-0000-0010-8000-00AA00389B71}
DEFINE_GUID(MEDIASUBTYPE_MP4A,
0x4134504D, 0x000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);

// {6134706D-0000-0010-8000-00AA00389B71}
DEFINE_GUID(MEDIASUBTYPE_mp4a,
0x6134706D, 0x000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);

#define WAVE_FORMAT_MP3 0x0055
// 00000055-0000-0010-8000-00AA00389B71
DEFINE_GUID(MEDIASUBTYPE_MP3,
WAVE_FORMAT_MP3, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);

#define WAVE_FORMAT_FLAC 0xF1AC
// 0000F1AC-0000-0010-8000-00AA00389B71
DEFINE_GUID(MEDIASUBTYPE_FLAC,
WAVE_FORMAT_FLAC, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);

// {1541C5C0-CDDF-477d-BC0A-86F8AE7F8354}
DEFINE_GUID(MEDIASUBTYPE_FLAC_FRAMED,
0x1541c5c0, 0xcddf, 0x477d, 0xbc, 0xa, 0x86, 0xf8, 0xae, 0x7f, 0x83, 0x54);

#define WAVE_FORMAT_MPEG2 0x0050
// 00000050-db46-11cf-b4d1-00805f6cbbea
DEFINE_GUID(MEDIASUBTYPE_MPEG2_AUDIO_MPCBUG,
WAVE_FORMAT_MPEG2, 0xdb46, 0x11cf, 0xb4, 0xd1, 0x00, 0x80, 0x5f, 0x6c, 0xbb, 0xea);

//
// RealMedia
//


// {57428EC6-C2B2-44a2-AA9C-28F0B6A5C48E}
DEFINE_GUID(MEDIASUBTYPE_RealMedia,
0x57428ec6, 0xc2b2, 0x44a2, 0xaa, 0x9c, 0x28, 0xf0, 0xb6, 0xa5, 0xc4, 0x8e);

// 30315652-0000-0010-8000-00AA00389B71
DEFINE_GUID(MEDIASUBTYPE_RV10,
0x30315652, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);

// 30325652-0000-0010-8000-00AA00389B71
DEFINE_GUID(MEDIASUBTYPE_RV20,
0x30325652, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);

// 30335652-0000-0010-8000-00AA00389B71
DEFINE_GUID(MEDIASUBTYPE_RV30,
0x30335652, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);

// 30345652-0000-0010-8000-00AA00389B71
DEFINE_GUID(MEDIASUBTYPE_RV40,
0x30345652, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);

// 345f3431-0000-0010-8000-00AA00389B71
DEFINE_GUID(MEDIASUBTYPE_14_4,
0x345f3431, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);

// 385f3832-0000-0010-8000-00AA00389B71
DEFINE_GUID(MEDIASUBTYPE_28_8,
0x385f3832, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);

// 43525441-0000-0010-8000-00AA00389B71
DEFINE_GUID(MEDIASUBTYPE_ATRC,
0x43525441, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);

// 4b4f4f43-0000-0010-8000-00AA00389B71
DEFINE_GUID(MEDIASUBTYPE_COOK,
0x4b4f4f43, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);

// 54454e44-0000-0010-8000-00AA00389B71
DEFINE_GUID(MEDIASUBTYPE_DNET,
0x54454e44, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);

// 52504953-0000-0010-8000-00AA00389B71
DEFINE_GUID(MEDIASUBTYPE_SIPR,
0x52504953, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);

enum
{
    WAVE_FORMAT_14_4 = 0x2002,
    WAVE_FORMAT_28_8 = 0x2003,
    WAVE_FORMAT_ATRC = 0x0270, //WAVE_FORMAT_SONY_SCX,
    WAVE_FORMAT_COOK = 0x2004,
    WAVE_FORMAT_DNET = 0x2005,
    WAVE_FORMAT_SIPR = 0x0130, //WAVE_FORMAT_SIPROLAB_ACEPLNET,
};

DEFINE_GUID(CLSID_FFDShow, 0x04FE9017, 0xf873, 0x410e, 0x87, 0x1e, 0xab, 0x91, 0x66, 0x1a, 0x4e, 0xf7);
DEFINE_GUID(CLSID_FFDShowRaw, 0x0B390488, 0xd80f, 0x4a68, 0x84, 0x08, 0x48, 0xdc, 0x19, 0x9f, 0x0e, 0x97);
DEFINE_GUID(CLSID_DirectVobSubFilter, 0x93a22e7a, 0x5091, 0x45ef, 0xba, 0x61, 0x6d, 0xa2, 0x61, 0x56, 0xa5, 0xd0);
DEFINE_GUID(CLSID_DirectVobSubFilter2, 0x9852a670, 0xf845, 0x491b, 0x9b, 0xe6, 0xeb, 0xd8, 0x41, 0xb8, 0xa6, 0x13);
DEFINE_GUID(CLSID_WM10RENDERER,  0xaa20215c, 0xb047, 0x4702, 0xba, 0x13, 0x57, 0x37, 0x61, 0x13, 0xaa, 0xd0);
DEFINE_GUID(CLSID_CDScaler, 0x0D71870A, 0x7563, 0x11D7, 0xB8, 0x4A, 0x00, 0x02, 0xA5, 0x62, 0x33, 0x77);

DEFINE_GUID(MEDIASUBTYPE_xvid, 0x64697678, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
DEFINE_GUID(MEDIASUBTYPE_XVID, 0x44495658, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
DEFINE_GUID(MEDIASUBTYPE_divx, 0x78766964, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
DEFINE_GUID(MEDIASUBTYPE_DIVX, 0x58564944, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
DEFINE_GUID(MEDIASUBTYPE_div3, 0x33766964, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
DEFINE_GUID(MEDIASUBTYPE_DIV3, 0x33564944, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
DEFINE_GUID(MEDIASUBTYPE_dx50, 0x30357864, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
DEFINE_GUID(MEDIASUBTYPE_DX50, 0x30355844, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
DEFINE_GUID(MEDIASUBTYPE_mp43, 0x3334706D, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
DEFINE_GUID(MEDIASUBTYPE_MP43, 0x3334504D, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
DEFINE_GUID(MEDIASUBTYPE_mp42, 0x3234706D, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
DEFINE_GUID(MEDIASUBTYPE_MP42, 0x3234504D, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
DEFINE_GUID(MEDIASUBTYPE_mp41, 0x3134706D, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
DEFINE_GUID(MEDIASUBTYPE_MP41, 0x3134504D, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
DEFINE_GUID(MEDIASUBTYPE_h263, 0x33363268, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
DEFINE_GUID(MEDIASUBTYPE_avc1, 0x31637661, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
DEFINE_GUID(MEDIASUBTYPE_AVC1, 0x31435641, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
DEFINE_GUID(MEDIASUBTYPE_MP4V, MAKEFOURCC('M', 'P', '4', 'V'), 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
DEFINE_GUID(MEDIASUBTYPE_mp4v, MAKEFOURCC('m', 'p', '4', 'v'), 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);

// made up type representing planar 4:2:2 YUV video
DEFINE_GUID(MEDIASUBTYPE_P422, MAKEFOURCC('P', '4', '2', '2'), 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);

// made up type representing planar 4:4:4 YUV video
DEFINE_GUID(MEDIASUBTYPE_P444, MAKEFOURCC('P', '4', '4', '4'), 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
