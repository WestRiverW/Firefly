/*
*   BridgeHook.h
*
*   
*
*   Created on: 2019-03-11
*   Author:
*   All rights reserved.
*/
#ifndef __BridgeHook_H__
#define __BridgeHook_H__

#include <lua/LuaConfig.h>
#include <utils/Timer.h>
#include <net/MsgClient.h>
#include <common/Bridge.h>
#include <common/BaseCore.h>
#include <base/BridgeBase.h>
#include <share/ContactCenter.h>

class Launch;

//
class BridgeHook : public BridgeBase
{
    friend class Launch;

public:
    BridgeHook();
    virtual ~BridgeHook();
	
public:
    virtual bool OnEventControl( unsigned short wIdentifier, void *pData, unsigned int nDataSize );
	
public:
    virtual bool OnTimer( unsigned int unTimerID, unsigned int unMsgID );
    virtual bool OnDataBase( unsigned short wRequestID, MsgHead *pMsgHead, void *pData, unsigned int nDataSize );
	
public:
    virtual bool OnClientLink( unsigned int nServerID, int nErrorCode );
    virtual bool OnClientShut( unsigned int nServerID, char cbShutReason );
    virtual bool OnClientRead( unsigned int nServerID, MsgHead *pMsgHead, void *pData, unsigned int nDataSize );

protected:
    virtual bool OnClientCenterMessage( unsigned int nServerID, MsgHead *pMsgHead, void *pData, unsigned int nDataSize );
    virtual bool OnClientHallMessage( unsigned int nServerID, MsgHead *pMsgHead, void *pData, unsigned int nDataSize );
    virtual bool OnClientGameMessage( unsigned int nServerID, MsgHead *pMsgHead, void *pData, unsigned int nDataSize );

protected:
    virtual bool OnClientPullCfgResp( unsigned int nServerID, MsgHead *pMsgHead, void *pData, unsigned int nDataSize );
    virtual bool OnClientCenterRegResp( unsigned int nServerID, MsgHead *pMsgHead, void *pData, unsigned int nDataSize );

public:
    virtual bool OnServerReady();
    virtual bool OnServerBind( unsigned int dwSocketID, unsigned int dwClientAddr );
    virtual bool OnServerShut( ServerItem *pItem );
    virtual bool OnServerRead( ServerItem *pItem, MsgHead *pMsgHead, void *pData, unsigned int nDataSize );

protected:
    std::map<int, ServerItem*>   m_mapSeq2NetItem;
    std::map<int, ServerItem*>   m_mapNetItem2Seq;

    ContactCenter* m_pGateLogic;
    Launch* m_pLaunchBase;
    bool                    m_bCfgIsUpdated;
    LuaConfig              m_cfg;
};

#endif