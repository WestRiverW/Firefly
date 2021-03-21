#include <memory.h>
#include <assert.h>
#include <glog/logging.h>
#include <net/MsgAssist.h>
#include <net/MsgServer.h>
#include <utils/Utility.h>
#include "../share/CommonDefine.h"
#include "BridgeBase.h"

BridgeBase::BridgeBase()
    :m_pIMsgServer(nullptr),
    m_pIMsgClient(nullptr),
	m_pBridge(nullptr),
    m_pTimer(nullptr),
    m_pLuaConfig(nullptr),
    m_pContactCenter(nullptr),
    m_pServerInfo(nullptr),
    m_strCenterIP(""),
    m_usCenterPort(0),
    m_eServerType(0)
{
}

BridgeBase::~BridgeBase()
{
}

bool BridgeBase::OnBridgeStart( FFObject *pObject )
{
	m_strCenterIP = m_pLuaConfig->getConfigByFun("GetServerConfig", "centerip");
    m_usCenterPort = atoi(m_pLuaConfig->getConfigByFun("GetServerConfig", "centerport").c_str());

    return true;
}

bool BridgeBase::OnBridgeStop( FFObject *pObject )
{
    return true;
}

bool BridgeBase::OnEventControl( unsigned short wIdentifier, void *pData, unsigned int nDataSize )
{
    return true;
}

bool BridgeBase::OnBridgeData( unsigned short wRequestID, void *pData, unsigned int nDataSize )
{
    return true;
}

bool BridgeBase::OnTimer( unsigned int unTimerID, unsigned int unMsgID )
{
	switch (unTimerID)
	{
	case CONNECT_TIMER:
	{
        switch (unMsgID)
        {
        case SERVE_TYPE_CENTER:
        {
            m_pIMsgClient->Connect(SERVE_TYPE_CENTER, m_strCenterIP.c_str(), m_usCenterPort);
            return true;
        }
        default:
            break;
        }
		break;
	}
	default:
		break;
	}
    return false;
}

bool BridgeBase::OnDataBase( unsigned short wRequestID, MsgHead *pMsgHead, void *pData, unsigned int nDataSize )
{
    return true;
}

bool BridgeBase::OnClientLink( unsigned int nServerID, int nErrorCode )
{
    if (0 == nErrorCode)
    {
        switch (nServerID)
        {
        case SERVE_TYPE_CENTER:
        {
            m_pContactCenter->PullCfgInfo(m_eServerType);
            return true;
        }
        default:
            break;
        }
    }
    return false;
}

bool BridgeBase::OnClientShut( unsigned int nServerID, char cbShutReason )
{
    return true;
}

bool BridgeBase::OnClientRead( unsigned int nServerID, MsgHead *pMsgHead, void *pData, unsigned int nDataSize )
{
    return true;
}

bool BridgeBase::OnServerReady()
{
    return true;
}

bool BridgeBase::OnServerBind( ServerItem *pItem )
{
    return true;
}

bool BridgeBase::OnServerShut( ServerItem *pItem  )
{
    return true;
}

bool BridgeBase::OnServerRead( ServerItem *pItem, MsgHead *pMsgHead, void *pData, unsigned int nDataSize )
{
    return true;
}

bool BridgeBase::SetConfig(pb::ServerInfo* pServerInfo)
{
    m_pServerInfo = pServerInfo;
	return true;
}