#include <memory.h>
#include <assert.h>
#include <glog/logging.h>
#include <utils/Utility.h>
#include <share/CommonDefine.h>
#include <share/ServerCmd.h>
#include <game.pb.h>
#include <common.pb.h>
#include "BridgeHook.h"
#include "share/MsgUtils.h"

BridgeHook::BridgeHook()
{
}

BridgeHook::~BridgeHook()
{
}

bool BridgeHook::OnServerBind(ServerItem* pServerItem)
{
    LOG(INFO) << "centerbind";
    return true;
}

bool BridgeHook::OnServerShut( ServerItem *pItem  )
{
    if( pItem )
    {
        //m_ServerMgr.Erase( pItem );
        pItem->ResetData();
    }

    return true;
}

bool BridgeHook::OnServerRead( ServerItem *pItem, MsgHead *pMsgHead, void *pData, unsigned int nDataSize )
{
	LOG(INFO) << "centerread" <<" mainid="<<pMsgHead->wMainCmdID<<" subid="<<pMsgHead->wSubCmdID;
    switch (pMsgHead->wMainCmdID)
    {
    case CMD_CENTER_BASE:
    {
        return OnSubMsgCenter(pItem,pMsgHead, pData, nDataSize);
    }
    default:
        break;
    }
    return true;
}

bool BridgeHook::OnSubMsgCenter(ServerItem* pItem, MsgHead* pMsgHead, void* pData, unsigned int nDataSize)
{
    switch (pMsgHead->wSubCmdID)
    {
    case SUB_PULL_SERVER_CFG:
    {
        pb::MsgBody msgBody;
        if (msgBody.ParseFromArray(pData, nDataSize))
        {
            pb::ServerInfo SvrInfo;
            if (SvrInfo.ParseFromArray(msgBody.common().c_str(),msgBody.common().size()))
            {
                int nMainType = SvrInfo.maintype();
                pb::ServerInfo *pSvrInfo = m_pServerMgr->GetConfig(nMainType);
                pb::MsgBody msgBody;
                pSvrInfo->SerializeToString(msgBody.mutable_common());
                char cbBodyBuffer[SOCKET_BUFFER_LEN] = { 0 };
                int nBodySize = 0;
                MsgUtils::encode(&msgBody, cbBodyBuffer, nBodySize);
                //head
                MsgHead msgHead;
                memset(&msgHead, 0, sizeof(msgHead));
                msgHead.wMainCmdID = CMD_CENTER_BASE;
                msgHead.wSubCmdID = SUB_PULL_SERVER_CFG_RESP;
                msgHead.nBodyLen = nBodySize;
                msgHead.nSocketID = pItem->GetSocketID();

                m_pIMsgServer->SendData(&msgHead, cbBodyBuffer, nBodySize);
            }
        }


		////body
		//pb::MsgBody msgBody;
		//pb::ServerInfo SvrInfo;
		//SvrInfo.set_maintype(nMainType);
		//SvrInfo.SerializeToString(msgBody.mutable_common());
		//char cbDataBuffer[SOCKET_BUFFER_LEN] = { 0 };
		//int nBufferSize = 0;
		//MsgUtils::encode(&msgBody, cbDataBuffer, nBufferSize);
		////head
		//MsgHead msgHead;
		//memset(&msgHead, 0, sizeof(msgHead));
		//msgHead.wMainCmdID = CMD_CENTER_BASE;
		//msgHead.wSubCmdID = SUB_PULL_SERVER_CFG;
		//msgHead.nBodyLen = nBufferSize;

  //      m_pIMsgServer->SendData(&msgHead, cbDataBuffer, nBufferSize);
        break;
    }

    default:
        break;
    }
    return true;
}
