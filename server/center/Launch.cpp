#include <stdio.h>
#include <assert.h>
#include <glog/logging.h>
#include <lua/LuaConfig.h>
#include "Launch.h"

Launch::Launch()
{
}

Launch::~Launch()
{
    
}

bool Launch::Start()
{
    if (LaunchBase::Start() == false)
    {
        Stop();
        return false;
    }

    if(StartServer() == false )
    {
        Stop();
        return false;
    }
    return true;
}

bool Launch::Init()
{
    LaunchBase::Init();

    if( m_Bridge.SetBridgeHook( &m_BridgeHook ) == false ) return false;
    m_BridgeHook.m_pIMsgServer = dynamic_cast<IMsgServer *>( &m_MsgServer );
    m_BridgeHook.m_pIMsgClient = dynamic_cast<IMsgClient*>(&m_MsgClient);
    m_BridgeHook.m_pBridge = &m_Bridge;
    m_BridgeHook.m_pTimer = &m_Timer;
    m_BridgeHook.m_pLuaConfig = &m_LuaConfig;
    m_BridgeHook.m_pContactCenter = &m_ContactCenter;
    m_BridgeHook.m_pServerInfo = &m_ServerInfo;
    m_BridgeHook.m_pServerMgr = &m_ServerMgr;
    
    m_ServerMgr.ParseConfig(&m_LuaConfig);

    return true;
}

bool Launch::StartServer()
{
    std::string centerport = m_LuaConfig.getConfigByFun("GetServerConfig", "centerport");
    std::string maxconnection = m_LuaConfig.getConfigByFun("GetServerConfig", "maxconnection");
    m_MsgServer.SetInfo(atoi(centerport.c_str()), atoi(maxconnection.c_str()));
    if (m_MsgServer.Start() == false)
    {
        return false;
    }
    return true;
}