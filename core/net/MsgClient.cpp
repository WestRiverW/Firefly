#include "MsgClient.h"
#include <assert.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include <assert.h>
#include <common/BaseDefine.h>
#include <memory.h>
#include <utils/Utility.h>
#include <algorithm>
#include <glog/logging.h>
#include "MsgAssist.h"

namespace Firefly
{
    extern thread_local std::string strThreadLogFlag;

#define MAX_EVENT 20

#define REQUEST_CONNECT             1
#define REQUEST_SEND_DATA           2
#define REQUEST_SEND_DATA_EX        3
#define REQUEST_CLOSE_SOCKET        4

    struct tagConnectRequest
    {
        unsigned int            dwServerID;
        unsigned int            dwServerIP;
        unsigned short          wPort;
    };

    struct tagSendDataRequest
    {
        unsigned int            dwServerID;
        MsgHead					MsgHeadInfo;
    };

    struct tagSendDataExRequest
    {
        unsigned int            dwServerID;
        MsgHead					MsgHeadInfo;
        unsigned int            wDataSize;
        char                    cbSendBuffer[SOCKET_BODY_LEN];
    };

    struct tagCloseDataRequest
    {
        unsigned int            dwServerID;
    };

    static int SetNoBlock(unsigned int socket)
    {
        int flags = fcntl(socket, F_GETFL, 0);

        if (flags < 0)
        {
            return -1;
        }

        fcntl(socket, F_SETFL, flags | O_NONBLOCK);
        return 0;
    }

    //////////////////////////////////////////////////////////////////////////
    ClientItem::ClientItem(unsigned short wIndex, IMsgClient* pIMsgClient)
        : m_hSocket(INVALID_SOCKET),
        m_ClientStatus(SOCKET_IDLE),
        m_wIndex(wIndex),
        m_pIMsgClient(pIMsgClient)
    {
        m_wRountID = 1;
        m_dwServerID = 0;
        m_wRecvSize = 0;
        memset(m_cbRecvBuf, 0, sizeof(m_cbRecvBuf));
        m_bNeedBuffer = false;
        m_dwBufferData = 0L;
        m_dwBufferSize = 0L;
        m_pcbDataBuffer = NULL;
        m_dwSendTickCount = 0;
        m_dwRecvTickCount = 0;
        m_dwRecvPacketCount = 0;
        m_dwClientIP = 0;
        m_wPort = 0;
    }

    ClientItem::~ClientItem()
    {
        if (m_hSocket != INVALID_SOCKET)
        {
            close(m_hSocket);
            m_hSocket = INVALID_SOCKET;
        }

        SafeDeleteArray(m_pcbDataBuffer);
    }

    void ClientItem::ResetData()
    {
        m_hSocket = INVALID_SOCKET;
        m_ClientStatus = SOCKET_IDLE;
        m_wRountID = std::max(1, m_wRountID + 1);
        m_dwServerID = 0;
        m_wRecvSize = 0;
        m_bNeedBuffer = false;
        m_dwBufferData = 0L;
        m_dwBufferSize = 0;
        m_dwSendTickCount = 0;
        m_dwRecvTickCount = 0;
        m_dwRecvPacketCount = 0;
        m_dwClientIP = 0;
        m_wPort = 0;
    }

    //epoll
    int ClientItem::AddSocketToEPoll(int epollfd)
    {
        struct epoll_event ev;
        ev.events = EPOLLIN | EPOLLET | EPOLLOUT;
        ev.data.ptr = (void*)this;
        int nErrorCode = epoll_ctl(epollfd, EPOLL_CTL_ADD, m_hSocket, &ev);
        LOG(INFO) << strThreadLogFlag << __FUNCTION__ << " nErrorCode:" << nErrorCode;

        if (nErrorCode < 0)
        {
            return -1;
        }
        else
        {
            return 0;
        }
    }

