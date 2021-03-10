#include <assert.h>
#include <memory.h>
#include <glog/logging.h>
#include <net/MsgClient.h>
#include <net/MsgServer.h>

#include "Bridge.h"

namespace Firefly
{
    Bridge::Bridge()
    {
    }

    Bridge::~Bridge()
    {
    }

    bool Bridge::Start()
    {
        assert((m_pIMsgServer != NULL) && (m_pIBridgeHook != NULL));
        if ((m_pIMsgServer == NULL) || (m_pIBridgeHook == NULL)) return false;
        if (m_AsynEngine.SetAsynHook(dynamic_cast<IAsynEngineHook*>(this)) == false)
        {
            assert(false);
            return false;
        }

        if (m_AsynEngine.Start() == false)
        {
            assert(false);
            return false;
        }

        return true;
    }

    bool Bridge::Stop()
    {
        m_AsynEngine.Stop();
        return true;
    }

    bool Bridge::SendControl(unsigned short wControlID, void* pData, unsigned int wDataSize)
    {
        std::unique_lock <std::mutex> lck(m_mutCritical);
        tagControl* pControlRequest = (tagControl*)m_cbBuffer;
        pControlRequest->wControlID = wControlID;
        pControlRequest->pData = pData;

        if (wDataSize > 0)
        {
            assert(pData != NULL);
            memcpy(m_cbBuffer + sizeof(tagControl), pData, wDataSize);
        }

        return m_AsynEngine.PostAsynData(MSG_CONTROL, m_cbBuffer, sizeof(tagControl) + wDataSize);
    }

    bool Bridge::SetServer(IMsgServer* pObject)
    {
        assert(pObject);
        m_pIMsgServer = dynamic_cast<IMsgServer*>(pObject);
        return true;
    }

    bool Bridge::SetBridgeHook(IBridgeHook* pObject)
    {
        assert(pObject);
        m_pIBridgeHook = dynamic_cast<IBridgeHook*>(pObject);
        return true;
    }

    bool Bridge::OnClientLink(unsigned int nServerID, int nErrorCode)
    {
        std::unique_lock <std::mutex> lck(m_mutCritical);
        tagClientLink* pLinkEvent = (tagClientLink*)m_cbBuffer;
        pLinkEvent->nServerID = nServerID;
        pLinkEvent->nErrorCode = nErrorCode;
        return m_AsynEngine.PostAsynData(MSG_CLIENT_LINK, m_cbBuffer, sizeof(tagClientLink));
    }

    bool Bridge::OnClientRead(MsgHead* pMsgHead, void* pData, unsigned int wDataSize)
    {
        std::unique_lock <std::mutex> lck(m_mutCritical);
        tagClientRead* pReadEvent = (tagClientRead*)m_cbBuffer;
        pReadEvent->MsgHeadInfo = *pMsgHead;
        pReadEvent->wDataSize = wDataSize;

        if (wDataSize > 0)
        {
            assert(pData != NULL);
            memcpy(m_cbBuffer + sizeof(tagClientRead), pData, wDataSize);
        }

        return m_AsynEngine.PostAsynData(MSG_CLIENT_READ, m_cbBuffer, sizeof(tagClientRead) + wDataSize);
    }

    bool Bridge::OnClientShut(unsigned int nServerID, char cbShutReason)
    {
        std::unique_lock <std::mutex> lck(m_mutCritical);
        tagClientShut* pShutEvent = (tagClientShut*)m_cbBuffer;
        pShutEvent->nServerID = nServerID;
        pShutEvent->cbShutReason = cbShutReason;
        return m_AsynEngine.PostAsynData(MSG_CLIENT_SHUT, m_cbBuffer, sizeof(tagClientShut));
    }

    bool Bridge::OnServerReady()
    {
        std::unique_lock <std::mutex> lck(m_mutCritical);
        tagServerReady* pReadyEvent = (tagServerReady*)m_cbBuffer;
        pReadyEvent->dwSocketID = 0;
        return m_AsynEngine.PostAsynData(MSG_SERVER_READY, m_cbBuffer, sizeof(tagServerReady));
    }

    bool Bridge::OnServerBind(ServerItem *pItem)
    {
        std::unique_lock <std::mutex> lck(m_mutCritical);
        tagServerAccept* pAcceptEvent = (tagServerAccept*)m_cbBuffer;
        pAcceptEvent->pServerItem = pItem;
        return m_AsynEngine.PostAsynData(MSG_SERVER_ACCEPT, m_cbBuffer, sizeof(tagServerAccept));
    }

