/////////////////////////////////////////////////////////////////////////////
// dTVdrv95.cpp
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
// This software was based on hwiodrv from the FreeTV project Those portions are
// Copyright (C) Mathias Ellinger
//
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 19 Nov 1998   Mathias Ellinger      initial version
//
// 24 Jul 2000   John Adcock           Original dTV Release
//                                     Added Memory Alloc functions
//
/////////////////////////////////////////////////////////////////////////////

#include <ioclass.h>
#include <debugout.h>

extern "C" {

DWORD _stdcall dTVdrvW32DeviceIOControl(
                                      DWORD           dwService,
                                      DWORD           dwDDB,
                                      DWORD           hDevice,
                                      PDIOCPARAMETERS ioParam
                                     );


DWORD _stdcall dTVdrvDynamicInit       (void);
DWORD _stdcall dTVdrvDynamicExit       (void);
DWORD _stdcall dTVdrvInitComplete      (void);

}


CIOAccessDevice * ioDevice;


//---------------------------------------------------------------------------
// Perform device IO control
//---------------------------------------------------------------------------
DWORD _stdcall dTVdrvW32DeviceIOControl(DWORD           dwService,
                        DWORD           dwDDB,
                        DWORD           hDevice,
                        PDIOCPARAMETERS ioParam)
{
  debugOut(dbTrace,"dTVdrvW32DeviceIOControl() %X",dwService);

  ioParam->dwIoControlCode = dwService;

  return ioDevice->deviceIOControl(ioParam);
 }

//---------------------------------------------------------------------------
// This function is called when the driver was loaded by Windows 95
// a static or dynamic vxd call your own startup code function from
// this function
//---------------------------------------------------------------------------
DWORD _stdcall dTVdrvDynamicInit(void)
 {
  debugInitialize("dTVdrv95",0xFFFFFFFFL);

  debugOut(dbTrace,"dTVdrvDynamicInit()");

  ioDevice = new CIOAccessDevice();

  return( ioDevice != 0);
 }


//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
DWORD _stdcall dTVdrvDynamicExit(void)
 {
  debugOut(dbTrace,"dTVdrvDynamicExit()");

  delete ioDevice;

  return 1;
 }


//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
DWORD _stdcall dTVdrvInitComplete(void)
 {
  debugOut(dbTrace,"dTVdrvInitComplete()");

  return 1;
}