    bool ClientItem::OnConnectCompleted()
    {
        int nErrorCode = errno;
        m_pIMsgClient->OnClientLink(m_dwServerID, nErrorCode);

        if (nErrorCode != 0)
        {
            LOG(INFO) << strThreadLogFlag << __FUNCTION__ << "tcpsocketclosesocket:9";
            CloseSocket();
            return false;
        }

        m_ClientStatus = SOCKET_CONNECT;
        return true;
    }

    bool ClientItem::OnCanSend()
    {
        assert(m_hSocket != INVALID_SOCKET);
        if (m_hSocket == INVALID_SOCKET)
        {
            return true;
        }

        if ((m_bNeedBuffer == true) && (m_dwBufferData > 0L))
        {
            unsigned int dwTotalCount = 0;
            unsigned int dwPacketSize = 4096;
            m_dwSendTickCount = Utility::GetTickCount() / 1000L;

            while (dwTotalCount < m_dwBufferData)
            {
                unsigned int wSendSize = (unsigned int)std::min(dwPacketSize, m_dwBufferData - dwTotalCount);
                int nSendCount = send(m_hSocket, (char*)m_pcbDataBuffer + dwTotalCount, wSendSize, 0);

                if (nSendCount == SOCKET_ERROR)
                {
                    if (errno == EWOULDBLOCK)
                    {
                        m_bNeedBuffer = false;
                        m_dwBufferData -= dwTotalCount;

                        if (m_dwBufferData > 0)
                        {
                            m_bNeedBuffer = true;
                            memcpy(m_pcbDataBuffer, m_pcbDataBuffer + dwTotalCount, m_dwBufferData);
                        }

                        return 1;
                    }

                    LOG(INFO) << strThreadLogFlag << __FUNCTION__ << "tcpsocketclosesocket:10";
                    CloseSocket();
                    return 1;
                }

                dwTotalCount += nSendCount;
            }

            m_dwBufferData = 0;
            m_bNeedBuffer = false;
        }

        return true;
    }

    bool ClientItem::OnCanRecv()
    {
        int nResultCode = m_wRecvSize;
        int nRecvLen = 0;

        while (true)
        {
            nRecvLen = recv(m_hSocket, (char*)m_cbRecvBuf + nResultCode, sizeof(m_cbRecvBuf) - nResultCode, 0);

            if (0 == nRecvLen)
            {
                LOG(INFO) << strThreadLogFlag << __FUNCTION__ << "tcpsocketclosesocket:6";
                CloseSocket();
                return false;
            }
            else if (nRecvLen < 0)
            {
                if (errno == EAGAIN)
                {
                    break;
                }
                else
                {
                    LOG(INFO) << strThreadLogFlag << __FUNCTION__ << "tcpsocketclosesocket:7";
                    CloseSocket();
                    return false;
                }
            }
            else if (errno == EINTR)
            {
                continue;
            }
            else
            {
                nResultCode += nRecvLen;
            }
        }

        m_wRecvSize = nResultCode;
        m_dwRecvTickCount = Utility::GetTickCount() / 1000L;
        {
            try
            {
                unsigned int nHandSize = 0;

                while (m_wRecvSize >= MsgAssist::MSG_HEAD_LENGTH)
                {
                    MsgHead* pMsgHead = (MsgHead*)(m_cbRecvBuf + nHandSize);
                    int nBodyLen = pMsgHead->nBodyLen;
                    int nPacketSize = MsgAssist::MSG_HEAD_LENGTH + nBodyLen;
                    if (pMsgHead->wMainCmdID != CMD_CORE_BASE)
                    {
                        m_pIMsgClient->OnClientRead(pMsgHead, (void*)(m_cbRecvBuf + nHandSize + MsgAssist::MSG_HEAD_LENGTH), nBodyLen);
                    }
                    else
                    {
                        switch (pMsgHead->wSubCmdID)
                        {
                        case SUB_CORE_HEART:
                        {
                            SendData(CMD_CORE_BASE, SUB_CORE_HEART);
                        }
                        }
                    }
                    m_dwRecvPacketCount++;
                    m_wRecvSize -= nPacketSize;
                    nHandSize += nPacketSize;
                }
                if (m_wRecvSize > 0 && nHandSize > 0)
                {
                    memcpy(m_cbRecvBuf, m_cbRecvBuf + nHandSize, m_wRecvSize);
                }
            }
            catch (...)
            {
                LOG(INFO) << strThreadLogFlag << __FUNCTION__ << "tcpsocketclosesocket:1";
                CloseSocket();
                return false;
            }
        }
        return true;
    }

