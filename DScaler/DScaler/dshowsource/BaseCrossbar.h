/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 Torbj�rn Jansson.  All rights reserved.
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
 * @file BaseCrossbar.h interface for the CBaseCrossbar class.
 */

#if !defined(AFX_BASECROSSBAR_H__FF62818A_2194_4F12_AA7C_B8D9AB84D0CC__INCLUDED_)
#define AFX_BASECROSSBAR_H__FF62818A_2194_4F12_AA7C_B8D9AB84D0CC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DSObject.h"
#include "exception.h"

/**
 * Exception class for crossbars
 * @see CDShowException
 * @see CSingleCrossbar
 */
class CCrossbarException: public CDShowException
{
public:
    CCrossbarException(CString msg,HRESULT hr):CDShowException(msg,hr) {};
    CCrossbarException(CString msg):CDShowException(msg) {};
};

/**
 * Base class for crossbars.
 * 
 */
class CDShowBaseCrossbar : public CDShowObject
{
public:
    CDShowBaseCrossbar(IGraphBuilder *pGraph);
    virtual ~CDShowBaseCrossbar();

    eDSObjectType getObjectType() {return DSHOW_TYPE_CROSSBAR;}

    /**
     * Get number of input and output pins.
     */
    virtual void GetPinCounts(long &cIn,long &cOut)=0;
    
    /**
     * @return Type of specified input
     */
    virtual PhysicalConnectorType GetInputType(long Index)=0;
    
    /**
     * @param Index input number
     * @return Name of input
     */
    virtual char* GetInputName(long Index);
    
    /**
     * Select input.
     * @param Index input index
     * @param bSetRelated a bool that specifies if the related pin is also to be selected, for example if you selecte a video input it will also set the corect audio input
     */
    virtual void SetInputIndex(long Index,bool bSetRelated)=0;
    
    /**
     * Which input is connected to specified output.
     * This function returns the input index of the pin that is routed to specified output
     * @param OutIndex output pin index.
     * @return index for currently selected index
     */
    virtual long GetInputIndex(long OutIndex)=0;
    
    /**
     * @param index input pin index.
     * @return true if specified input is routed to an output.
     */
    virtual bool IsInputSelected(long index)=0;
};

#endif // !defined(AFX_BASECROSSBAR_H__FF62818A_2194_4F12_AA7C_B8D9AB84D0CC__INCLUDED_)
