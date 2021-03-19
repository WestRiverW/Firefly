/*
*   LaunchBase.h
*
*   
*
*   Created on: 2018-11-07
*   Author:
*   All rights reserved.
*/
#ifndef __LaunchBase_H__
#define __LaunchBase_H__

#include <mutex>
#include <condition_variable>
#include <net/MsgServer.h>
#include <net/MsgClient.h>
#include <common/Bridge.h>
#include <utils/Timer.h>
#include <lua/LuaConfig.h>
#include <common/BaseDefine.h>
#include "../share/ContactCenter.h"
#include <common.pb.h>

using namespace Firefly;

class LaunchBase
{
public:
    LaunchBase();
    virtual ~LaunchBase();

public:
    virtual bool Start();
    virtual bool Stop();

protected:
    virtual bool Init();
    virtual bool StartCore();
    virtual bool StartServer();

protected:
    bool LoadConfig();
    void SetServerInfo(pb::ServerInfo& cfgInfo);

protected:
    MsgServer           m_MsgServer;
    MsgClient       	m_MsgClient;
    Timer             	m_Timer;
    Bridge          	m_Bridge;
    LuaConfig           m_LuaConfig;
    ContactCenter       m_ContactCenter;

    pb::ServerInfo          m_ServerInfo;
    std::mutex              m_mutMain;
    std::condition_variable m_condMain;

    std::string         m_strCenterIP;
    unsigned short      m_usCenterPort;
};

#endif