    bool ClientItem::OnCloseCompleted()
    {
        LOG(INFO) << strThreadLogFlag << __FUNCTION__ << " 1";
        m_pIMsgClient->OnClientShut(m_dwServerID, 0);
        LOG(INFO) << strThreadLogFlag << __FUNCTION__ << " 2";
        ResetData();
        LOG(INFO) << strThreadLogFlag << __FUNCTION__ << " 3";
        return true;
    }

    ///////////////////////////////////////////////////////////////////////////////
    bool ClientItem::Connect(int epollfd, unsigned int dwServerID, unsigned int dwServerIP, unsigned short wPort)
    {
        assert(m_ClientStatus == SOCKET_IDLE);
        assert((dwServerIP != INADDR_NONE) && (dwServerIP != 0));

        try
        {
            if (m_hSocket != (unsigned short)INVALID_SOCKET) throw CONNECT_EXCEPTION;
            if (m_ClientStatus != SOCKET_IDLE) throw CONNECT_EXCEPTION;
            if ((dwServerIP == INADDR_NONE) || (dwServerIP == 0)) throw CONNECT_EXCEPTION;

            m_wRecvSize = 0;
            m_dwSendTickCount = Utility::GetTickCount() / 1000L;
            m_dwRecvTickCount = Utility::GetTickCount() / 1000L;
            m_dwServerID = dwServerID;
            m_hSocket = socket(AF_INET, SOCK_STREAM, 0);
            if (m_hSocket == INVALID_SOCKET) throw 0;

            //SetNoBlock(m_hSocket);
            struct sockaddr_in SocketAddr;
            memset(&SocketAddr, 0, sizeof(SocketAddr));
            SocketAddr.sin_family = AF_INET;
            SocketAddr.sin_port = htons(wPort);
            SocketAddr.sin_addr.s_addr = dwServerIP;
            //success 0, fail -1
            int nErrorCode = connect(m_hSocket, (struct sockaddr*)&SocketAddr, sizeof(SocketAddr));
            if ((nErrorCode == SOCKET_ERROR) && (errno != EWOULDBLOCK)) throw CONNECT_EXCEPTION;

            if (nErrorCode < 0)
            {
                if (errno == EINPROGRESS)
                {
                    m_ClientStatus = SOCKET_WAIT;
                }
                else
                {
                    throw CONNECT_EXCEPTION;
                }
            }
            else
            {
                SetNoBlock(m_hSocket);
                m_dwClientIP = SocketAddr.sin_addr.s_addr;
                m_wPort = SocketAddr.sin_port;
                m_pIMsgClient->OnClientLink(m_dwServerID, 0);
                m_ClientStatus = SOCKET_CONNECT;
                nErrorCode = AddSocketToEPoll(epollfd);

                if (nErrorCode < 0)
                {
                    throw CONNECT_EXCEPTION;
                }
            }

            return true;
        }
        catch (...)
        {
            LOG(INFO) << strThreadLogFlag << __FUNCTION__;
            return OnConnectCompleted();
        }

        return true;
    }

    bool ClientItem::SendData(unsigned short wMainCmdID, unsigned short wSubCmdID)
    {
        assert(m_hSocket != INVALID_SOCKET);
        assert(m_ClientStatus == SOCKET_CONNECT);
        if (m_hSocket == INVALID_SOCKET) return false;
        if (m_ClientStatus != SOCKET_CONNECT) return false;

        MsgHead msgHeadInfo;
        msgHeadInfo.wMainCmdID = wMainCmdID;
        msgHeadInfo.wSubCmdID = wSubCmdID;

        char cbDataBuffer[SOCKET_BUFFER_LEN] = { 0 };
        int nPacketSize = MsgAssist::MSG_HEAD_LENGTH;
        memcpy(cbDataBuffer, &msgHeadInfo, nPacketSize);
        return SendBuffer(cbDataBuffer, nPacketSize);
    }

