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

#include <base/LaunchBase.h>
#include <share/ContactCenter.h>
#include <common.pb.h>
#include "BridgeHook.h"

class Launch : public LaunchBase
{
    friend class BridgeHook;

public:
    Launch();
    virtual ~Launch();

public:
    bool Start();

protected:
    bool Init();
    bool StartServer();

protected:
    BridgeHook      m_BridgeHook;
};

#endif