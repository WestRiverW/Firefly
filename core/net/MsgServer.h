/*
*   MsgServer.h
*
*   Network MsgServer.
*
*   Created on: 2018-11-06
*   Author:
*   All rights reserved.
*/
#ifndef __MsgServer_H__
#define __MsgServer_H__

#include <vector>
#include <arpa/inet.h>
#include <utils/Thread.h>
#include <common/BaseDefine.h>
#include <common/BaseCore.h>
#include <common/AsynEngine.h>
#include <utils/Utility.h>

namespace Firefly
{
    class ServerItem;
    class MsgServer;

    enum eMsgType
    {
        eMsgType_Send,
        eMsgType_Recv,
    };

    class FFMsgServer
    {
    public:
        const eMsgType       m_eMsgType;

    public:
        FFMsgServer(eMsgType msgType);
        virtual ~FFMsgServer();

    public:
        eMsgType GetMsgType()
        {
            return m_eMsgType;
        }
    };

    class FFMsgServerSend : public FFMsgServer
    {
    public:
        unsigned char   m_cbBuffer[SOCKET_BUFFER_LEN];
        unsigned int    m_wHead;
        unsigned int    m_wTail;

    public:
        FFMsgServerSend();
        virtual ~FFMsgServerSend();

    public:
        void clear();
        unsigned int length() const;
    };

    typedef std::vector<FFMsgServerSend*>  FFMsgServerSendPtr;

    class IServerItemHook : public FFObject
    {
    public:
        virtual ~IServerItemHook() {}
        virtual bool OnServerBind(ServerItem* pServerItem) = 0;
        virtual bool OnServerRead(ServerItem* pItem, MsgHead* pMsgHead, void* pData, unsigned int nDataSize) = 0;
        virtual bool OnServerShut(ServerItem* pServerItem) = 0;
    };

    class ServerThreadAccept : public Thread
    {
    public:
        ServerThreadAccept() {}
        virtual ~ServerThreadAccept() {}

    public:
        bool InitThread(int nepfd, int hListenSocket, MsgServer* pServer);
        std::string GetThreadFlag();

    private:
        virtual bool OnRun();

    private:
        int                     m_nEpfd;
        int                     m_hListenSocket;
        MsgServer* m_pServer;
    };

    class ServerThreadDetect : public Thread
    {
    protected:
        unsigned int            m_dwElapseTime;
        unsigned int            m_dwDetectTime;
        MsgServer* m_pServer;

    public:
        ServerThreadDetect() {}
        virtual ~ServerThreadDetect() {}

    public:
        bool InitThread(MsgServer* pServer);
        std::string GetThreadFlag();
    private:
        virtual bool OnRun();
    };

    class ServerThreadReadWrite : public Thread
    {
    public:
        ServerThreadReadWrite() {}
        virtual ~ServerThreadReadWrite() {}

    public:
        bool InitThread(int nEpfd);
        std::string GetThreadFlag();
    private:
        virtual bool OnRun();

    private:
        int                 m_nEpfd;
    };

    class ServerItem
    {
        friend class MsgServer;
    public:
        ServerItem(unsigned short wIndex, IServerItemHook* pIServerItemHook);
        virtual ~ServerItem();

    public:
        int Attach(int hSocket, unsigned int nClientIP, unsigned short port);
        int ResetData();

        void SetEvents(uint32_t unevents)
        {
            m_EpollEvents = unevents;
        }
        uint32_t GetEvents() const
        {
            return m_EpollEvents;
        }
    public:
        inline unsigned short GetIndex()
        {
            return m_wIndex;
        }
        inline unsigned short GetRountID()
        {
            return m_wRountID;
        }
        inline unsigned int GetSocketID()
        {
            return Utility::MAKELONG(m_wIndex, m_wRountID);
        }
        inline int GetConnectType()
        {
            return m_enConnectType;
        }
    public:
        bool OnCanSend();
        bool OnCanRecv();
        bool OnCloseCompleted();

    public:
        inline char* GetClientIpStr()
        {
            return m_szIp;
        }
        inline unsigned int GetClientIP()
        {
            return m_dwClientIP;
        }
        inline unsigned int GetActiveTime()
        {
            return m_dwActiveTime;
        }
        inline std::mutex& GetCriticalSection()
        {
            return m_CriticalSection;
        }
        inline unsigned int GetSendTickCount()
        {
            return m_dwSendTickCount;
        }
        inline unsigned int GetRecvTickCount()
        {
            return m_dwRecvTickCount;
        }
        inline unsigned int GetRecvPacketCount()
        {
            return m_dwRecvPacketCount;
        }
        inline unsigned short GetClientPort()
        {
            return m_usPort;
        }
    public:
        inline void SetClientIpStr(sockaddr_in* pclientaddr);
        bool SendData(MsgHead* pMsgHead);
        bool SendData(MsgHead* pMsgHead, unsigned char* pData, unsigned short nDataSize);
        int SendData(FFMsgServerSend* pMsgServerSend);
        bool CloseSocket(unsigned short wRountID);