    bool ClientItem::SendData(MsgHead* pMsgHead, void* pData, unsigned int wDataSize)
    {
        assert(m_hSocket != INVALID_SOCKET);
        assert(m_ClientStatus == SOCKET_CONNECT);

        if (m_hSocket == INVALID_SOCKET) return false;
        if (m_ClientStatus != SOCKET_CONNECT) return false;

        char cbDataBuffer[SOCKET_BUFFER_LEN] = { 0 };
        int nPacketSize = 0;
        MsgAssist::encode(pMsgHead, pData, wDataSize, cbDataBuffer, nPacketSize);
        auto nSize = SendBuffer(cbDataBuffer, nPacketSize);
        return nSize;
    }

    bool ClientItem::CloseSocket()
    {
        if (m_hSocket != INVALID_SOCKET)
        {
            close(m_hSocket);
            m_hSocket = INVALID_SOCKET;
        }

        OnCloseCompleted();
        return true;
    }

    unsigned int ClientItem::SendBuffer(void* pBuffer, unsigned int wSendSize)
    {
        if (m_bNeedBuffer)
        {
            AmortizeBuffer(pBuffer, wSendSize);
            return 0;
        }
        else
        {
            unsigned int wTotalCount = 0;
            m_dwSendTickCount = Utility::GetTickCount() / 1000L;

            while (wTotalCount < wSendSize)
            {
                int nSendCount = send(m_hSocket, (char*)pBuffer + wTotalCount, wSendSize - wTotalCount, 0);
                if (nSendCount == SOCKET_ERROR)
                {
                    if (errno == EWOULDBLOCK)
                    {
                        AmortizeBuffer((char*)pBuffer + wTotalCount, wSendSize - wTotalCount);
                        return wTotalCount;
                    }

                    CloseSocket();
                    LOG(INFO) << strThreadLogFlag << __FUNCTION__ << "tcpsocketclosesocket:2";
                    return 0;
                }
                else
                {
                    wTotalCount += nSendCount;
                }
            }

            return wSendSize;
        }
    }

    void ClientItem::AmortizeBuffer(void* pData, unsigned int wDataSize)
    {
        if ((m_dwBufferData + wDataSize) > m_dwBufferSize)
        {
            char* pcbDataBuffer = NULL;
            char* pcbDeleteBuffer = m_pcbDataBuffer;
            unsigned int dwNeedSize = m_dwBufferData + wDataSize;
            unsigned int dwApplySize = std::max(dwNeedSize, m_dwBufferSize * 2);

            try
            {
                pcbDataBuffer = new char[dwApplySize];
            }
            catch (...) {}

            if (pcbDataBuffer == NULL)
            {
                LOG(INFO) << strThreadLogFlag << __FUNCTION__ << "tcpsocketclosesocket:3";
                CloseSocket();
                return;
            }

            m_dwBufferSize = dwApplySize;
            m_pcbDataBuffer = pcbDataBuffer;
            memcpy(m_pcbDataBuffer, pcbDeleteBuffer, m_dwBufferData);
            SafeDeleteArray(pcbDeleteBuffer);
        }

        m_bNeedBuffer = true;
        m_dwBufferData += wDataSize;
        memcpy(m_pcbDataBuffer + m_dwBufferData - wDataSize, pData, wDataSize);
        return;
    }

    //////////////////////////////////////////////////////////////////////////
    ClientRWThread::ClientRWThread()
        : m_nEPollfd(0),
        m_pIMsgClient(NULL)
    {
    }

    ClientRWThread::~ClientRWThread()
    {
    }

