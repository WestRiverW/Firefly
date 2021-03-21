#include <stdio.h>
#include <assert.h>
#include <glog/logging.h>
#include <lua/LuaConfig.h>
#include "LaunchBase.h"

LaunchBase::LaunchBase()
{

}

LaunchBase::~LaunchBase()
{
    Stop();
}

bool LaunchBase::Start()
{
    if( Init() == false )
    {
        Stop();
        return false;
    }

    if( StartCore() == false )
    {
        Stop();
        return false;
    }

    return true;
}

bool LaunchBase::Stop()
{
    m_Timer.Stop();
    m_Bridge.Stop();
    m_MsgServer.Stop();
    m_MsgClient.Stop();
    return true;
}

bool LaunchBase::Init()
{
    if( m_MsgServer.SetNetServer( &m_Bridge ) == false ) return false;
    if( m_MsgClient.SetClientEvent( &m_Bridge ) == false ) return false;
    if( m_Bridge.SetServer( &m_MsgServer ) == false ) return false;
    if( m_Timer.SetTimer( &m_Bridge ) == false ) return false;

    //if( m_Bridge.SetBridgeHook( m_pBridgeHook ) == false ) return false;
    //m_pBridgeHook->m_pIMsgServer = dynamic_cast<IMsgServer *>( &m_MsgServer );
    //m_pBridgeHook->m_pTimer = &m_Timer;

    m_ContactCenter.SetMsgClient(dynamic_cast<IMsgClient*>(&m_MsgClient));

    LoadConfig();

    return true;
}

bool LaunchBase::StartCore()
{
    if( m_Timer.Start() == false )
    {
        assert( false );
        return false;
    }

    if( m_Bridge.Start() == false )
    {
        assert( false );
        return false;
    }

    if( m_MsgClient.Start() == false )
    {
        assert( false );
        return false;
    }

    return true;
}

bool LaunchBase::StartServer()
{
    m_MsgServer.SetInfo(m_ServerInfo.port(), m_ServerInfo.maxconnect());
    if (m_MsgServer.Start() == false)
    {
        return false;
    }
    else
    {
        return true;
    }
}

bool LaunchBase::LoadConfig()
{
    m_LuaConfig.init("ServerConfig.lua");
    return true;
}

void LaunchBase::SetServerInfo(pb::ServerInfo& cfgInfo)
{
    m_ServerInfo = cfgInfo;
    std::unique_lock <std::mutex> lck(m_mutMain);
    m_condMain.notify_one();
}