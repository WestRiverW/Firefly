/*
*   BridgeHook.h
*
*   
*
*   Created on: 2018-11-13
*   Author:
*   All rights reserved.
*/
#ifndef __BridgeBase_H__
#define __BridgeBase_H__

#include <utils/Timer.h>
#include <lua/LuaConfig.h>
#include <common/Bridge.h>
#include <common/BaseCore.h>
#include "../share/ContactCenter.h"
#include <common.pb.h>

using namespace Firefly;

class BridgeBase : public IBridgeHook
{
public:
    BridgeBase();
    virtual ~BridgeBase();

public:
    virtual bool OnBridgeStart( FFObject *pObject );
    virtual bool OnBridgeStop( FFObject *pObject );

public:
    virtual bool OnEventControl( unsigned short wIdentifier, void *pData, unsigned int nDataSize );
    virtual bool OnBridgeData( unsigned short wRequestID, void *pData, unsigned int nDataSize );

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
    virtual bool OnServerRead( ServerItem *pItem, MsgHead *pMsgHead, void *pData, unsigned int nDataSize);
	
protected:
    virtual bool SetConfig(pb::ServerInfo* pServerInfo);

protected:
    IMsgServer      	*m_pIMsgServer;
    IMsgClient      	*m_pIMsgClient;
	Bridge* m_pBridge;
	Timer* m_pTimer;
    LuaConfig           *m_pLuaConfig;
    ContactCenter       *m_pContactCenter;
    pb::ServerInfo      *m_pServerInfo;

    std::string         m_strCenterIP;
    unsigned short      m_usCenterPort;
};

#endif