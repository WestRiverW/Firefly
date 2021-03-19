#include "Launch.h"
#include <stdio.h>
#include <assert.h>
#include <glog/logging.h>
#include <share/CommonDefine.h>

std::mutex              g_mutMain;
std::condition_variable g_condMain;

Launch::Launch()
{
}

Launch::~Launch()
{
    Stop();
}

bool Launch::Start()
{
    if (LaunchBase::Start() == false)
    {
        Stop();
        return false;
    }
    //Connect center server.
    m_Timer.SetTimer( CONNECT_TIMER, 1, 1, SERVE_TYPE_CENTER );
    std::unique_lock <std::mutex> lck(g_mutMain);
    g_condMain.wait( lck );

    if( StartServer() == false )
    {
        Stop();
        return false;
    }

    return true;
}

bool Launch::Init()
{
    LaunchBase::Init();

    if (m_Bridge.SetBridgeHook(&m_BridgeHook) == false) return false;
	m_BridgeHook.m_pIMsgServer = dynamic_cast<IMsgServer*>(&m_MsgServer);
	m_BridgeHook.m_pIMsgClient = dynamic_cast<IMsgClient*>(&m_MsgClient);
	m_BridgeHook.m_pBridge = &m_Bridge;
	m_BridgeHook.m_pTimer = &m_Timer;
	m_BridgeHook.m_pLuaConfig = &m_LuaConfig;
	m_BridgeHook.m_pContactCenter = &m_ContactCenter;
	m_BridgeHook.m_pServerInfo = &m_ServerInfo;
    return true;
}

bool Launch::StartServer()
{
    m_MsgServer.SetInfo( m_ServerInfo.port(), m_ServerInfo.maxconnect() );
    if( m_MsgServer.Start() == false )
    {
        return false;
    }

    return true;
}