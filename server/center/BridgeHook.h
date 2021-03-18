/*
*   BridgeHook.h
*
*   
*
*   Created on: 2018-03-18
*   Author:
*   All rights reserved.
*/
#ifndef __BridgeHook_H__
#define __BridgeHook_H__

#include <map>
#include <utils/Timer.h>
#include <common/BaseCore.h>
#include <base/BridgeBase.h>
#include <share/ServerMgr.h>

class BridgeHook : public BridgeBase
{
    friend class Launch;

public:
    BridgeHook();
    virtual ~BridgeHook();

    virtual bool OnServerBind( ServerItem *pItem );
    virtual bool OnServerShut( ServerItem *pItem );
    virtual bool OnServerRead( ServerItem *pItem, MsgHead *pMsgHead, void *pData, unsigned int nDataSize);

protected:
    //ServerMgr               m_ServerMgr;
};

#endif