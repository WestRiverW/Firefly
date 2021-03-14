/*
*   MsgClient.h
*
*   Network Client Process.
*
*   Created on: 2018-11-07
*   Author:
*   All rights reserved.
*/
#ifndef __MsgClient_H__
#define __MsgClient_H__

#include <vector>
#include <mutex>
#include <condition_variable>
#include <sys/epoll.h>
#include <utils/Thread.h>
#include <utils/DataQueue.h>
#include <common/BaseDefine.h>
#include <common/BaseCore.h>

namespace Firefly
{
    class ClientItem
    {
    public:
        ClientItem(unsigned short wIndex, IMsgClient* pIMsgClient);
        virtual ~ClientItem();
        void ResetData();

    public:
        int AddSocketToEPoll(int epollfd);

    public:
        bool OnConnectCompleted();
        bool OnCanSend();
        bool OnCanRecv();
        bool OnCloseCompleted();
    public:
        bool Connect(int epollfd, unsigned int dwServerID, unsigned int dwServerIP, unsigned short wPort);
        bool SendData(unsigned short wMainCmdID, unsigned short wSubCmdID);
        bool SendData(MsgHead* pMsgHead, void* pData, unsigned int wDataSize);
        bool CloseSocket();
    public:
        inline std::mutex& GetCriticalSection()
        {
            return m_mutClientItem;
        }
    private:
        unsigned int SendBuffer(void* pBuffer, unsigned int wSendSize);
        void AmortizeBuffer(void* pData, unsigned int wDataSize);
    public:
        unsigned short      m_hSocket;
        unsigned char       m_ClientStatus;
        unsigned short      m_wIndex;
        unsigned short      m_wRountID;
        unsigned int        m_dwServerID;
        IMsgClient 			*m_pIMsgClient;
        std::mutex          m_mutClientItem;

    public:
        unsigned int        m_wRecvSize;
        unsigned char       m_cbRecvBuf[SOCKET_BUFFER_LEN];

    public:
        bool                m_bNeedBuffer;
        unsigned int        m_dwBufferData;
        unsigned int        m_dwBufferSize;
        char* m_pcbDataBuffer;

    public:
        unsigned int        m_dwSendTickCount;
        unsigned int        m_dwRecvTickCount;
        unsigned int        m_dwRecvPacketCount;

        unsigned int        m_dwClientIP;
        unsigned short      m_wPort;
    };

    typedef std::vector<ClientItem*> CClientItemPtrArray;
    class MsgClient;
    //epoll_wait
    class ClientRWThread : public Thread
    {
    protected:
        int                         m_nEPollfd;
        MsgClient* m_pIMsgClient;
    public:
        ClientRWThread();
        virtual ~ClientRWThread();

    public:
        void SetEpoll(int epoll)
        {
            m_nEPollfd = epoll;
        }
        void SetMsgClient(MsgClient* pMsgClient)
        {
            m_pIMsgClient = pMsgClient;
        }

        std::string GetThreadFlag();

    public:
        virtual bool OnRun();
        virtual bool OnStrat();
        virtual bool OnStop();

    private:
        unsigned int OnEpollWaitResult(epoll_event ev);
    };

    //Connect、SendData、Close
    class MsgClientThread : public Thread
    {
        friend class MsgClient;
    protected:
        DataQueue                 m_DataQueue;
        std::mutex                  m_mutDataQueue;
        //
        std::mutex                  m_mutService;
        std::condition_variable     m_condService;

        int                         m_nEPollfd;
        MsgClient* m_pIMsgClient;

    public:
        MsgClientThread();
        virtual ~MsgClientThread();

    public:
        void SetEpoll(int epoll)
        {
            m_nEPollfd = epoll;
        }
        void SetMsgClient(MsgClient* pMsgClient)
        {
            m_pIMsgClient = pMsgClient;
        }
        std::string GetThreadFlag();
    public:
        virtual bool OnRun();
        virtual bool OnStrat();
        virtual bool OnStop();

    public:
        bool PostRequest(unsigned short wIdentifier, void* const pBuffer, unsigned int wDataSize);

    private:
        unsigned int OnServiceRequest();
    };

    //////////////////////////////////////////////////////////////////////////
    class MsgClient : public IMsgClient
    {
    protected:
        bool                    m_bService;
        IClient* m_pIClientEvent;
        int                     m_nEPollfd;

    protected:
        unsigned short          m_wMaxConnect;
        std::mutex              m_ItemLocked;
        CClientItemPtrArray  m_SocketItemFree;
        CClientItemPtrArray  m_SocketItemActive;
        CClientItemPtrArray  m_SocketItemStore;

        ClientRWThread       m_ClientRWThread;
        MsgClientThread  m_MsgClientThread;

    public:
        MsgClient();
        virtual ~MsgClient();

    public:
        virtual bool Start();
        virtual bool Stop();

    public:
        virtual bool SetClientEvent(IClient* pObject);
    public:
        virtual bool Connect(unsigned int dwServerID, unsigned int dwServerIP, unsigned short wPort);
        virtual bool Connect(unsigned int dwServerID, const char* szServerIP, unsigned short wPort);
        virtual bool SendData(unsigned int dwServerID, MsgHead* pMsgHead);
        virtual bool SendData(unsigned int dwServerID, MsgHead* pMsgHead, void* pData, unsigned int wDataSize);
        virtual bool CloseSocket(unsigned int dwServerID);

    public:
        bool OnClientLink(unsigned int dwServerID, int nErrorCode);
        bool OnClientRead(MsgHead* pMsgHead, void* pData, unsigned int wDataSize);
        bool OnClientShut(unsigned int dwServerID, char cbShutReason);

    public:
        ClientItem* ActiveSocketItem();
        ClientItem* GetSocketItem(unsigned int dwServerID);
        bool FreeSocketItem(unsigned int dwServerID);
    };
}

#endif