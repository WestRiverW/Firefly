/*
*   Timer.h
*
*   Timer
*
*   Created on: 2018-11-14
*   Author:
*   All rights reserved.
*/
#ifndef __Timer_H__
#define __Timer_H__

#include <vector>
#include <mutex>
#include <common/BaseCore.h>
#include "Thread.h"

namespace Firefly
{
    class Timer;

    class TimerThread : public Thread
    {
    protected:
        unsigned long long        m_llTimerSpace;
        unsigned long long        m_llLastTickCount;
        unsigned long long        m_llLastMoreSleep;
        unsigned long long        m_llBeforeSleepTime;
        unsigned long long        m_llAfterSleepTime;
        unsigned long long        m_llHeartCount;
        unsigned long long        m_llStartTickCount;

    protected:
        Timer* m_pTimer;

    public:
        TimerThread();
        virtual ~TimerThread();

    public:
        bool InitThread(Timer* pTimer, unsigned int dwTimerSpace);
        std::string GetThreadFlag();

    private:
        virtual bool OnStrat();
        virtual bool OnRun();
    };
    //////////////////////////////////////////////////////////////////////////

    struct tagTimerItem
    {
        unsigned long long            llElapse;
        unsigned long long            llTimeLeave;
        unsigned long long            llExeCount;
        unsigned int            unTimerID;
        unsigned int            dwRepeatTimes;
        unsigned int            unParam;
    };

    typedef std::vector<tagTimerItem*> CTimerItemArray;

    class Timer : public ITimer
    {
        friend class TimerThread;

    protected:
        bool                    m_bService;
        unsigned long long            m_llTimePass;
        unsigned long long            m_llTimeLeave;

    protected:
        unsigned long long            m_llTimerSpace;
        ITimerEvent* m_pITimerEvent;

    protected:
        TimerThread             m_TimerThread;
        CTimerItemArray         m_TimerItemFree;
        CTimerItemArray         m_TimerItemActive;
        std::mutex              m_CriticalSection;

        unsigned long long      m_llStartTickCount;
        unsigned long long      m_llLastTickCount;
        unsigned int            m_nTimeCount;

    public:
        Timer();
        virtual ~Timer();

    public:
        virtual bool Start();
        virtual bool Stop();

    public:
        virtual bool SetTimer(ITimerEvent* pObject);

    public:
        //dwElapse interval，dwRepeat:count.
        virtual bool SetTimer(unsigned int unTimerID, unsigned int dwElapse, unsigned int dwRepeat, unsigned int unParam);
        virtual bool KillTimer(unsigned int unTimerID);
        virtual bool KillAllTimer();

    private:
        void OnTimerThreadHook();
    };
}

#endif