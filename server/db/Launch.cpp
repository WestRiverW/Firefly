#include "Launch.h"
#include <stdio.h>
#include <assert.h>
#include <glog/logging.h>
#include <share/CommonDefine.h>

Launch::Launch()
{
}

Launch::~Launch()
{
}

bool Launch::Start()
{
    LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " startting service......";

    if (LaunchBase::Start() == false)
    {
        Stop();
        return false;
    }

    m_Timer.SetTimer( CONNECT_TIMER, 1, 1, 0 );
    std::unique_lock <std::mutex> lck( m_mutMain );
    m_condMain.wait( lck );

    if( StartServer() == false )
    {
        Stop();
        return false;
    }

    return true;
}

bool Launch::Init()
{
    if( m_MsgServer.SetNetServer( &m_Bridge ) == false )
    {
        return false;
    }

    if( m_MsgClient.SetClientEvent( &m_Bridge ) == false )
    {
        return false;
    }

    if( m_Bridge.SetServer( &m_MsgServer ) == false )
    {
        return false;
    }

    if( m_Bridge.SetBridgeHook( &m_BridgeHook ) == false )
    {
        return false;
    }

    if( m_Timer.SetTimer( &m_Bridge ) == false )
    {
        return false;
    }

    if( m_DBEngine.SetDBEngineHook( &m_DBEngineHook, 4 ) == false )
    {
        return false;
    }

    m_BridgeHook.m_pIDBEngine = dynamic_cast<IDBEngine *>( &m_DBEngine );
    m_BridgeHook.m_pIMsgServer = dynamic_cast<IMsgServer *>( &m_MsgServer );
    m_BridgeHook.m_pIMsgClient = dynamic_cast<IMsgClient *>( &m_MsgClient );
    m_BridgeHook.m_pTimer = &m_Timer;
    m_BridgeHook.m_pLaunchBase = this;
    m_BridgeHook.m_pBridge = &m_Bridge;
    m_DBEngineHook.m_pIDBEngineEvent = dynamic_cast<IDBEngine *>( &m_Bridge );
    m_DBEngineHook.m_Timer = &m_Timer;
    m_ContactCenter.SetMsgClient( dynamic_cast<IMsgClient *>( &m_MsgClient ) );
    m_BridgeHook.m_pContactCenter = &m_ContactCenter;
    LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " init good......";
    return true;
}

bool Launch::StartCore()
{
    if (LaunchBase::StartCore() == false)
    {
        assert(false);
        return false;
    }

    if( m_DBEngine.Start() == false )
    {
        assert( false );
        return false;
    }

    return true;
}

bool Launch::StartServer()
{
    //m_MsgServer.SetInfo(m_ServerInfo.nport(), m_ServerInfo.nmaxconnection() );

    if( m_MsgServer.Start() == false )
    {
        return false;
    }

    return true;
}