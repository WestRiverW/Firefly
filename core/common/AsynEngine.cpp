#include <assert.h>
#include <memory.h>
#include <glog/logging.h>

#include "AsynEngine.h"

namespace Firefly
{
    AsynThread::AsynThread()
    {
        m_pIAsynEngineHook = NULL;
        memset( m_cbBuffer, 0, sizeof( m_cbBuffer ) );
    }

    AsynThread::~AsynThread()
    {
    }

    void AsynThread::SetAsynEngineHook( IAsynEngineHook *pIAsynEngineHook )
    {
        m_pIAsynEngineHook = pIAsynEngineHook;
    }

    std::string AsynThread::GetThreadFlag()
    {
        std::string data = "[AsynTaskThread] ";
        return data;
    }

    bool AsynThread::OnStart()
    {
        assert( m_pIAsynEngineHook != NULL );
        bool bSuccess = m_pIAsynEngineHook->OnAsynEngineStart();
        m_pAsynEngine->m_bService = true;
        return bSuccess;
    }

    bool AsynThread::OnStop()
    {
        m_pAsynEngine->m_bService = false;
        assert( m_pIAsynEngineHook != NULL );
        bool bSuccess = m_pIAsynEngineHook->OnAsynEngineStop();
        return bSuccess;
    }

    bool AsynThread::OnRun()
    {
        assert( m_pIAsynEngineHook != NULL );
        std::unique_lock <std::mutex> lck( m_pAsynEngine->m_muxAsync );
        m_pAsynEngine->m_condAsync.wait( lck );
        LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " AsynThread::OnRun()";

        if( m_pAsynEngine == NULL )
        {
            LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " AsynEngine == NULL";
            return false;
        }
    
        if( !m_pAsynEngine->m_bService )
        {
            LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " AsynEngine == NULL";
            return false;
        }

        do
        {
            tagDataHead DataHead;
            int nRemainTask = 0;
            {
                std::unique_lock <std::mutex> lck( m_pAsynEngine->m_CriticalSection );
                nRemainTask = m_pAsynEngine->m_DataQueue.Size();

                if( 0 < nRemainTask )
                {
                    m_pAsynEngine->m_DataQueue.DistillData( DataHead, m_cbBuffer, sizeof( m_cbBuffer ) );
                }
                else
                {
                    break;
                }
            }

            try
            {
                m_pIAsynEngineHook->OnAsynEngineData( DataHead.wIdentifier, m_cbBuffer, DataHead.nDataSize );
            }
            catch( ... )
            {
                return false;
            }
        }
        while( true );

        return true;
    }

    //////////////////////////////////////////////////////////////////////////
    AsynEngine::AsynEngine()
    {
        m_bService = false;
        m_pIAsynEngineHook = NULL;
    }

    AsynEngine::~AsynEngine()
    {
        Stop();
    }

    bool AsynEngine::Start()
    {
        assert( m_bService == false );
        if( m_bService == true ) return false;

        m_AsynThread.SetAsynEngineHook( m_pIAsynEngineHook );
        m_AsynThread.m_pAsynEngine = this;

        if( m_AsynThread.StartThread() == false )
        {
            assert( false );
            return false;
        }

        return true;
    }

    bool AsynEngine::Stop()
    {
        m_bService = false;
        m_condAsync.notify_one();
        m_AsynThread.StopThread();
        m_pIAsynEngineHook = NULL;
        m_AsynThread.SetAsynEngineHook( NULL );
        m_DataQueue.RemoveData( false );
        return true;
    }

    bool AsynEngine::SetAsynHook( FFObject *pObject )
    {
        m_pIAsynEngineHook = dynamic_cast<IAsynEngineHook *>( pObject );
        return true;
    }

    bool AsynEngine::PostAsynData( unsigned short wIdentifier, void *pData, unsigned int nDataSize )
    {
        assert( ( m_bService == true ) );
        if( m_bService == false ) return false;

        std::unique_lock<std::mutex> ThreadLock( m_CriticalSection );
        if( m_DataQueue.InsertData( wIdentifier, pData, nDataSize ) == false )
        {
            assert( false );
            return false;
        }

        m_condAsync.notify_one();
        LOG_IF( INFO, 6 != wIdentifier ) << strThreadLogFlag << __FUNCTION__ << ",wIdentifier:" << wIdentifier << ",nDataSize:" << nDataSize << ",size:" << m_DataQueue.Size() << ",this:" << this;
        return true;
    }
}