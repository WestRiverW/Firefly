#include "Timer.h"
#include <assert.h>
#include <unistd.h>
#include "Utility.h"
#include <glog/logging.h>

namespace Firefly
{
    const unsigned long long NO_TIME_LEAVE = 0xFFFFFFFFFFFFFFFF;

    TimerThread::TimerThread()
    {
        m_llLastTickCount = 0;
        m_llTimerSpace = 50000;
        m_pTimer = NULL;
        return;
    }

    TimerThread::~TimerThread()
    {
    }

    bool TimerThread::InitThread(Timer* pTimer, unsigned int dwTimerSpace)
    {
        assert(dwTimerSpace >= 1L);
        assert(pTimer != NULL);
        if (pTimer == NULL) return false;

        m_llLastTickCount = 0;
        m_llTimerSpace = dwTimerSpace;
        m_pTimer = pTimer;
        m_llLastMoreSleep = 0;
        m_llHeartCount = 0;
        m_llStartTickCount = Utility::GetElapseTime();
        return true;
    }

    std::string TimerThread::GetThreadFlag()
    {
        std::string data = "[TimeThread] ";
        return data;
    }

    bool TimerThread::OnStrat()
    {
        return true;
    }

    bool TimerThread::OnRun()
    {
        assert(m_pTimer != NULL);
        unsigned long long llTimerSpace = m_llTimerSpace;
        unsigned long long llNowTickCount = Utility::GetElapseTime();

        if ((m_llLastTickCount != 0) && (llNowTickCount > m_llLastTickCount))
        {
            unsigned long long llHandleTickCount = llNowTickCount - m_llLastTickCount;
            llTimerSpace = (llTimerSpace > llHandleTickCount) ? (llTimerSpace - llHandleTickCount) : 0;
        }

        llTimerSpace = (llTimerSpace > m_llLastMoreSleep) ? (llTimerSpace - m_llLastMoreSleep) : 0;
        unsigned long long llElapseTickCount = Utility::GetElapseTime() - m_llStartTickCount;
        llElapseTickCount = llElapseTickCount / m_llTimerSpace;

        if (m_llHeartCount != llElapseTickCount)
        {
            llTimerSpace = 0;
        }

        m_llBeforeSleepTime = Utility::GetElapseTime();

        if (llTimerSpace > 0)
        {
            usleep(llTimerSpace);
        }

        m_llAfterSleepTime = Utility::GetElapseTime();
        m_llLastMoreSleep = m_llAfterSleepTime - m_llBeforeSleepTime;
        m_llLastMoreSleep = (m_llLastMoreSleep > llTimerSpace) ? (m_llLastMoreSleep - llTimerSpace) : 0;
        m_llLastTickCount = Utility::GetElapseTime();
        m_pTimer->OnTimerThreadHook();
        ++m_llHeartCount;
        return true;
    }

    Timer::Timer()
    {
        m_bService = false;
        m_llTimePass = 0L;
        m_llTimeLeave = NO_TIME_LEAVE;
        m_llTimerSpace = 50000;
        m_pITimerEvent = NULL;
        return;
    }

    Timer::~Timer()
    {
        Stop();
        tagTimerItem* pTimerItem = NULL;

        for (size_t i = 0; i < m_TimerItemFree.size(); i++)
        {
            pTimerItem = m_TimerItemFree[i];
            assert(pTimerItem != NULL);
            delete pTimerItem;
            pTimerItem = NULL;
        }

        for (size_t i = 0; i < m_TimerItemActive.size(); i++)
        {
            pTimerItem = m_TimerItemActive[i];
            assert(pTimerItem != NULL);
            delete pTimerItem;
            pTimerItem = NULL;
        }

        m_TimerItemFree.clear();
        m_TimerItemActive.clear();
        return;
    }

    bool Timer::SetTimer(unsigned int unTimerID, unsigned int dwElapse, unsigned int dwRepeat, unsigned int unParam)
    {
        std::unique_lock<std::mutex> ThreadLock(m_CriticalSection);
        assert(dwRepeat > 0L);
        if (dwRepeat == 0) return false;

        unsigned int dwTimerSpace = m_llTimerSpace / 1000;
        dwElapse = (dwElapse * 1000 + dwTimerSpace - 1) / dwTimerSpace * dwTimerSpace;
        bool bTimerExist = false;
        tagTimerItem* pTimerItem = NULL;

        for (size_t i = 0; i < m_TimerItemActive.size(); i++)
        {
            pTimerItem = m_TimerItemActive[i];
            if (pTimerItem->unTimerID == unTimerID)
            {
                bTimerExist = true;
                break;
            }
        }

        if (bTimerExist == false)
        {
            int nFreeCount = m_TimerItemFree.size();

            if (nFreeCount > 0)
            {
                pTimerItem = m_TimerItemFree[nFreeCount - 1];
                assert(pTimerItem != NULL);
                m_TimerItemFree.erase(m_TimerItemFree.begin() + nFreeCount - 1);
            }
            else
            {
                try
                {
                    pTimerItem = new tagTimerItem;
                    assert(pTimerItem != NULL);

                    if (pTimerItem == NULL) return false;
                }
                catch (...)
                {
                    return false;
                }
            }
        }

        dwElapse = dwElapse * 1000;
        assert(pTimerItem != NULL);
        pTimerItem->llElapse = dwElapse;
        pTimerItem->llTimeLeave = Utility::GetElapseTime();
        pTimerItem->llExeCount = 1;
        pTimerItem->unTimerID = unTimerID;
        pTimerItem->dwRepeatTimes = dwRepeat;
        pTimerItem->unParam = unParam;
        unsigned long long llExeTime = pTimerItem->llTimeLeave + pTimerItem->llExeCount * pTimerItem->llElapse;
        //m_llTimeLeave = std::min( m_llTimeLeave, pTimerItem->llTimeLeave );
        m_llTimeLeave = (m_llTimeLeave > llExeTime) ? llExeTime : m_llTimeLeave;
        if (bTimerExist == false) m_TimerItemActive.push_back(pTimerItem);

        return false;
    }

