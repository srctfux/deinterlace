/////////////////////////////////////////////////////////////////////////////
// $Id: events.h,v 1.7 2002-10-11 21:49:11 ittarnavsky Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2002 Jeroen Kooiman.  All rights reserved.
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
//
// $Log: not supported by cvs2svn $
/////////////////////////////////////////////////////////////////////////////

#ifndef __EVENTS_H___
#define __EVENTS_H___

//See top of events.cpp for a description of how to use it

#include <vector>
#include <deque>

enum eEventType
{
    EVENT_NONE = 0,
    EVENT_INIT,
    EVENT_DESTROY,
    EVENT_SOURCE_PRECHANGE,
    EVENT_SOURCE_CHANGE,
    EVENT_VIDEOINPUT_PRECHANGE,
    EVENT_VIDEOINPUT_CHANGE,
    EVENT_AUDIOINPUT_PRECHANGE,
    EVENT_AUDIOINPUT_CHANGE,
    EVENT_VIDEOFORMAT_PRECHANGE,
    EVENT_VIDEOFORMAT_CHANGE,
    EVENT_CHANNEL_PRECHANGE,
    EVENT_CHANNEL_CHANGE,
	EVENT_MUTE,
	EVENT_VOLUME,
	EVENT_MIXERVOLUME,
	EVENT_AUDIOSTANDARD_DETECTED,
	EVENT_AUDIOCHANNELSUPPORT_DETECTED,
	EVENT_SOUNDCHANNEL
};
#define EVENT_ENDOFLIST EVENT_NONE


class CEventObject;

typedef void (__cdecl EVENTCALLBACK)(void *pThis, CEventObject *pEventObject, eEventType Event, long OldValue, long NewValue, eEventType *ComingUp);


class CEventObject
{
public:
    CEventObject();
    ~CEventObject();
    virtual void OnEvent(CEventObject *pEventObject, eEventType Event, long OldValue, long NewValue, eEventType *ComingUp) {;}
};

class CEventCollector
{
protected:
    typedef struct 
    {
        eEventType *EventList;
        void *pThis;
        EVENTCALLBACK *pfnEventCallback;
        CEventObject *pEventObject;
    } TEventCallbackInfo;
    
    vector<TEventCallbackInfo> m_EventObjects;

	typedef struct
	{
		CEventObject *pEventObject;
		eEventType Event;
		long	   OldValue;
		long	   NewValue;
		eEventType *ComingUp;
	} TEventInfo;

	deque<TEventInfo> m_ScheduledEventList;
	CRITICAL_SECTION  m_EventCriticalSection;
	CRITICAL_SECTION  m_LastEventCriticalSection;
	long			  m_ScheduleTimerID;

	HANDLE			  m_EventCollectorThread;		
    BOOL			  m_bStopThread;

	vector<TEventInfo> m_LastEvents;
    
	void StartThread();
    void StopThread();
protected:
    eEventType *CEventCollector::CopyEventList(eEventType *EventList);

	void RaiseScheduledEvent(CEventObject *pObject, eEventType Event, long OldValue, long NewValue, eEventType *ComingUp);
    void ScheduleEvent(CEventObject *pObject, eEventType Event, long OldValue, long NewValue, eEventType *ComingUp);

	static VOID CALLBACK StaticEventTimerWrap(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
public:
    CEventCollector();
    ~CEventCollector();

    void Register(EVENTCALLBACK *pfnCallback, void *pThis, eEventType *EventList);
    void Unregister(EVENTCALLBACK *pfnCallback, void *pThis);

    void Register(CEventObject *pObject, eEventType *EventList);
    void Unregister(CEventObject *pObject);

	void EventTimer();

    void RaiseEvent(CEventObject *pEventObject, eEventType Event, long OldValue, long NewValue, eEventType *ComingUp = NULL);    

	int LastEventValues(eEventType Event, CEventObject **pObject, long *OldValue, long *NewValue);
	int LastEventValues(CEventObject *pObject, eEventType Event, long *OldValue, long *NewValue);
	int NumEventsWaiting();
	
};

//Defined, allocated & destroyed in Dscaler.cpp
extern CEventCollector *EventCollector;

#endif
