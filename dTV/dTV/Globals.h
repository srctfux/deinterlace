/////////////////////////////////////////////////////////////////////////////
// globals.h
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
// ########### IMPORTANT ###########
//
// Globals are ugly and complicates keeping track of code.
// If you see variables that are used by only one module, please 
// delete it from here, and declare the variable at the top of the module 
// and then delete the 'extern' declaration.  Your Computer Science class
// should have taught you to avoid excessive global variables ;-)
// It will also make it easier to rewrite to C++ as well in the future.
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
// 02 Jan 2001   John Adcock           Added CurrentVBILines and removed BurstDMA
//
// 05 Jan 2001   John Adcock           Added DoAccurateFlips
//
// 08 Jan 2001   John Adcock           Started Global Variable Tidy up
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __GLOBALS_H___
#define __GLOBALS_H___

#endif
