/*
*   Thread.h
*
*   Thread Base Class.
*
*   Created on: 2018-11-07
*   Author:
*   All rights reserved.
*/
#ifndef __Thread_H__
#define __Thread_H__

#include <thread>

using namespace std;

namespace Firefly
{
    class Thread
    {
    public:
        Thread();
        virtual ~Thread();

    public:
        virtual bool StartThread();
        virtual bool StopThread();
        virtual void Join();
    public:
        void join();
        virtual std::string GetThreadFlag()
        {
            return "[default] ";
        }
    protected:
        virtual bool OnStart()
        {
            return true;
        }
        virtual bool OnRun()
        {
            return true;
        }
        virtual bool OnStop()
        {
            return true;
        }

    private:
        void ThreadFunction(void* pData);

    private:
        bool        m_bRun;

    private:
        std::thread* m_pThreadID;
    };
}

#endif