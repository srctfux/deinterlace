/////////////////////////////////////////////////////////////////////////////
// $Id: Perf.h,v 1.5 2003-02-22 13:36:37 laurentg Exp $
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
// Revision 1.4  2002/10/27 20:39:08  laurentg
// Performance statistics only computed in DEBUG buildd
// Developer OSD screen only present in DEBUG build
//
// Revision 1.3  2002/01/31 13:02:46  robmuller
// Improved accuracy and reliability of the performance statistics.
//
// Revision 1.2  2001/12/16 16:31:43  adcockj
// Bug fixes
//
// Revision 1.1  2001/12/16 13:00:51  laurentg
// New statistics
//
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __PERF_H___
#define __PERF_H___

enum ePerfType
{
    PERF_WAIT_FIELD = 0,
#ifdef _DEBUG
    PERF_INPUT_FILTERS,
    PERF_OUTPUT_FILTERS,
    PERF_PULLDOWN_DETECT,
    PERF_DEINTERLACE,
    PERF_RATIO,
    PERF_CALIBRATION,
    PERF_TIMESHIFT,
    PERF_VBI,
    PERF_LOCK_OVERLAY,
    PERF_UNLOCK_OVERLAY,
    PERF_FLIP_OVERLAY,
#endif
    PERF_TYPE_LASTONE,
};

class CPerfItem
{
public:
	void Resume();
	void Suspend();
    CPerfItem(const char* Name);
    ~CPerfItem();
    void Reset();
    void StartCount();
    void StopCount();
    const char* GetName();
    DWORD GetLastDuration();
    unsigned int GetNbCounts();
    DWORD GetAverageDuration();
    DWORD GetMinDuration();
    DWORD GetMaxDuration();

protected:
    const char*     m_Name;
    DWORD           m_LastDuration;
    DWORD           m_SumDuration;
    unsigned int    m_NbCounts;
    DWORD           m_MinDuration;
    DWORD           m_MaxDuration;

private:
	DWORD           m_SuspendCounter;
    DWORD           m_TickStart;
    BOOL            m_IsCounting;
};

class CPerf
{
public:
	void Resume();
	void Suspend();
    CPerf();
    ~CPerf();
    void Reset();
    void InitCycle();
    void StartCount(ePerfType PerfType);
    void StopCount(ePerfType PerfType);
    int GetDurationLastCycle(ePerfType PerfType);
    unsigned int GetNbCycles(int NbFramesPerSec);
    BOOL IsValid(ePerfType PerfType);
    const char* GetName(ePerfType PerfType);
    DWORD GetLastDuration(ePerfType PerfType);
    unsigned int GetNbCounts(ePerfType PerfType);
    DWORD GetAverageDuration(ePerfType PerfType);
    DWORD GetMinDuration(ePerfType PerfType);
    DWORD GetMaxDuration(ePerfType PerfType);
    int GetNumberDroppedFields();
    int GetNumberLateFields();
    int GetNumberUsedFields();
    int GetNumberNoFlipAtTime();
    double GetAverageDroppedFields();
    double GetAverageLateFields();
    double GetAverageUsedFields();
    double GetAverageNoFlipAtTime();
    int GetDroppedFieldsLastSecond();
    int GetLateFieldsLastSecond();
    int GetUsedFieldsLastSecond();
    int GetNoFlipAtTimeLastSecond();

private:
    CPerfItem*  m_PerfItems[PERF_TYPE_LASTONE];
    BOOL        m_PerfCalculated[PERF_TYPE_LASTONE];
    DWORD       m_TickStart;
    DWORD       m_TickStartLastSec;
    int         m_TotalDroppedFields;
    int         m_TotalLateFields;
    int         m_TotalUsedFields;
    int         m_TotalNoFlipAtTime;
    double      m_DroppedFieldsLastSec;
    double      m_LateFieldsLastSec;
    double      m_UsedFieldsLastSec;
    double      m_NoFlipAtTimeLastSec;
    BOOL        m_ResetRequested;
};

extern CPerf* pPerf;

#endif