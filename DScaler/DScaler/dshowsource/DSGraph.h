/////////////////////////////////////////////////////////////////////////////
// $Id: DSGraph.h,v 1.3 2002-02-05 17:27:47 tobbej Exp $
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
// Change Log
//
// Date          Developer             Changes
//
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.2  2002/02/03 11:02:34  tobbej
// various updates for new filter
//
// Revision 1.1  2001/12/17 19:30:24  tobbej
// class for managing the capture graph
//
//
/////////////////////////////////////////////////////////////////////////////

/**
 * @file DSGraph.h interface for the CDShowGraph class.
 */

#if !defined(AFX_DSGRAPH_H__21DAAF0A_220E_4F8F_A4F6_988DCB31AAE1__INCLUDED_)
#define AFX_DSGRAPH_H__21DAAF0A_220E_4F8F_A4F6_988DCB31AAE1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "exception.h"
#include "CaptureDevice.h"

#include "..\..\..\DSRend\DSRend.h"

class CDShowGraph
{
public:
	/**
	 * Constructor.
	 * @throws 
	 */
	CDShowGraph(string device,string deviceName);
	virtual ~CDShowGraph();

	CDShowCaptureDevice* getCaptureDevice();
	bool getNextSample(CComPtr<IMediaSample> &pSample);
	void getConnectionMediatype(AM_MEDIA_TYPE *pmt);
	void showRendererProperies(HWND hParent);
	int getDroppedFrames();

	void start();
	void stop();
	
private:
	void initGraph();
	void createRenderer();

	void setRes(long x,long y);
	void findStreamConfig();

	///custom video renderer. used for transfering the picture to dscaler
	CComPtr<IBaseFilter> m_renderer;
	CComPtr<IDSRendFilter> m_DSRend;
	CComPtr<IQualProp> m_pQualProp;
	
	///IAMStreamConfig interface for the filter connected to our renderer
	CComPtr<IAMStreamConfig> m_pStreamCfg;

	CComPtr<IGraphBuilder> m_pGraph;
	CComPtr<ICaptureGraphBuilder2> m_pBuilder;
	CComPtr<IMediaControl> m_pControl;
		
	CDShowCaptureDevice *m_pSource;

	//CMediaType m_currentMediaType;

#ifdef _DEBUG
	DWORD m_hROT;
#endif
};

#endif // !defined(AFX_DSGRAPH_H__21DAAF0A_220E_4F8F_A4F6_988DCB31AAE1__INCLUDED_)
