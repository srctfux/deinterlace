/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 John Adcock.  All rights reserved.
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

/**
 * @file sourceprovider.h sourceprovider Header file
 */

#ifndef __SOURCEPROVIDER_H___
#define __SOURCEPROVIDER_H___

#include "Source.h"

/** Interface for a class that can find mutiple sources on the system.
    It is intended that each PCI brige chip has it's own source provider
*/
class ISourceProvider
{
public:
    virtual int GetNumberOfSources() = 0;
    virtual SmartPtr<CSource> GetSource(int SourceIndex) = 0;
};

#endif