/*
*   BridgeHook.h
*
*   
*
*   Created on: 2018-11-13
*   Author:
*   All rights reserved.
*/
#ifndef __BridgeHook_H__
#define __BridgeHook_H__

#include <utils/Timer.h>
#include <common/Bridge.h>
#include <common/BaseCore.h>
#include <share/ContactCenter.h>
#include <base/BridgeBase.h>

class Launch;

class BridgeHook : public BridgeBase
{
    friend class Launch;

public:
    BridgeHook();
    virtual ~BridgeHook();

public:
    virtual bool OnTimer( unsigned int unTimerID, unsigned int unMsgID );
    virtual bool OnDataBase( unsigned short wRequestID, MsgHead *pMsgHead, void *pData, unsigned int nDataSize );

public:
    virtual bool OnClientLink( unsigned int nServerID, int nErrorCode );
    virtual bool OnClientShut( unsigned int nServerID, char cbShutReason );
    virtual bool OnClientRead( unsigned int nServerID, MsgHead *pMsgHead, void *pData, unsigned int nDataSize );

public:
    virtual bool OnServerReady();
    virtual bool OnServerBind( ServerItem *pItem );
    virtual bool OnServerShut( ServerItem *pItem );
    virtual bool OnServerRead( ServerItem *pItem, MsgHead *pMsgHead, void *pData, unsigned int nDataSize );

protected:
    IDBEngine               *m_pIDBEngine;
    ContactCenter           *m_pContactCenter;
    Launch                  *m_pLaunchBase;
    bool                    m_bCfgIsUpdated;
};

#endif