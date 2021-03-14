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
#include <lua/LuaConfig.h>
#include <common/Bridge.h>
#include <common/BaseCore.h>
#include <common/UserManager.h>
#include <base/BridgeBase.h>
#include <share/ContactCenter.h>
#include <game.pb.h>
#include "Table.h"

typedef std::map<int,Table *>					CTableArray;

class Launch;

class BridgeHook : public BridgeBase, public IDataTransition
{
    friend class Launch;

public:
    BridgeHook();
    virtual ~BridgeHook();

public:
    virtual bool OnEventControl( unsigned short wIdentifier, void *pData, unsigned int wDataSize );

public:
    virtual bool OnTimer( unsigned int unTimerID, unsigned int unMsgID );

public:
    virtual bool OnClientLink( unsigned int nServerID, int nErrorCode );
    virtual bool OnClientShut( unsigned int nServerID, char cbShutReason );
    virtual bool OnClientRead( unsigned int nServerID, MsgHead *pMsgHead, void *pData, unsigned int wDataSize );

public:
    virtual bool OnServerReady();
    virtual bool OnServerReadyDelay();
    virtual bool OnServerShut( ServerItem *pItem );
    virtual bool OnServerRead( ServerItem *pItem, MsgHead *pMsgHead, void *pData, unsigned int wDataSize );

protected:
    virtual void OnLoginResp( void *pData, unsigned int wDataSize );
public:
    bool SendData( MsgHead *pMsgHead, void *pData, int wDataSize );

protected:
    bool OnSubServerHallMsg( MsgHead *pMsgHead, void *pData, unsigned int wDataSize );
    bool OnSubServerGameMsg( MsgHead *pMsgHead, void *pData, unsigned int wDataSize );

public:
    bool OnClientCenterRegResp( void *pData, unsigned int wDataSize );
    void SetLoadSoNumber( int nLoadNumber );
	
protected:
	Table * ActiveTable(int nTableID);
	Table * GetTable(int nTableID);
	bool FreeTable(int nTableID);

protected:
    int m_nLoadSoNumber;
    std::vector<void*>     m_vecModuleHandle;
    Launch* m_pLaunchBase;
    bool                    m_bCfgIsUpdated;

    std::map<int, ITable*> m_mapGameType2Table;

protected:
    std::map<int, int>      m_mapUserGate;
    CTableArray				m_mapTableFree;
    CTableArray				m_mapTableActive;
    CTableArray				m_mapTableStore;
    UserManager      		m_UserManager;
};

#endif