    std::string ClientRWThread::GetThreadFlag()
    {
        std::string data = "[CliRWThread] ";
        return data;
    }

    bool ClientRWThread::OnStrat()
    {
        return true;
    }

    bool ClientRWThread::OnStop()
    {
        return true;
    }

    bool ClientRWThread::OnRun()
    {
        int nWaitCount = 0;
        struct epoll_event epoll_events[MAX_EVENT];

        /*
            When successful, epoll_wait() returns the number of file descriptors
            ready for the requested I/O, or zero if no file descriptor became
            ready during the requested timeout milliseconds.  When an error
            occurs, epoll_wait() returns -1 and errno is set appropriately.
        */
        do
        {
            nWaitCount = epoll_wait(m_nEPollfd, epoll_events, MAX_EVENT, -1);
        } while (nWaitCount == -1 && errno == EINTR);
        LOG(INFO) << strThreadLogFlag << __FUNCTION__ << " tcpsocketepollwait:1";
        if (nWaitCount == -1 && errno != EINTR)
        {
            LOG(INFO) << strThreadLogFlag << __FUNCTION__ << " tcpsocketepollwait:2";
            return false;
        }

        //nWaitCount>0
        for (int i = 0; i < nWaitCount; ++i)
        {
            LOG(INFO) << strThreadLogFlag << __FUNCTION__ << " tcpsocketepollwait:3 " << nWaitCount;
            OnEpollWaitResult(epoll_events[i]);
        }

        return true;
    }

    unsigned int ClientRWThread::OnEpollWaitResult(epoll_event ev)
    {
        uint32_t events = ev.events;
        ClientItem* pClientItem = (ClientItem*)ev.data.ptr;
        assert(pClientItem);
        std::unique_lock<std::mutex> ThreadLock(pClientItem->GetCriticalSection());

        //i: error
        if (events & EPOLLERR || events & EPOLLHUP)
        {
            LOG(INFO) << strThreadLogFlag << __FUNCTION__ << "tcpsocketclosesocket:4";
            pClientItem->CloseSocket();
        }
        //ii:connect
        /*else if (!pEpollData->bConnect)
        {
            if (events & EPOLLOUT)
            {
                pEpollData->bConnect = true;
            }
        }*/
        //iii:read
        else if (events & EPOLLIN)
        {
            LOG(INFO) << strThreadLogFlag << __FUNCTION__ << "tcpsocketepollwait:5 ";
            pClientItem->OnCanRecv();
        }
        //iiii:write
        else if (events & EPOLLOUT)
        {
            LOG(INFO) << strThreadLogFlag << __FUNCTION__ << "tcpsocketepollwait:6 ";
            pClientItem->OnCanSend();
        }

        return 0;
    }

    //////////////////////////////////////////////////////////////////////////

    MsgClientThread::MsgClientThread()
        : m_nEPollfd(0),
        m_pIMsgClient(NULL)
    {
    }

    MsgClientThread::~MsgClientThread()
    {
    }

    std::string MsgClientThread::GetThreadFlag()
    {
        std::string data = "[CliServiceThread] ";
        return data;
    }

    bool MsgClientThread::PostRequest(unsigned short wIdentifier, void* const pBuffer, unsigned int wDataSize)
    {
        std::unique_lock<std::mutex> ThreadLock(m_mutDataQueue);
        m_DataQueue.InsertData(wIdentifier, pBuffer, wDataSize);
        m_condService.notify_one();
        return true;
    }

    bool MsgClientThread::OnStrat()
    {
        return true;
    }

    bool MsgClientThread::OnStop()
    {
        LOG(INFO) << strThreadLogFlag << __FUNCTION__;
        std::unique_lock<std::mutex> ThreadLock(m_mutDataQueue);
        m_DataQueue.RemoveData(false);
        return true;
    }

    bool MsgClientThread::OnRun()
    {
        std::unique_lock <std::mutex> lck(m_mutService);
        m_condService.wait(lck);
        OnServiceRequest();
        return true;
    }

