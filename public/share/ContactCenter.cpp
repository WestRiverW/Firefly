#include <glog/logging.h>
#include <net/MsgAssist.h>
#include <game.pb.h>
#include <common.pb.h>
#include "ServerCmd.h"
#include "CommonDefine.h"
#include "ContactCenter.h"

void ContactCenter::SetMsgClient( IMsgClient *socketService )
{
    m_pIMsgClient = socketService;
}

void ContactCenter::Connect(unsigned int dwServerID, const char* szServerIP, unsigned short wPort)
{
    m_pIMsgClient->Connect(dwServerID, szServerIP, wPort);
}

void ContactCenter::Register(pb::ServerInfo &cfgInfo)
{
    /*LOG( INFO ) << strThreadLogFlag << __FUNCTION__;
    protocol::SvrRegInfo regInfo;
    regInfo.set_nmaintype( SERVE_TYPE_GAME );
    regInfo.set_nsubtype( cfgInfo.nport() );
    regInfo.set_szip( "" );
    regInfo.set_nport( cfgInfo.nport() );
    protocol::MsgBody info;
    regInfo.SerializeToString( info.mutable_body() );
    if(pSubGameList)
    {
        pSubGameList->SerializeToString( info.mutable_common() );
    }

    char cbDataBuffer[SOCKET_BUFFER_LEN] = { 0 };
    int nBufferSize = 0;
    MsgAssist::encode( &info, cbDataBuffer, nBufferSize );

    MsgHead msgHeadInfo;
    msgHeadInfo.wMainCmdID = CMD_CENTER_BASE;
    msgHeadInfo.wSubCmdID = SUB_CENTER_SERVER_REG;
    msgHeadInfo.nBodyLen = nBufferSize;

    m_pIMsgClient->SendData( SERVE_TYPE_CENTER, &msgHeadInfo, cbDataBuffer, nBufferSize );*/
    }

void ContactCenter::PullCfgInfo(int nMainType)
{
    /*LOG( INFO ) << strThreadLogFlag << __FUNCTION__;
    protocol::SvrCfgInfoReq serverInfoReq;
    serverInfoReq.set_nmaintype( nMainType );
    protocol::MsgBody info;
    serverInfoReq.SerializeToString( info.mutable_body() );
    char cbDataBuffer[SOCKET_BUFFER_LEN] = { 0 };
    int nBufferSize = 0;
    MsgAssist::encode( &info, cbDataBuffer, nBufferSize );
	
	MsgHead msgHeadInfo;
	msgHeadInfo.wMainCmdID = CMD_CENTER_BASE;
	msgHeadInfo.wSubCmdID = SUB_CENTER_PULL_SERVER_CFG;
	msgHeadInfo.nBodyLen = nBufferSize;
	
	LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << "  len:" << nBufferSize;
	
    m_pIMsgClient->SendData( SERVE_TYPE_CENTER, &msgHeadInfo, cbDataBuffer, nBufferSize );*/
}

/*void ContactCenter::PullServerInfo(int nMainType )
{
    LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " PullServerInfo ";
    protocol::SvrCfgInfoReq serverInfoReq;
    serverInfoReq.set_nmaintype( nMainType );
    protocol::MsgBody info;
    serverInfoReq.SerializeToString( info.mutable_body() );
    char cbDataBuffer[SOCKET_BUFFER_LEN] = { 0 };
    int nBufferSize = 0;
    MsgAssist::encode( &info, cbDataBuffer, nBufferSize );
	
	MsgHead msgHeadInfo;
	msgHeadInfo.wMainCmdID = CMD_CENTER_BASE;
	msgHeadInfo.wSubCmdID = SUB_CENTER_PULL_SERVER_REG_INFO;
	msgHeadInfo.nBodyLen = nBufferSize;
	
    m_pIMsgClient->SendData( SERVE_TYPE_CENTER, &msgHeadInfo, cbDataBuffer, nBufferSize );
}*/

