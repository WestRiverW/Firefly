#include <memory.h>
#include <assert.h>
#include <glog/logging.h>
#include <net/MsgAssist.h>
#include <utils/Utility.h>
#include <lua/LuaConfig.h>
#include <net/MsgServer.h>
#include <share/ServerCmd.h>
#include <share/CommonDefine.h>
#include <game.pb.h>
#include <common.pb.h>
#include "Launch.h"
#include "BridgeHook.h"

BridgeHook::BridgeHook()
    : m_bCfgIsUpdated( false )
{
    m_pIDBEngine = NULL;
}

BridgeHook::~BridgeHook()
{
}

bool BridgeHook::OnTimer( unsigned int unTimerID, unsigned int unMsgID )
{
    switch( unTimerID )
    {
        case CONNECT_TIMER:
        {
            LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " timer task id:" << "CONNECT_TIMER";
            LuaConfig cfg;
            cfg.init( "ServerConfig.lua" );
            std::string centerport = cfg.getConfigByFun( "GetServerConfig", "centerport" );
            std::string centerhost = cfg.getConfigByFun( "GetServerConfig", "centerip" );
            //m_pContactCenter->doConnect( SERVE_TYPE_CENTER, centerhost.c_str(), atoi( centerport.c_str() ) );
        }
        break;

        default:
            break;
    }

    return true;
}

//数据库事
bool BridgeHook::OnDataBase( unsigned short wRequestID, MsgHead *pMsgHead, void *pData, unsigned int wDataSize )
{
    LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " wRequestID:" << wRequestID ;
    //protocol::MsgBody msg;

    //if( msg.ParseFromArray( ( char * )pData, wDataSize ) )
    //{
    //    return true;
    //}

    return true;
}

//连接事件
bool BridgeHook::OnClientLink( unsigned int nServerID, int nErrorCode )
{
    LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " nServerID:" << nServerID << ",nErrorCode:" << nErrorCode;

    if( nErrorCode == 0 )
    {
        if( !m_bCfgIsUpdated )
        {
            m_pContactCenter->PullCfgInfo( SERVE_TYPE_DB);
        }
        else
        {
            //m_pContactCenter->RegisterServer( m_ServerInfo );
        }
    }
    else
    {
        m_pTimer->SetTimer( CONNECT_TIMER, 3, 1, 0 );
    }

    return true;
}

//关闭事件
bool BridgeHook::OnClientShut( unsigned int nServerID, char cbShutReason )
{
    m_pTimer->SetTimer( CONNECT_TIMER, 3, 1, 0 );
    return true;
}

//读取事件
bool BridgeHook::OnClientRead( unsigned int nServerID, MsgHead *pMsgHead, void *pData, unsigned int wDataSize )
{
    LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " main cmd id:" << pMsgHead->wMainCmdID << ",sub Cmd id:" << pMsgHead->wSubCmdID << ",wDataSize:" << wDataSize;

    return true;
}


bool BridgeHook::OnServerReady()
{
    if( m_bCfgIsUpdated )
    {
        //m_pContactCenter->RegisterServer( m_ServerInfo );
    }

    return true;
}
//读取事件
bool BridgeHook::OnServerBind(ServerItem* pServerItem)
{
    return true;
}

//读取事件
bool BridgeHook::OnServerShut( ServerItem *pItem )
{
    //清除信息
    LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << "dbshut " << pItem->GetIndex();

    if( pItem )
    {
        pItem->ResetData();
    }

    return true;
}

//读取事件
bool BridgeHook::OnServerRead(ServerItem *pItem, MsgHead *pMsgHead, void *pData, unsigned int wDataSize )
{
    LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << "dbread main id:" << pMsgHead->wMainCmdID << ",sub id:" << pMsgHead->wSubCmdID << ",wDataSize:" << wDataSize;
    
	//标识是哪个大厅，哪个游戏过来的消息
	pMsgHead->wHallGateIndex = pItem->GetSocketID();
	//m_pIDBEngine->PostDBRequest( pMsgHead->wSubCmdID, pMsgHead, pData, wDataSize );
	
	return true;
}
