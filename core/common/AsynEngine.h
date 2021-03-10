/*
*   AsynEngine.h
*
*   Ansy Process.
*
*   Created on: 2018-11-13
*   Author:
*   All rights reserved.
*/
#ifndef __AsynEngine_H__
#define __AsynEngine_H__

#include <mutex>
#include <condition_variable>

#include <utils/Thread.h>
#include "BaseCore.h"

namespace Firefly
{
    class AsynEngine;

    class AsynThread : public Thread
    {
        friend class AsynEngine;
    public:
        AsynThread();
        virtual ~AsynThread();

    public:
        void SetAsynEngineHook(IAsynEngineHook* pIAsynEngineHook);
        std::string GetThreadFlag();

    public:
        virtual bool OnStart();
        virtual bool OnRun();
        virtual bool OnStop();

    protected:
        IAsynEngineHook* m_pIAsynEngineHook;
        AsynEngine* m_pAsynEngine;
    private:
        char                m_cbBuffer[ASYN_DATA_LEN];
    };

    class AsynEngine : public IAsynEngine
    {
        friend class AsynThread;

    public:
        AsynEngine();
        virtual ~AsynEngine();

    public:
        virtual bool Start();
        virtual bool Stop();

    public:
        virtual bool SetAsynHook(FFObject* pObject);
        virtual bool PostAsynData(unsigned short wIdentifier, void* pData, unsigned int wDataSize);

    protected:
        bool                   m_bService;
        IAsynEngineHook* m_pIAsynEngineHook;

    protected:
        DataQueue              m_DataQueue;
        std::mutex             m_CriticalSection;
        AsynThread             m_AsynThread;
        std::mutex                      m_muxAsync;
        std::condition_variable         m_condAsync;
    };
}

#endif