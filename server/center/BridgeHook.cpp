#include <memory.h>
#include <assert.h>
#include <glog/logging.h>
#include <utils/Utility.h>
#include <share/CommonDefine.h>
#include <share/ServerCmd.h>
#include <game.pb.h>
#include <common.pb.h>
#include "BridgeHook.h"

BridgeHook::BridgeHook()
{
}

BridgeHook::~BridgeHook()
{
}

bool BridgeHook::OnServerBind(ServerItem* pServerItem)
{
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

    return true;
}
