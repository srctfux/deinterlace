//
// $Id: TunerID.h,v 1.8 2005-03-09 07:53:41 atnak Exp $
//
/////////////////////////////////////////////////////////////////////////////
//
// copyleft 2003 itt@myself.com
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
//
/////////////////////////////////////////////////////////////////////////////
//
// $Log: not supported by cvs2svn $
// Revision 1.7  2005/03/06 12:43:13  to_see
// Added some tuner
//
// Revision 1.6  2004/12/29 20:17:19  to_see
// Added TCL 2002N tuner (from v4l2)
//
// Revision 1.5  2004/08/27 13:11:27  to_see
// Added Philips 4in1 tuner (from v4l2)
//
// Revision 1.4  2003/12/18 15:57:41  adcockj
// Added MT2050 tuner type support (untested)
//
// Revision 1.3  2003/10/27 10:39:54  adcockj
// Updated files for better doxygen compatability
//
// Revision 1.2  2003/02/06 21:27:05  ittarnavsky
// moved the tuner names here
//
// Revision 1.1  2003/02/06 19:40:00  ittarnavsky
// contains definition of eTunerId
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file tunerid.h tunerid Header
 */

#if !defined(__TUNERID_H__)
#define __TUNERID_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

enum eTunerId
{
    TUNER_AUTODETECT = -2,
    TUNER_USER_SETUP = -1,
    TUNER_ABSENT = 0,    
    TUNER_PHILIPS_PAL_I,
    TUNER_PHILIPS_NTSC,                     
    TUNER_PHILIPS_SECAM,        
    TUNER_PHILIPS_PAL,      
    TUNER_TEMIC_4002FH5_PAL,
    TUNER_TEMIC_4032FY5_NTSC,       
    TUNER_TEMIC_4062FY5_PAL_I,      
    TUNER_TEMIC_4036FY5_NTSC,
    TUNER_ALPS_TSBH1_NTSC,  
    TUNER_ALPS_TSBE1_PAL,   
    TUNER_ALPS_TSBB5_PAL_I,     
    TUNER_ALPS_TSBE5_PAL,   
    TUNER_ALPS_TSBC5_PAL,   
    TUNER_TEMIC_4006FH5_PAL,
    TUNER_PHILIPS_1236D_NTSC_INPUT1,
    TUNER_PHILIPS_1236D_NTSC_INPUT2,
    TUNER_ALPS_TSCH6_NTSC,
    TUNER_TEMIC_4016FY5_PAL,
    TUNER_PHILIPS_MK2_NTSC,
    TUNER_TEMIC_4066FY5_PAL_I,
    TUNER_TEMIC_4006FN5_PAL,
    TUNER_TEMIC_4009FR5_PAL,
    TUNER_TEMIC_4039FR5_NTSC,
    TUNER_TEMIC_4046FM5_MULTI,
    TUNER_PHILIPS_PAL_DK,
    TUNER_PHILIPS_MULTI,
    TUNER_LG_I001D_PAL_I,
    TUNER_LG_I701D_PAL_I,
    TUNER_LG_R01F_NTSC,
    TUNER_LG_B01D_PAL,
    TUNER_LG_B11D_PAL,
    TUNER_TEMIC_4009FN5_PAL,
    TUNER_MT2032,
    TUNER_SHARP_2U5JF5540_NTSC,
    TUNER_LG_TAPCH701P_NTSC,
    TUNER_SAMSUNG_PAL_TCPM9091PD27,
    TUNER_TEMIC_4106FH5,
    TUNER_TEMIC_4012FY5,
    TUNER_TEMIC_4136FY5,
    TUNER_LG_TAPCNEW_PAL,
    TUNER_PHILIPS_FM1216ME_MK3,
    TUNER_LG_TAPCNEW_NTSC,
    TUNER_MT2032_PAL,
    TUNER_PHILIPS_FI1286_NTSC_M_J,
    TUNER_MT2050,
    TUNER_MT2050_PAL,
	TUNER_PHILIPS_4IN1,
    TUNER_TCL_2002N,
    TUNER_HITACHI_NTSC,
    TUNER_PHILIPS_PAL_MK,
    TUNER_PHILIPS_FM1236_MK3,
    TUNER_LG_NTSC_TAPE,
    TUNER_TNF_8831BGFF,
    TUNER_PHILIPS_FM1256_IH3,
    TUNER_PHILIPS_FQ1286,
    TUNER_LG_PAL_TAPE,
    TUNER_PHILIPS_FM1216ME,
    TUNER_PHILIPS_FQ1236A_MK4,
    TUNER_TDA8275,
    TUNER_LASTONE,
};

extern const char *TunerNames[TUNER_LASTONE];

#endif // !defined(__TUNERID_H__)
