/////////////////////////////////////////////////////////////////////////////
// $Id: TiffSource.h,v 1.2 2001-11-24 22:51:20 laurentg Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 Laurent Garnier.  All rights reserved.
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
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.1  2001/11/24 17:55:23  laurentg
// CTiffSource class added
//
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __TIFFSOURCE_H___
#define __TIFFSOURCE_H___

#include "StillSource.h"

class CTiffSource : public CStillSource
{
public:
    CTiffSource(LPCSTR FilePath);
private:
    BOOL ReadNextFrameInFile();
};

#endif