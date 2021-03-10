#include "Thread.h"
#include <assert.h>
#include <mutex>
#include <condition_variable>

namespace Firefly
{
    thread_local std::string strThreadLogFlag;

    struct tagThreadParameter
    {
        bool                    bSuccess;
        Thread* pServiceThread;
        std::mutex              mutThread;
        std::condition_variable condThread;
    };

    Thread::Thread()
    {
        m_bRun = false;
        m_pThreadID = nullptr;
    }

    Thread::~Thread()
    {
        StopThread();
    }

    bool Thread::StartThread()
    {
        assert(m_bRun == false);
        tagThreadParameter ThreadParameter;
        ThreadParameter.bSuccess = false;
        ThreadParameter.pServiceThread = this;
        m_bRun = true;
        m_pThreadID = new std::thread(&Thread::ThreadFunction, this, &ThreadParameter);

        if (m_pThreadID == nullptr)
        {
            return false;
        }

        std::unique_lock <std::mutex> lck(ThreadParameter.mutThread);
        ThreadParameter.condThread.wait(lck);

        if (ThreadParameter.bSuccess == false)
        {
            StopThread();
        }

        return true;
    }

    bool Thread::StopThread()
    {
        if (m_bRun)
        {
            m_bRun = false;
            this->Join();
            delete m_pThreadID;
            m_pThreadID = nullptr;
        }

        return true;
    }

    void Thread::Join()
    {
        if (m_pThreadID && m_pThreadID->joinable())
        {
            m_pThreadID->join();
        }
    }

    void Thread::ThreadFunction(void* pData)
    {
        tagThreadParameter* pThreadParameter = (tagThreadParameter*)pData;
        Thread* pServiceThread = pThreadParameter->pServiceThread;

        try
        {
            pThreadParameter->bSuccess = pServiceThread->OnStart();
        }
        catch (...)
        {
            assert(false);
            pThreadParameter->bSuccess = false;
        }

        //accept
        pThreadParameter->condThread.notify_one();

        while (m_bRun)
        {
            try
            {
                pServiceThread->OnRun();
            }
            catch (...)
            {
                assert(false);
            }
        }

        try
        {
            pServiceThread->OnStop();
        }
        catch (...)
        {
            assert(false);
        }
    }
}