    bool Timer::KillTimer(unsigned int unTimerID)
    {
        std::unique_lock<std::mutex> ThreadLock(m_CriticalSection);
        for (size_t i = 0; i < m_TimerItemActive.size(); i++)
        {
            tagTimerItem* pTimerItem = m_TimerItemActive[i];
            if (pTimerItem->unTimerID != unTimerID) continue;
            m_TimerItemActive.erase(m_TimerItemActive.begin() + i);
            m_TimerItemFree.push_back(pTimerItem);

            if (m_TimerItemActive.size() == 0)
            {
                m_llTimePass = 0L;
                m_llTimeLeave = NO_TIME_LEAVE;
            }

            return true;
        }

        return false;
    }

    bool Timer::KillAllTimer()
    {
        std::unique_lock<std::mutex> ThreadLock(m_CriticalSection);
        m_TimerItemFree.insert(m_TimerItemFree.end(), m_TimerItemActive.begin(), m_TimerItemActive.end());
        m_TimerItemActive.clear();
        m_llTimePass = 0L;
        m_llTimeLeave = NO_TIME_LEAVE;
        return true;
    }

    bool Timer::Start()
    {
        assert(m_bService == false);
        if (m_bService == true) return false;

        m_llStartTickCount = Utility::GetElapseTime();
        m_llLastTickCount = m_llStartTickCount;
        m_nTimeCount = 0;
        m_llTimePass = 0L;
        m_llTimeLeave = NO_TIME_LEAVE;
        if (m_TimerThread.InitThread(this, m_llTimerSpace) == false) return false;
        if (m_TimerThread.StartThread() == false) return false;

        m_bService = true;
        return true;
    }

    bool Timer::Stop()
    {
        m_bService = false;
        m_TimerThread.StopThread();
        KillAllTimer();
        return true;
    }

    bool Timer::SetTimer(ITimerEvent* pObject)
    {
        assert(m_bService == false);
        if (m_bService == true) return false;

        if (pObject != NULL)
        {
            assert(pObject != NULL);
            m_pITimerEvent = pObject;

            if (m_pITimerEvent == NULL) return false;
        }
        else m_pITimerEvent = NULL;
        return true;
    }

    void Timer::OnTimerThreadHook()
    {
        std::unique_lock<std::mutex> ThreadLock(m_CriticalSection);

        if (m_llTimeLeave == NO_TIME_LEAVE)
        {
            assert(m_TimerItemActive.size() == 0);
            return;
        }

        // unsigned long long llNowTickCount=Utility::GetElapseTime();
        // unsigned long long llElapseTickCount = llNowTickCount-m_llStartTickCount;
        // unsigned int nDeltaTickCount = llNowTickCount-m_llLastTickCount;
        // m_llLastTickCount=llNowTickCount;
        // m_nTimeCount++;
        // LOG(INFO)<<"OnTimerThreadHookaaab:ElapseTickCount="<<llElapseTickCount/m_llTimerSpace<<"  timercount="<<m_nTimeCount<<"  delta="<<nDeltaTickCount;
        m_llTimePass = Utility::GetElapseTime();

        if (m_llTimeLeave <= m_llTimePass)
        {
            bool bKillTimer = false;
            unsigned long long llTimeLeave = NO_TIME_LEAVE;

            for (size_t i = 0; i < m_TimerItemActive.size(); )
            {
                tagTimerItem* pTimerItem = m_TimerItemActive[i];
                bKillTimer = false;
                unsigned long long llExeTime = pTimerItem->llTimeLeave + pTimerItem->llExeCount * pTimerItem->llElapse;

                if (llExeTime <= m_llTimePass)
                {
                    assert(m_pITimerEvent != NULL);
                    m_pITimerEvent->OnTimer(pTimerItem->unTimerID, pTimerItem->unParam);

                    if (pTimerItem->dwRepeatTimes != TIMES_INFINITY)
                    {
                        if (pTimerItem->dwRepeatTimes == 1L)
                        {
                            bKillTimer = true;
                            m_TimerItemActive.erase(m_TimerItemActive.begin() + i);
                            m_TimerItemFree.push_back(pTimerItem);
                        }
                        else pTimerItem->dwRepeatTimes--;
                    }

                    if (bKillTimer == false) ++pTimerItem->llExeCount;//pTimerItem->llTimeLeave = pTimerItem->llElapse + m_llTimePass;
                }

                if (bKillTimer == false)
                {
                    i++;
                    //llTimeLeave = std::min(llTimeLeave, pTimerItem->llTimeLeave);
                    llExeTime = pTimerItem->llTimeLeave + pTimerItem->llExeCount * pTimerItem->llElapse;
                    llTimeLeave = (llTimeLeave > llExeTime) ? llExeTime : llTimeLeave;
                }
            }

            m_llTimeLeave = llTimeLeave;
        }

        return;
    }
}