    unsigned int MsgClientThread::OnServiceRequest()
    {
        do
        {
            tagDataHead DataHead;
            char cbBuffer[ASYN_DATA_LEN] = { 0 };
            {
                std::unique_lock<std::mutex> ThreadLock(m_mutDataQueue);
                if (m_DataQueue.Size() > 0)
                {
                    if (m_DataQueue.DistillData(DataHead, cbBuffer, sizeof(cbBuffer)) == false) return 0;
                }
                else
                {
                    break;
                }
            }
            switch (DataHead.wIdentifier)
            {
            case REQUEST_CONNECT:
            {
                assert(DataHead.wDataSize == sizeof(tagConnectRequest));
                tagConnectRequest* pConnectRequest = (tagConnectRequest*)cbBuffer;
                ClientItem* pClientItem = m_pIMsgClient->GetSocketItem(pConnectRequest->dwServerID);

                if (NULL == pClientItem)
                {
                    ClientItem* pClientItem = m_pIMsgClient->ActiveSocketItem();
                    if (pClientItem)
                    {
                        pClientItem->Connect(m_nEPollfd, pConnectRequest->dwServerID, pConnectRequest->dwServerIP, pConnectRequest->wPort);
                    }
                }
                else
                {
                    
                }

                break;
            }

            case REQUEST_SEND_DATA:
            {
                assert(DataHead.wDataSize == sizeof(tagSendDataRequest));
                tagSendDataRequest* pSendDataRequest = (tagSendDataRequest*)cbBuffer;
                ClientItem* pClientItem = m_pIMsgClient->GetSocketItem(pSendDataRequest->dwServerID);

                if (pClientItem)
                {
                    std::unique_lock<std::mutex> ThreadLock(pClientItem->GetCriticalSection());
                    pClientItem->SendData(&(pSendDataRequest->MsgHeadInfo), NULL, 0);
                }

                break;
            }

            case REQUEST_SEND_DATA_EX:
            {
                LOG(INFO) << strThreadLogFlag << __FUNCTION__ << "  clientsenddataex: 1";
                tagSendDataExRequest* pSendDataExRequest = (tagSendDataExRequest*)cbBuffer;
                assert(DataHead.wDataSize >= (sizeof(tagSendDataExRequest) - sizeof(pSendDataExRequest->cbSendBuffer)));
                assert(DataHead.wDataSize == (pSendDataExRequest->wDataSize + sizeof(tagSendDataExRequest) - sizeof(pSendDataExRequest->cbSendBuffer)));
                ClientItem* pClientItem = m_pIMsgClient->GetSocketItem(pSendDataExRequest->dwServerID);
                LOG(INFO) << strThreadLogFlag << __FUNCTION__ << "  clientsenddataex: 2 " << pSendDataExRequest->dwServerID;
                if (pClientItem)
                {
                    LOG(INFO) << strThreadLogFlag << __FUNCTION__ << "  clientsenddataex: 3 ";
                    std::unique_lock<std::mutex> ThreadLock(pClientItem->GetCriticalSection());
                    pClientItem->SendData(&(pSendDataExRequest->MsgHeadInfo), pSendDataExRequest->cbSendBuffer, pSendDataExRequest->wDataSize);
                }
                LOG(INFO) << strThreadLogFlag << __FUNCTION__ << "  clientsenddataex: 4 ";
                break;
            }

            case REQUEST_CLOSE_SOCKET:
            {
                tagCloseDataRequest* pCloseDataExRequest = (tagCloseDataRequest*)cbBuffer;
                ClientItem* pClientItem = m_pIMsgClient->GetSocketItem(pCloseDataExRequest->dwServerID);

                if (pClientItem)
                {
                    std::unique_lock<std::mutex> ThreadLock(pClientItem->GetCriticalSection());
                    LOG(INFO) << strThreadLogFlag << __FUNCTION__ << "tcpsocketclosesocket:5";
                    pClientItem->CloseSocket();
                }
                else
                {
                }

                break;
            }
            }
        } while (true);

        return 0;
    }

