#include "BridgeHook.h"
#include <memory.h>
#include <assert.h>
#include <glog/logging.h>
#include <share/ServerCmd.h>
#include <share/CommonDefine.h>
#include "Launch.h"

BridgeHook::BridgeHook()
    : m_bCfgIsUpdated( false )
{
    
}

BridgeHook::~BridgeHook()
{
}

bool BridgeHook::OnTimer( unsigned int unTimerID, unsigned int unMsgID )
{
    LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " task id:" << unTimerID << ",bindParam:" << unMsgID;

    return true;
}

bool BridgeHook::OnClientLink( unsigned int nServerID, int nErrorCode )
{
    LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << "hallsocket link nServerID:" << nServerID << ",nErrorCode:" << nErrorCode;

    if( nErrorCode == 0 )
    {
        switch( nServerID )
        {
            case SERVE_TYPE_CENTER:
            {
                if( !m_bCfgIsUpdated )
                {
                    //->PullCfgInfo( SERVE_TYPE_HALL);
                }
                else
                {
                   // m_pContactCenter->RegisterServer( m_ServerInfo );
                }

                break;
            }

            case SERVE_TYPE_DB:
            {
				MsgHead msgHeadInfo;
				msgHeadInfo.wMainCmdID = CMD_CORE_BASE;
				msgHeadInfo.wSubCmdID = SUB_CORE_HEART;
                m_pIMsgClient->SendData( nServerID, &msgHeadInfo );
                break;
            }
        }
    }
    else
    {
        m_pTimer->SetTimer( CONNECT_TIMER, 3, 1, nServerID );
    }

    return true;
}

bool BridgeHook::OnClientShut( unsigned int nServerID, char cbShutReason )
{
    LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " hallsocket shut " << nServerID ;
    m_pTimer->SetTimer( CONNECT_TIMER, 3, 1, nServerID );
    return true;
}

bool BridgeHook::OnClientRead( unsigned int nServerID, MsgHead *pMsgHead, void *pData, unsigned int wDataSize )
{
    return true;
}

bool BridgeHook::OnMainTcpSocketCenter( MsgHead *pMsgHead, void *pData, unsigned int wDataSize )
{
    return true;
}


bool BridgeHook::OnMainTcpSocketHall( MsgHead *pMsgHead, void *pData, unsigned int wDataSize )
{
    return true;
}

bool BridgeHook::OnServerReady()
{
    LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " m_bCfgIsUpdated:" << m_bCfgIsUpdated;

    if( m_bCfgIsUpdated )
    {
        //m_pContactCenter->RegisterServer( m_ServerInfo );
    }

    return true;
}

bool BridgeHook::OnServerBind( ServerItem *pItem )
{
    return true;
}

bool BridgeHook::OnServerShut( ServerItem *pItem )
{
    LOG( INFO ) << strThreadLogFlag << __FUNCTION__;

    if( pItem )
    {
        pItem->ResetData();
    }

    return true;
}

bool BridgeHook::OnServerRead(ServerItem *pItem, MsgHead *pMsgHead, void *pData, unsigned int wDataSize )
{
    return true;
}
