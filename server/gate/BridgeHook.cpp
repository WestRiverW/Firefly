#include <memory.h>
#include <assert.h>
#include <arpa/inet.h>
#include <glog/logging.h>
#include <net/MsgAssist.h>
#include <utils/Utility.h>
#include <lua/LuaConfig.h>
#include <net/MsgServer.h>
#include <share/ServerCmd.h>
#include <share/CommonDefine.h>
#include "Launch.h"
#include "BridgeHook.h"

BridgeHook::BridgeHook()
    : m_bCfgIsUpdated( false )
{
}

BridgeHook::~BridgeHook()
{
}

//
bool BridgeHook::OnEventControl( unsigned short wIdentifier, void *pData, unsigned int nDataSize )
{
    return true;
}

//
bool BridgeHook::OnTimer(unsigned int unTimerID, unsigned int unMsgID)
{
	if (BridgeBase::OnTimer(unTimerID, unMsgID))
	{
		return true;
	}
	else
	{
		switch (unMsgID)
		{
		default:
			break;
		}
	}
    
    return true;
}

//
bool BridgeHook::OnDataBase( unsigned short wRequestID, MsgHead *pMsgHead, void *pData, unsigned int nDataSize )
{
    switch( wRequestID )
    {
        default:
            break;
    }

    return true;
}

//
bool BridgeHook::OnClientLink( unsigned int nServerID, int nErrorCode )
{
	LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " clientline serverid=" << nServerID << "  errorcode=" << nErrorCode;
    if (BridgeBase::OnClientLink(nServerID,nErrorCode))
    {
        return true;
    }
    if( nErrorCode == 0 )
    {
        int nServerType = nServerID / 100000;

        if( SERVE_TYPE_HALL == nServerType || SERVE_TYPE_GAME == nServerType )
        {
            //m_pIMsgClient->SendData( nServerID, CMD_CORE_BASE, SUB_CORE_HEART );
        }
        else if( SERVE_TYPE_CENTER == nServerID )
        {
			//m_pGateLogic->PullCfgInfo( SERVE_TYPE_GATE );
            // if( !m_bCfgIsUpdated )
            // {
                // m_pGateLogic->PullCfgInfo( SERVE_TYPE_GATE );
            // }
            // else
            // {
                // m_pGateLogic->RegisterServer( m_ServerInfo );
            // }
        }
    }
    else
    {
        m_pTimer->SetTimer( CONNECT_TIMER, 3, 1, nServerID );
    }

    return true;
}

//
bool BridgeHook::OnClientShut( unsigned int nServerID, char cbShutReason )
{
    LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " gatesocketshut";
    m_pTimer->SetTimer( CONNECT_TIMER, 3, 1, nServerID );
    return true;
}

//
bool BridgeHook::OnClientRead(unsigned int nServerID,  MsgHead *pMsgHead, void *pData, unsigned int nDataSize )
{
    LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " gatesocketread " << pMsgHead->wMainCmdID << "  " << pMsgHead->wSubCmdID;


    return true;
}

bool BridgeHook::OnClientCenterMessage( unsigned int nServerID, MsgHead *pMsgHead, void *pData, unsigned int nDataSize )
{
    return true;
}

bool BridgeHook::OnClientPullCfgResp( unsigned int nServerID, MsgHead *pMsgHead, void *pData, unsigned int nDataSize )
{

    return true;
}

bool BridgeHook::OnClientCenterRegResp( unsigned int nServerID, MsgHead *pMsgHead, void *pData, unsigned int nDataSize )
{
    //m_pGateLogic->PullServerInfo(SERVE_TYPE_GAME);
    //m_pGateLogic->PullServerInfo(SERVE_TYPE_HALL);
    return true;
}

bool BridgeHook::OnClientHallMessage( unsigned int nServerID, MsgHead *pMsgHead, void *pData, unsigned int nDataSize )
{

    return true;
}


bool BridgeHook::OnClientGameMessage( unsigned int nServerID, MsgHead *pMsgHead, void *pData, unsigned int nDataSize )
{
    LOG( INFO ) << strThreadLogFlag << __FUNCTION__  << " -------------CMD_GAME_BASE----------1--";

    return true;
}
///////////////////////////////////////////////////////////////////////////////////////////


bool BridgeHook::OnServerReady()
{
    if( m_bCfgIsUpdated )
    {
        //m_pGateLogic->RegisterServer( m_ServerInfo );
    }

    return true;
}
//
bool BridgeHook::OnServerBind( unsigned int dwSocketID, unsigned int dwClientAddr )
{
    LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << "gatebind " << dwSocketID;
    return true;
}

//
bool BridgeHook::OnServerShut( ServerItem *pItem )
{
    return true;
}

//
bool BridgeHook::OnServerRead(ServerItem *pItem, MsgHead *pMsgHead, void *pData, unsigned int nDataSize )
{
    LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << "gateread id:" << pMsgHead->wMainCmdID << ",sub id:" << pMsgHead->wSubCmdID << ",nDataSize:" << nDataSize << " index:" << pItem->GetSocketID();

    switch( pMsgHead->wMainCmdID )
    {
    }

    return true;
}

//////////////////////////////////////////////////////////////////////////