    protected:
        inline FFMsgServerSend* GetSendBuffer(unsigned int wPacketSize);
        void ResetSendBuffer(FFMsgServerSend* pMsgServerSend);

    protected:
        inline bool IsAllowSendData()
        {
            return m_dwRecvPacketCount > 0L;
        }
        inline bool IsValidSocket()
        {
            return (m_hSocketHandle != INVALID_SOCKET);
        }
    public:
        unsigned int GetShutDownTickCount()
        {
            return m_dwShutDownTickCount;
        }
    public:
        bool ShutDown(unsigned short wRountID);
    private:
        unsigned short        m_wIndex;
        unsigned short        m_wRountID;
        std::mutex            m_CriticalSection;
    protected:
        char                  m_szIp[20];
        unsigned int          m_dwClientIP;
        unsigned short        m_usPort;
        unsigned int          m_dwActiveTime;

    protected:
        int                   m_hSocketHandle;
        unsigned short        m_wSurvivalTime;
        uint32_t              m_EpollEvents;

    protected:
        unsigned int          m_wRecvSize;
        unsigned char         m_cbRecvBuf[SOCKET_BUFFER_LEN];

    protected:
        eConnectType         m_enConnectType;
        unsigned int          m_dwShutDownTickCount;
        IServerItemHook* m_pIServerItemHook;

    protected:
        FFMsgServerSendPtr    m_MsgSendActive;
        FFMsgServerSendPtr    m_MsgSendFree;

    protected:
        unsigned int          m_dwSendTickCount;
        unsigned int          m_dwRecvTickCount;
        unsigned int          m_dwRecvPacketCount;

    protected:
        bool                  m_bShutDown;
    };

    typedef std::vector<ServerItem*>    ServerItemList;
    typedef std::vector<ServerThreadReadWrite*> ServerThreadRWList;

    class MsgServer : public IMsgServer, public IServerItemHook,
        public IAsynEngineHook
    {
        friend class ServerThreadDetect;
        friend class ServerThreadAccept;
        friend class ServerThreadReadWrite;

    public:
        MsgServer();
        virtual ~MsgServer();

    public:
        virtual bool Start();
        virtual bool Stop();
    public:
        virtual unsigned short GetServicePort();
        virtual int GetConnectType(unsigned int dwSocketID);

    public:
        virtual bool SetNetServer(IServer* pObject);
        virtual bool SetInfo(unsigned short wServicePort, unsigned short wMaxConnect);

    public:
        virtual bool SendData(MsgHead* pMsgHead);
        virtual bool SendData(MsgHead* pMsgHead, void* pData, unsigned int nDataSize);
        virtual bool SendDataEx(MsgHead* pMsgHead, void* pData, unsigned int nDataSize);

    public:
        virtual bool CloseSocket(unsigned int dwSocketID);
        virtual bool ShutDown(unsigned int dwSocketID);

    public:
        virtual bool OnServerReady();
        virtual bool OnServerBind(ServerItem* pServerItem);
        virtual bool OnServerRead(ServerItem* pItem, MsgHead* pMsgHead, void* pData, unsigned int nDataSize);
        virtual bool OnServerShut(ServerItem* pServerItem);

        virtual bool OnAsynEngineStart();
        virtual bool OnAsynEngineStop();
        virtual bool OnAsynEngineData(unsigned short wIdentifier, void* pData, unsigned int nDataSize);

    protected:
        ServerItem* ActiveServerItem();
        ServerItem* GetServerItem(unsigned short wIndex);
        bool FreeServerItem(ServerItem* pServerItem);

    private:
        bool DetectSocket();
    protected:
        unsigned short      m_wMaxConnect;
        int                 m_nEPollfd;
        unsigned short      m_wServicePort;
        char                m_cbBuffer[ASYN_DATA_LEN];
        bool                m_bService;
        int                 m_hServerSocket;

    protected:
        std::mutex          m_ItemLocked;
        ServerItemList  m_ServerItemFree;
        ServerItemList  m_ServerItemActive;
        ServerItemList  m_ServerItemStore;
        ServerItemList  m_TempServerItemArray;

    protected:
        std::mutex                      m_BufferLocked;
        ServerThreadDetect          m_ServerDetectThread;
        ServerThreadAccept          m_ServerAcceptThread;
        ServerThreadRWList          m_ServerRWThreadArray;

        IServer* m_pIServerEvent;
        AsynEngine              m_AsynEngine;
    };
}
#endif