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
    Stop();
}

bool Launch::Start()
{
    LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " startting service......";

    if( Init() == false )
    {
        LOG( ERROR ) << "init faild......";
        Stop();
        return false;
    }

    if( StartCore() == false )
    {
        LOG( ERROR ) << "kernel service faild to stard......";
        Stop();
        return false;
    }

    m_Timer.SetTimer( CONNECT_TIMER, 1, 1, SERVE_TYPE_CENTER );
    std::unique_lock <std::mutex> lck( m_mutMain );
    m_condMain.wait( lck );

    if( StartServer() == false )
    {
        LOG( ERROR ) << "net work service faild to stard......";
        Stop();
        return false;
    }

    return true;
}

bool Launch::Init()
{
    LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " initializing service......";
    if( m_MsgServer.SetNetServer( &m_Bridge ) == false ) return false;
    if( m_MsgClient.SetClientEvent( &m_Bridge ) == false ) return false;
    if( m_Bridge.SetServer( &m_MsgServer ) == false ) return false;
    if( m_Bridge.SetBridgeHook( &m_BridgeHook ) == false ) return false;
    if( m_Timer.SetTimer( &m_Bridge ) == false ) return false;

    m_BridgeHook.m_pIMsgServer = dynamic_cast<IMsgServer *>( &m_MsgServer );
    m_BridgeHook.m_pIMsgClient = dynamic_cast<IMsgClient *>( &m_MsgClient );
    m_BridgeHook.m_pTimer = &m_Timer;
    m_BridgeHook.m_pLaunchBase = this;
    m_BridgeHook.m_pBridge = &m_Bridge;
    m_ContactCenter.SetMsgClient( dynamic_cast<IMsgClient *>( &m_MsgClient ) );
    m_BridgeHook.m_pContactCenter = &m_ContactCenter;
    int webport = 56003;//m_BridgeHook.OnWebServerPort();
    m_HttpSvr.InitThread( &m_BridgeHook, ( unsigned short )webport );
    LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " init good......" << webport;
    return true;
}

bool Launch::StartCore()
{
    LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " kernel service startting......";
    if( m_Timer.Start() == false )
    {
        LOG( ERROR ) << "Timer faild to stard......";
        assert( false );
        return false;
    }

    if( m_Bridge.Start() == false )
    {
        LOG( ERROR ) << "Bridge faild to stard......";
        assert( false );
        return false;
    }

    if( m_MsgClient.Start() == false )
    {
        LOG( ERROR ) << "MsgClient faild to stard......";
        assert( false );
        return false;
    }

    LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " HttpSvr work service startting 111 ";

    if( m_HttpSvr.StartThread() == false )
    {
        LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " HttpSvr work service startting 222";
        return false;
    }

    return true;
}

bool Launch::StartServer()
{
    //m_MsgServer.SetInfo( m_ServerInfo.nport(), m_ServerInfo.nmaxconnection() );

    if( m_MsgServer.Start() == false )
    {
        return false;
    }

    return true;
}