    //////////////////////////////////////////////////////////////////////////
    MsgClient::MsgClient()
        : m_wMaxConnect(1024)
    {
        m_bService = false;
        m_pIClientEvent = NULL;
        return;
    }

    MsgClient::~MsgClient()
    {
        Stop();
    }

    bool MsgClient::Start()
    {
        assert((m_bService == false) && (m_pIClientEvent != NULL));
        if ((m_bService == true) || (m_pIClientEvent == NULL)) return false;
        m_nEPollfd = epoll_create1(0);

        if (m_nEPollfd < 0)
        {
            return false;
        }

        m_ClientRWThread.SetEpoll(m_nEPollfd);
        m_MsgClientThread.SetEpoll(m_nEPollfd);
        m_ClientRWThread.SetMsgClient(this);
        m_MsgClientThread.SetMsgClient(this);

        if (m_ClientRWThread.StartThread() == false) return false;
        if (m_MsgClientThread.StartThread() == false) return false;

        m_bService = true;
        return true;
    }

    bool MsgClient::Stop()
    {
        m_bService = false;
        close(m_nEPollfd);
        m_nEPollfd = 0;
        m_MsgClientThread.StopThread();
        m_ClientRWThread.StopThread();
        return true;
    }

    bool MsgClient::SetClientEvent(IClient* pObject)
    {
        assert(m_bService == false);
        if (m_bService == true) return false;
        m_pIClientEvent = pObject;

        if (m_pIClientEvent == NULL)
        {
            assert(false);
            return false;
        }

        return true;
    }

    bool MsgClient::Connect(unsigned int dwServerID, unsigned int dwServerIP, unsigned short wPort)
    {
        assert(m_bService == true);
        if (m_bService == false) return false;

        tagConnectRequest ConnectRequest;
        memset(&ConnectRequest, 0, sizeof(ConnectRequest));
        ConnectRequest.dwServerID = dwServerID;
        ConnectRequest.wPort = wPort;
        ConnectRequest.dwServerIP = htonl(dwServerIP);
        return m_MsgClientThread.PostRequest(REQUEST_CONNECT, &ConnectRequest, sizeof(ConnectRequest));
    }

    bool MsgClient::Connect(unsigned int dwServerID, const char* szServerIP, unsigned short wPort)
    {
        assert(m_bService == true);
        if (m_bService == false) return false;

        tagConnectRequest ConnectRequest;
        memset(&ConnectRequest, 0, sizeof(ConnectRequest));
        ConnectRequest.dwServerID = dwServerID;
        ConnectRequest.wPort = wPort;
        ConnectRequest.dwServerIP = Utility::TranslateAddress(szServerIP);
        return m_MsgClientThread.PostRequest(REQUEST_CONNECT, &ConnectRequest, sizeof(ConnectRequest));
    }

    bool MsgClient::SendData(unsigned int dwServerID, MsgHead* pMsgHead)
    {
        assert(m_bService == true);
        if (m_bService == false) return false;

        tagSendDataRequest SendDataRequest;
        memset(&SendDataRequest, 0, sizeof(SendDataRequest));
        SendDataRequest.dwServerID = dwServerID;
        SendDataRequest.MsgHeadInfo = *pMsgHead;
        return m_MsgClientThread.PostRequest(REQUEST_SEND_DATA, &SendDataRequest, sizeof(SendDataRequest));
    }

    bool MsgClient::SendData(unsigned int dwServerID, MsgHead* pMsgHead, void* pData, unsigned int wDataSize)
    {
        assert(m_bService == true);
        if (m_bService == false) return false;
        tagSendDataExRequest SendRequestEx;
        memset(&SendRequestEx, 0, sizeof(SendRequestEx));
        SendRequestEx.dwServerID = dwServerID;
        SendRequestEx.MsgHeadInfo = *pMsgHead;
        SendRequestEx.wDataSize = wDataSize;

        if (wDataSize > 0)
        {
            assert(pData != NULL);
            memcpy(SendRequestEx.cbSendBuffer, pData, wDataSize);
        }

        unsigned int wSendSize = sizeof(SendRequestEx) - sizeof(SendRequestEx.cbSendBuffer) + wDataSize;
        return m_MsgClientThread.PostRequest(REQUEST_SEND_DATA_EX, &SendRequestEx, wSendSize);
    }

