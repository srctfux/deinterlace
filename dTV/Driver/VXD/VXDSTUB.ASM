;///////////////////////////////////////////////////////////////////////////
; Makefile
;///////////////////////////////////////////////////////////////////////////
; Copyright (c) 2000 John Adcock.  All rights reserved.
;///////////////////////////////////////////////////////////////////////////
;
;	This file is subject to the terms of the GNU General Public License as
;	published by the Free Software Foundation.  A copy of this license is
;	included with this software distribution in the file COPYING.  If you
;	do not have a copy, you may obtain a copy by writing to the Free
;	Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
;
;	This software is distributed in the hope that it will be useful,
;	but WITHOUT ANY WARRANTY; without even the implied warranty of
;	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;	GNU General Public License for more details
;///////////////////////////////////////////////////////////////////////////
;
; This software was based on hwiodrv from the FreeTV project Those portions are
; Copyright (C) Mathias Ellinger
;
;///////////////////////////////////////////////////////////////////////////
; Change Log
;
; Date          Developer             Changes
;
; 15 Oct 1998   Mathias Ellinger      initial version
;
; 24 Jul 2000   John Adcock           Changed variable names to dTV
;
;///////////////////////////////////////////////////////////////////////////

PAGE 58,132

dTVdrv95_DYNAMIC   EQU 1            ; this equate makes the VXD dynamically
dTVdrv95_DEVICE_ID EQU 0666H        ; hehe
DDK_VERSION      EQU 400h
Create_dTVdrv95_Service_Table EQU 1

 .386p
 .xlist
 include vmm.inc
 .list

BEGIN_SERVICE_TABLE dTVdrv95
dTVdrv95_Service     dTVdrvGetVersion, local
END_SERVICE_TABLE   dTVdrv95

;============================================================================
;        V I R T U A L   D E V I C E   D E C L A R A T I O N
;============================================================================

DECLARE_VIRTUAL_DEVICE dTVdrv95, 1, 0, dTVdrv95_Control, dTVdrv95_DEVICE_ID,\
                               UNDEFINED_INIT_ORDER,0,0,0



VxD_LOCKED_CODE_SEG

;===========================================================================
;
;   PROCEDURE: hwioControl
;
;   DESCRIPTION:
;    Device control procedure for the hwio95 VxD
;
;   ENTRY:
;    EAX = Control call ID
;
;   EXIT:
;    If carry clear then
;        Successful
;    else
;        Control call failed
;
;   USES:
;    EAX, EBX, ECX, EDX, ESI, EDI, Flags
;
;============================================================================

BEGINPROC dTVdrv95_Control

  Control_Dispatch   SYS_DYNAMIC_DEVICE_INIT, dTVdrvDynamicInit, sCall
  Control_Dispatch   SYS_DYNAMIC_DEVICE_EXIT, dTVdrvDynamicExit, sCall
  Control_Dispatch   INIT_COMPLETE,           dTVdrvInitComplete, sCall
  Control_Dispatch   W32_DEVICEIOCONTROL,     dTVdrvW32DeviceIOControl, sCall, <ecx, ebx, edx, esi>

  clc
  ret
ENDPROC dTVdrv95_Control


BEGINPROC dTVdrvGetVersion
  mov eax,100
  clc
  ret
ENDPROC dTVdrvGetVersion

VxD_LOCKED_CODE_ENDS

END
