/*
*   Bridge.h
*
*   Message transfer.
*
*   Created on: 2018-11-13
*   Author:
*   All rights reserved.
*/
#ifndef __Bridge_H__
#define __Bridge_H__

#include <mutex>

#include <common/BaseCore.h>
#include "AsynEngine.h"

namespace Firefly
{
    class Bridge : public IBridge, public IServer, public IClient,
        public IAsynEngineHook, public ITimerEvent, public IDBEngineEvent
    {
    public:
        Bridge();
        ~Bridge();

    public:
        virtual bool Start();
        virtual bool Stop();
        bool SendControl(unsigned short wControlID, void* pData, unsigned int nDataSize);
    public:
        virtual bool SetServer(IMsgServer* pObject);
        virtual bool SetBridgeHook(IBridgeHook* pObject);

    public:
        virtual bool OnClientLink(unsigned int nSocketID, int nErrorCode);
        virtual bool OnClientShut(unsigned int nSocketID, char cbShutReason);
        virtual bool OnClientRead(MsgHead* pMsgHead, void* pData, unsigned int nDataSize);

    public:
        virtual bool OnServerReady();
		/// <summary>
		/// Server network bind success.
		/// </summary>
		/// <param name="pItem">related to client connection.</param>
		/// <returns>If it is success with binding operation</returns>
        virtual bool OnServerBind(ServerItem *pItem);
        virtual bool OnServerShut(ServerItem* pItem);
        virtual bool OnServerRead(ServerItem* pItem, MsgHead* pMsgHead, void* pData, unsigned int nDataSize);

    public:
        virtual bool OnAsynEngineStart();
        virtual bool OnAsynEngineStop();
        virtual bool OnAsynEngineData(unsigned short wIdentifier, void* pData, unsigned int nDataSize);

    public:
        virtual bool OnTimer(unsigned int unTimerID, unsigned int unParam);
        virtual bool OnDBResult(unsigned short wRequestID, MsgHead* pMsgHead, void* pData, unsigned int nDataSize);

    protected:
        IMsgServer* m_pIMsgServer;
        IBridgeHook* m_pIBridgeHook;
        /// <summary>
        /// Asynchronous data process.
        /// </summary>
        AsynEngine      m_AsynEngine;

    protected:
        char                    m_cbBuffer[ASYN_DATA_LEN];
        std::mutex              m_mutCritical;
    };
}
#endif