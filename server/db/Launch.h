/*
*   Launch.h
*
*   
*
*   Created on: 2018-11-07
*   Author:
*   All rights reserved.
*/
#ifndef __Launch_H__
#define __Launch_H__

#include <utils/Timer.h>
#include <data/DBEngine.h>
#include <net/MsgServer.h>
#include <net/MsgClient.h>
#include <common/Bridge.h>
#include <share/ContactCenter.h>
#include <base/LaunchBase.h>
#include "DBEngineHook.h"
#include "BridgeHook.h"

class Launch : public LaunchBase
{
    friend class BridgeHook;
    friend class DBEngineHook;

public:
    Launch();
    virtual ~Launch();

public:
    bool Start();

protected:
    bool Init();
    bool StartCore();
    bool StartServer();

protected:
    DBEngine            m_DBEngine;
    BridgeHook          m_BridgeHook;
    DBEngineHook        m_DBEngineHook;
};

#endif