    bool MsgClient::CloseSocket(unsigned int dwServerID)
    {
        LOG(INFO) << strThreadLogFlag << __FUNCTION__ << "tcpsocketclosesocket:8";
        assert(m_bService == true);
        if (m_bService == false) return false;

        tagCloseDataRequest CloseDataRequest;
        memset(&CloseDataRequest, 0, sizeof(CloseDataRequest));
        CloseDataRequest.dwServerID = dwServerID;
        return m_MsgClientThread.PostRequest(REQUEST_CLOSE_SOCKET, &CloseDataRequest, sizeof(CloseDataRequest));
    }

    bool MsgClient::OnClientLink(unsigned int dwServerID, int nErrorCode)
    {
        assert(m_pIClientEvent != NULL);
        return m_pIClientEvent->OnClientLink(dwServerID, nErrorCode);
    }

    bool MsgClient::OnClientShut(unsigned int dwServerID, char cbShutReason)
    {
        assert(m_pIClientEvent != NULL);
        FreeSocketItem(dwServerID);
        return m_pIClientEvent->OnClientShut(dwServerID, cbShutReason);
    }

    bool MsgClient::OnClientRead(MsgHead* pMsgHead, void* pData, unsigned int wDataSize)
    {
        assert(m_pIClientEvent != NULL);
        return m_pIClientEvent->OnClientRead(pMsgHead, pData, wDataSize);
    }

    ClientItem* MsgClient::ActiveSocketItem()
    {
        std::unique_lock<std::mutex> ThreadLock(m_ItemLocked);
        ClientItem* pClientItem = NULL;

        if (m_ClientItemFree.size() > 0)
        {
            pClientItem = *(m_ClientItemFree.begin());
            m_ClientItemFree.erase(m_ClientItemFree.begin());
            m_ClientItemActive.push_back(pClientItem);
        }
        else
        {
            unsigned short wStoreCount = m_ClientItemStore.size();

            if (wStoreCount < m_wMaxConnect)
            {
                try
                {
                    pClientItem = new ClientItem(wStoreCount, dynamic_cast<IMsgClient*>(this));

                    if (pClientItem == NULL)
                    {
                        assert(false);
                        return NULL;
                    }

                    m_ClientItemActive.push_back(pClientItem);
                    m_ClientItemStore.push_back(pClientItem);
                }
                catch (...)
                {
                    assert(false);
                    return NULL;
                }
            }
        }

        return pClientItem;
    }

    ClientItem* MsgClient::GetSocketItem(unsigned int dwServerID)
    {
        std::unique_lock<std::mutex> ThreadLock(m_ItemLocked);
        ClientItem* pClientItem = NULL;
        int nActiveCount = m_ClientItemActive.size();

        for (int i = 0; i < nActiveCount; ++i)
        {
            if (m_ClientItemActive.at(i)->m_dwServerID == dwServerID)
            {
                pClientItem = m_ClientItemActive.at(i);
                break;
            }
        }

        return pClientItem;
    }

    bool MsgClient::FreeSocketItem(unsigned int dwServerID)
    {
        std::unique_lock<std::mutex> ThreadLock(m_ItemLocked);
        auto iterEnd = m_ClientItemActive.end();
        ClientItem* pClientItem = NULL;

        for (auto iter = m_ClientItemActive.begin(); iter != iterEnd; ++iter)
        {
            pClientItem = *iter;

            if (pClientItem->m_dwServerID == dwServerID)
            {
                m_ClientItemActive.erase(iter);
                m_ClientItemFree.push_back(pClientItem);
                return true;
            }
        }

        assert(false);
        return false;
    }
}