    bool Bridge::OnServerRead(ServerItem* pItem, MsgHead* pMsgHead, void* pData, unsigned int wDataSize)
    {
        assert((wDataSize + sizeof(tagServerRead)) <= ASYN_DATA_LEN);
        if ((wDataSize + sizeof(tagServerRead)) > ASYN_DATA_LEN) return false;

        std::unique_lock <std::mutex> lck(m_mutCritical);
        tagServerRead* pReadEvent = (tagServerRead*)m_cbBuffer;
        pReadEvent->MsgHeadInfo = *pMsgHead;
        pReadEvent->wDataSize = wDataSize;
        pReadEvent->pData = (void*)pItem;

        if (wDataSize > 0)
        {
            assert(pData != NULL);
            memcpy(m_cbBuffer + sizeof(tagServerRead), pData, wDataSize);
        }
        return m_AsynEngine.PostAsynData(MSG_SERVER_READ, m_cbBuffer, sizeof(tagServerRead) + wDataSize);
    }

    bool Bridge::OnServerShut(ServerItem* pItem)
    {
        std::unique_lock <std::mutex> lck(m_mutCritical);
        tagServerShut* pCloseEvent = (tagServerShut*)m_cbBuffer;
        pCloseEvent->pData = (void*)pItem;
        return m_AsynEngine.PostAsynData(MSG_SERVER_SHUT, m_cbBuffer, sizeof(tagServerShut));
    }

    bool Bridge::OnTimer(unsigned int unTimerID, unsigned int unParam)
    {
        std::unique_lock <std::mutex> lck(m_mutCritical);
        tagTimer* pTimerEvent = (tagTimer*)m_cbBuffer;
        pTimerEvent->unTimerID = unTimerID;
        pTimerEvent->unParam = unParam;
        return m_AsynEngine.PostAsynData(MSG_TIMER, m_cbBuffer, sizeof(tagTimer));
    }

    bool Bridge::OnDBResult(unsigned short wRequestID, MsgHead* pMsgHead, void* pData, unsigned int wDataSize)
    {
        assert((wDataSize + sizeof(tagDataBase)) <= ASYN_DATA_LEN);
        if ((wDataSize + sizeof(tagDataBase)) > ASYN_DATA_LEN) return false;
        std::unique_lock <std::mutex> lck(m_mutCritical);
        tagDataBase* pDataBaseEvent = (tagDataBase*)m_cbBuffer;
        pDataBaseEvent->wRequestID = wRequestID;
        pDataBaseEvent->MsgHeadInfo = *pMsgHead;

        if (wDataSize > 0)
        {
            assert(pData != NULL);
            memcpy(m_cbBuffer + sizeof(tagDataBase), pData, wDataSize);
        }
        return m_AsynEngine.PostAsynData(MSG_DATABASE, m_cbBuffer, sizeof(tagDataBase) + wDataSize);
    }

    bool Bridge::OnAsynEngineStart()
    {
        assert(m_pIBridgeHook != NULL);
        if (m_pIBridgeHook == NULL) return false;
        if (m_pIBridgeHook->OnBridgeStart(dynamic_cast<IAsynEngineHook*>(this)) == false)
        {
            assert(false);
            return false;
        }
        return true;
    }

    bool Bridge::OnAsynEngineStop()
    {
        assert(m_pIBridgeHook != NULL);
        if (m_pIBridgeHook == NULL) return false;
        if (m_pIBridgeHook->OnBridgeStop(dynamic_cast<IAsynEngineHook*>(this)) == false)
        {
            assert(false);
            return false;
        }
        return true;
    }

