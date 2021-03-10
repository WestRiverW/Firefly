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

#include <common/Bridge.h>
#include <base/LaunchBase.h>
#include "BridgeHook.h"

class Launch : public LaunchBase
{
    friend class BridgeHook;

public:
    Launch();
    virtual ~Launch();

public:
    virtual bool Start();

protected:
    virtual bool Init();
    virtual bool StartServer();

protected:
    BridgeHook      m_BridgeHook;
};

#endif