    bool Bridge::OnAsynEngineData(unsigned short wIdentifier, void* pData, unsigned int wDataSize)
    {
        assert(m_pIMsgServer != NULL);
        assert(m_pIBridgeHook != NULL);
        switch (wIdentifier)
        {
        case MSG_TIMER:
        {
            assert(wDataSize == sizeof(tagTimer));
            if (wDataSize != sizeof(tagTimer)) return false;
            tagTimer* pTimerEvent = (tagTimer*)pData;
            m_pIBridgeHook->OnTimer(pTimerEvent->unTimerID, pTimerEvent->unParam);
            return true;
        }
        case MSG_CONTROL:
        {
            tagControl* pControlEvent = (tagControl*)pData;
            m_pIBridgeHook->OnEventControl(pControlEvent->wControlID, pControlEvent + 1, wDataSize - sizeof(tagControl));
            return true;
        }
        case MSG_DATABASE:
        {
            assert(wDataSize >= sizeof(tagDataBase));
            if (wDataSize < sizeof(tagDataBase)) return false;
            tagDataBase* pDataBaseEvent = (tagDataBase*)pData;
            m_pIBridgeHook->OnDataBase(pDataBaseEvent->wRequestID, &pDataBaseEvent->MsgHeadInfo, pDataBaseEvent + 1, wDataSize - sizeof(tagDataBase));
            return true;
        }
        case MSG_CLIENT_READ:
        {
            tagClientRead* pReadEvent = (tagClientRead*)pData;
            assert(wDataSize >= sizeof(tagClientRead));
            assert(wDataSize == (sizeof(tagClientRead) + pReadEvent->wDataSize));
            if (wDataSize < sizeof(tagClientRead)) return false;
            if (wDataSize != (sizeof(tagClientRead) + pReadEvent->wDataSize)) return false;
            m_pIBridgeHook->OnClientRead(pReadEvent->nServerID, &(pReadEvent->MsgHeadInfo), pReadEvent + 1, pReadEvent->wDataSize);
            return true;
        }

        case MSG_CLIENT_SHUT:
        {
            assert(wDataSize == sizeof(tagClientShut));
            if (wDataSize != sizeof(tagClientShut)) return false;
            tagClientShut* pCloseEvent = (tagClientShut*)pData;
            m_pIBridgeHook->OnClientShut(pCloseEvent->nServerID, pCloseEvent->cbShutReason);
            return true;
        }

        case MSG_CLIENT_LINK:
        {
            assert(wDataSize == sizeof(tagClientLink));
            if (wDataSize != sizeof(tagClientLink)) return false;
            tagClientLink* pConnectEvent = (tagClientLink*)pData;
            m_pIBridgeHook->OnClientLink(pConnectEvent->nServerID, pConnectEvent->nErrorCode);
            return true;
        }

        case MSG_SERVER_ACCEPT:
        {
            assert(wDataSize == sizeof(tagServerAccept));
            if (wDataSize != sizeof(tagServerAccept)) return false;
            bool bSuccess = false;
            tagServerAccept* pAcceptEvent = (tagServerAccept*)pData;
            try
            {
                bSuccess = m_pIBridgeHook->OnServerBind(pAcceptEvent->pServerItem);
            }
            catch (...) {}
            if (bSuccess == false) m_pIMsgServer->CloseSocket(pAcceptEvent->pServerItem->GetSocketID());
            return true;
        }
        case MSG_SERVER_READ:
        {
            tagServerRead* pReadEvent = (tagServerRead*)pData;
            assert(wDataSize >= sizeof(tagServerRead));
            assert(wDataSize == (sizeof(tagServerRead) + pReadEvent->wDataSize));
            ServerItem* pServerItem = (ServerItem*)pReadEvent->pData;
            if (wDataSize < sizeof(tagServerRead))
            {
                m_pIMsgServer->CloseSocket(pServerItem->GetSocketID());
                return false;
            }
            if (wDataSize != (sizeof(tagServerRead) + pReadEvent->wDataSize))
            {
                m_pIMsgServer->CloseSocket(pServerItem->GetSocketID());
                return false;
            }
            bool bSuccess = false;
            try
            {
                bSuccess = m_pIBridgeHook->OnServerRead(pServerItem, &(pReadEvent->MsgHeadInfo), pReadEvent + 1, pReadEvent->wDataSize);
            }
            catch (...) {}
            if (bSuccess == false) m_pIMsgServer->CloseSocket(pServerItem->GetSocketID());
            return true;
        }
        case MSG_SERVER_SHUT:
        {
            assert(wDataSize == sizeof(tagServerShut));
            if (wDataSize != sizeof(tagServerShut)) return false;
            tagServerShut* pCloseEvent = (tagServerShut*)pData;
            ServerItem* pServerItem = (ServerItem*)pCloseEvent->pData;
            m_pIBridgeHook->OnServerShut(pServerItem);
            return true;
        }
        case MSG_SERVER_READY:
        {
            assert(wDataSize == sizeof(tagServerReady));
            if (wDataSize != sizeof(tagServerReady)) return false;
            m_pIBridgeHook->OnServerReady();
            return true;
        }
        }
        return m_pIBridgeHook->OnBridgeData(wIdentifier, pData, wDataSize);
    }

}
