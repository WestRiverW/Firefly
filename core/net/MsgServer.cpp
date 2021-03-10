#include "MsgServer.h"
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include <assert.h>
#include <condition_variable>
#include <mutex>
#include <memory.h>
#include "MsgAssist.h"
#include <glog/logging.h>

namespace Firefly
{
    #define MAX_EVENT 20

    #define DEAD_QUOTIETY               0
    #define DANGER_QUOTIETY             1
    #define SAFETY_QUOTIETY             2

    #define Asyn_SEND_DATA      1
    #define Asyn_SEND_BATCH     2
    #define Asyn_SHUT_DOWN      3
    #define Asyn_ALLOW_BATCH    4
    #define Asyn_CLOSE_SOCKET   5
    #define Asyn_DETECT_SOCKET  6
    #define Asyn_CONNECTED      7

    static std::mutex               g_mutAccept;
    static std::condition_variable  g_condAccept;

    static std::mutex               g_mutRW;
    static std::condition_variable  g_condRW;

    static ServerItemList       g_TCPNetworkList;
    static std::mutex               g_mutServerItem;

    struct tagSendDataRequest
    {
        MsgHead					MsgHeadInfo;
        unsigned int            wDataSize;
        unsigned char           cbSendBuffer[SOCKET_BODY_LEN];
    };

    struct tagCloseSocket
    {
        unsigned short          wIndex;
        unsigned short          wRountID;
    };

    struct tagShutDown
    {
        unsigned short          wIndex;
        unsigned short          wRountID;
    };
    /*
    typedef union epoll_data {
        void *ptr;
        int fd;
        __uint32_t u32;
        __uint64_t u64;
    } epoll_data_t;

    struct epoll_event {
        __uint32_t events;  //Epoll events
        epoll_data_t data;  //User data variable
    };

    */

    static int setSocketNoBlocking( int fd )
    {
        int nFlags = fcntl( fd, F_GETFL, 0 );

        if( nFlags < 0 )
        {
            return -1;
        }

        nFlags |= O_NONBLOCK;
        int nRet = fcntl( fd, F_SETFL, nFlags );

        if( nRet < 0 )
        {
            return -1;
        }

        return 0;
    }

    COverLapped::COverLapped( enOperationType OperationType )
        : m_OperationType( OperationType )
    {
    }

    COverLapped::~COverLapped()
    {
    }

    COverLappedSend::COverLappedSend()
        : COverLapped( enOperationType_Send )
    {
        clear();
    }

    COverLappedSend::~COverLappedSend()
    {
    }

    void COverLappedSend::clear()
    {
        memset( m_cbBuffer, 0, sizeof( m_cbBuffer ) );
        m_wHead = 0;
        m_wTail = 0;
    }

    unsigned int COverLappedSend::length() const
    {
        return SOCKET_BUFFER_LEN;
    }

    bool ServerThreadAccept::InitThread( int nepfd, int hListenSocket, MsgServer *pServer )
    {
        assert( nepfd );
        assert( hListenSocket );
        assert( pServer );
        m_nEpfd = nepfd;
        m_hListenSocket = hListenSocket;
        m_pServer = pServer;
        return true;
    }

    std::string ServerThreadAccept::GetThreadFlag()
    {
        std::string data = "[SvrAccThread] ";
        return data;
    }

    bool ServerThreadAccept::OnRun()
    {
        try
        {
            std::unique_lock <std::mutex> lck( g_mutAccept );
            g_condAccept.wait( lck );
            char host_buf[NI_MAXHOST] = { 0 };
            char port_buf[NI_MAXSERV] = { 0 };
            ServerItem *pServerItem = NULL;
            LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << "  tcpnetworkaccept 1";

            for( ;; )
            {
                struct sockaddr client_addr = { 0 };
                socklen_t client_addr_len = sizeof( client_addr );
                int nAcceptfd = accept( m_hListenSocket, &client_addr, &client_addr_len );
                LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << "  tcpnetworkaccept 2 " << nAcceptfd;

                if( nAcceptfd <= 0 )
                {
                    LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << "  tcpnetworkaccept 3 ";

                    if( errno != EAGAIN &&
                            errno != ECONNABORTED &&
                            errno != EPROTO &&
                            errno != EINTR )
                    {
                        assert( false );
                        perror( "accept error!" );
                    }
                    else
                    {
                        break;
                    }
                }

                int nRet = setSocketNoBlocking( nAcceptfd );

                if( nRet < 0 )
                {
                    assert( NULL );
                    break;
                }

                nRet = getnameinfo( &client_addr, sizeof( client_addr ),
                                    host_buf, sizeof( host_buf ) / sizeof( host_buf[0] ),
                                    port_buf, sizeof( port_buf ) / sizeof( port_buf[0] ),
                                    NI_NUMERICHOST | NI_NUMERICSERV );
								
                pServerItem = m_pServer->ActiveNetworkItem();
                if( pServerItem == NULL )
                {
                    assert( NULL );
                    break;
                }

                sockaddr_in *pclientaddr = ( sockaddr_in * )( &client_addr );
                pServerItem->SetClientIpStr( pclientaddr );
                pServerItem->Attach( nAcceptfd, pclientaddr->sin_addr.s_addr, pclientaddr->sin_port );
                struct epoll_event ev;
                ev.data.ptr = ( void * )pServerItem;
                ev.events = EPOLLIN | EPOLLET | EPOLLOUT;
                nRet = epoll_ctl( m_nEpfd, EPOLL_CTL_ADD, nAcceptfd, &ev );
                LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << "  tcpnetworkaccept 4 ";
            }
        }
        catch( ... )
        {
        }

        return true;
    }

    //////////////////////////////////////////////////////////////////////////
    /// <Summary>
    /// Init server info.
    /// </Summary>
    /// <Param name="pServer">Network server pointer.</Param>
    /// <Returns></Returns>
    bool ServerThreadDetect::InitThread( MsgServer *pServer )
    {
        assert( pServer != NULL );

        m_dwElapseTime = 0L;
        m_dwDetectTime = 20000000;
        m_pServer = pServer;
        return true;
    }

    std::string ServerThreadDetect::GetThreadFlag()
    {
        std::string data = "[SvrDtcThread] ";
        return data;
    }

    bool ServerThreadDetect::OnRun()
    {
        assert( m_pServer != NULL );
        usleep( 500000 );
        m_dwElapseTime += 500000;

        if( m_dwElapseTime >= m_dwDetectTime )
        {
            m_dwElapseTime = 0L;
            m_pServer->DetectSocket();
        }

        return true;
    }

    //////////////////////////////////////////////////////////////////////////
    bool ServerThreadReadWrite::InitThread( int nEpfd )
    {
        assert( nEpfd );
        m_nEpfd = nEpfd;
        return true;
    }

    std::string ServerThreadReadWrite::GetThreadFlag()
    {
        std::string data = "[SvrRWThread] ";
        return data;
    }

    bool ServerThreadReadWrite::OnRun()
    {
        std::unique_lock <std::mutex> lck( g_mutRW );
        g_condRW.wait( lck );
        LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << "  tcpnetworkepollwait read write 2";
        g_mutServerItem.lock();

        if( 0 == g_TCPNetworkList.size() )
        {
            g_mutServerItem.unlock();
            return true;
        }

        ServerItem     *pServerItem = *( g_TCPNetworkList.rbegin() );
        g_TCPNetworkList.pop_back();
        g_mutServerItem.unlock();
        std::unique_lock <std::mutex> datlck( pServerItem->GetCriticalSection() );

        if( ( pServerItem->GetEvents() & EPOLLIN ) == EPOLLIN )
        {
            LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << "  tcpnetworkepollwait read ";
            pServerItem->OnCanRecv();
        }

        if( ( pServerItem->GetEvents() & EPOLLOUT ) == EPOLLOUT )
        {
            LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << "  tcpnetworkepollwait write ";
            pServerItem->OnCanSend();
        }

        return true;
    }
    //////////////////////////////////////////////////////////////////////////
    ServerItem::ServerItem( unsigned short wIndex, IServerItemHook *pIServerItemHook )
        : m_wIndex( wIndex ),
          m_pIServerItemHook( pIServerItemHook )
    {
        m_dwClientIP = 0L;
        m_dwActiveTime = 0L;
        m_wRountID = 1;
        m_wSurvivalTime = DEAD_QUOTIETY;
        m_hSocketHandle = INVALID_SOCKET;
        m_bShutDown = false;
        m_wRecvSize = 0;
        memset( m_cbRecvBuf, 0, sizeof( m_cbRecvBuf ) );
        m_dwSendTickCount = 0L;
        m_dwRecvTickCount = 0L;
        m_dwRecvPacketCount = 0L;
        m_enConnectType = eConnectType_Unknow;
        m_dwShutDownTickCount = 0;
    }

    ServerItem::~ServerItem()
    {
        for( size_t i = 0; i < m_OverLappedSendFree.size(); i++ )
        {
            delete m_OverLappedSendFree[i];
        }

        for( size_t i = 0; i < m_OverLappedSendActive.size(); i++ )
        {
            delete m_OverLappedSendActive[i];
        }

        m_OverLappedSendFree.clear();
        m_OverLappedSendActive.clear();
    }

    int ServerItem::Attach( int hSocket, unsigned int nClientIP, unsigned short port )
    {
        assert( hSocket );
        assert( nClientIP );
        assert( m_hSocketHandle == INVALID_SOCKET );
        m_hSocketHandle = hSocket;
        m_dwClientIP = nClientIP;
        m_usPort = port;
        m_dwActiveTime = time( NULL );
        m_wSurvivalTime = SAFETY_QUOTIETY;
        m_pIServerItemHook->OnServerBind( this );
        return GetSocketID();
    }

    int ServerItem::ResetData()
    {
        m_wRountID = std::max( 1, m_wRountID + 1 );
        m_dwClientIP = 0;
        m_usPort = 0;
        m_dwActiveTime = 0;
        m_hSocketHandle = INVALID_SOCKET;
        m_wRecvSize = 0;
        memset( m_cbRecvBuf, 0, sizeof( m_cbRecvBuf ) );
        m_enConnectType = eConnectType_Unknow;
        m_dwSendTickCount = 0;
        m_dwRecvTickCount = 0;
        m_dwRecvPacketCount = 0;
        m_bShutDown = false;
        m_wSurvivalTime = 0;
        m_OverLappedSendFree.insert( m_OverLappedSendFree.end(), m_OverLappedSendActive.begin(), m_OverLappedSendActive.end() );
        m_OverLappedSendActive.clear();
        return 0;
    }

    bool ServerItem::OnCanSend()
    {
        if( m_hSocketHandle == INVALID_SOCKET )
        {
            return true;
        }

        if( m_OverLappedSendActive.size() > 0 )
        {
            for( unsigned int i = 0; i < m_OverLappedSendActive.size(); ++i )
            {
                COverLappedSend *pOverLappedSend = m_OverLappedSendActive[i];
                int nRet = SendData( pOverLappedSend );

                if( nRet < 0 )
                {
                    LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " tcpnetworkclosesocket:5";
                    CloseSocket( m_wRountID );
                    return false;
                }
            }
        }

        return true;
    }

    bool ServerItem::OnCanRecv()
    {
    ContinueRecvGoto:
        bool bContinueRecvGoto = false;
        int nResultCode = m_wRecvSize;
        int nRecvLen = 0;

        while( true && nResultCode < SOCKET_BUFFER_LEN )
        {
            nRecvLen = recv( m_hSocketHandle, ( char * )m_cbRecvBuf + nResultCode, sizeof( m_cbRecvBuf ) - nResultCode, 0 );
            if( 0 == nRecvLen )
            {
                LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " tcpnetworkclosesocket:6";
                CloseSocket( m_wRountID );
                return false;
            }
            else if( nRecvLen < 0 )
            {
                if( errno == EAGAIN )
                {
                    DLOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " networkrecv " << " top 3 ";
                    break;
                }
                else if( errno == EINTR )
                {
                    DLOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " networkrecv " << " top 5 ";
                    continue;
                }
                else
                {
                    LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " tcpnetworkclosesocket:7";
                    CloseSocket( m_wRountID );
                    return false;
                }
            }
            else
            {
                nResultCode += nRecvLen;

                if( nResultCode >= SOCKET_BUFFER_LEN )
                {
                    bContinueRecvGoto = true;
                    break;
                }
            }
        }

        DLOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " networkrecv " << " top 6 ";
        m_wRecvSize = nResultCode;
        m_wSurvivalTime = SAFETY_QUOTIETY;
        m_dwRecvTickCount = Utility::GetTickCount();
        if( m_enConnectType == eConnectType_WebSocket )
        {
            try
            {
                while (m_wRecvSize > 0)
                {
                    int nRet = Utility::WSGetFrameType((char*)m_cbRecvBuf, m_wRecvSize);
                    std::string strFrame;
                    if (nRet == enWSContinueFrame)
                    {
                        DLOG(INFO) << strThreadLogFlag << __FUNCTION__ << " networkrecv 1";
                        strFrame = "enWSContinueFrame";
                        break;
                    }
                    if (nRet > enWSPongFrame)
                    {
                        DLOG(INFO) << strThreadLogFlag << __FUNCTION__ << " networkrecv 2";
                        strFrame = "big than enWSPongFrame";
                        LOG(INFO) << strThreadLogFlag << __FUNCTION__ << " tcpnetworkclosesocket:8";
                        CloseSocket(m_wRountID);
                        return true;
                    }

                    if (nRet == enWSTextFrame)
                    {
                        DLOG(INFO) << strThreadLogFlag << __FUNCTION__ << " networkrecv 3";
                        strFrame = "enWSTextFrame";
                        std::string strRecvData;
                        unsigned int wPacketSize = 0;
                        strRecvData.assign((const char*)(&wPacketSize), MsgAssist::MSG_HEAD_LENGTH);

                        do
                        {
                            std::string strtempData;
                            unsigned int wPacketHead = 0;
                            unsigned int wPacketBody = 0;
                            nRet = Utility::WSDecodeFrame((char*)m_cbRecvBuf + wPacketSize, m_wRecvSize - wPacketSize, strtempData, wPacketHead, wPacketBody);
                            wPacketSize = wPacketSize + wPacketHead + wPacketBody;
                            strRecvData = strRecvData + strtempData;
                        } while (nRet == 0);

                        if (nRet == enWSErrorFrame)
                        {
                            DLOG(INFO) << strThreadLogFlag << __FUNCTION__ << " networkrecv 4 nRet:" << nRet;
                            break;
                        }
                        m_dwRecvPacketCount++;
                        unsigned int nPacket = htonl(strRecvData.size());
                        MsgHead* pMsgHead = (MsgHead*)(m_cbRecvBuf + wPacketSize);
                        if (pMsgHead->nBodyLen > 0)
                        {
                            if (pMsgHead->wMainCmdID != CMD_CORE_BASE)
                            {
                                m_pIServerItemHook->OnServerRead(this,pMsgHead,(void*)(m_cbRecvBuf + wPacketSize),pMsgHead->nBodyLen);
                            }
                            else
                            {
                            }
                            m_wRecvSize -= wPacketSize;
                            if (m_wRecvSize > 0) memcpy(m_cbRecvBuf, m_cbRecvBuf + wPacketSize, m_wRecvSize);
                        }
                        else
                        {
                            throw(0);
                        }
                    }
                    else if (nRet == enWSBinaryFrame)
                    {
                        DLOG(INFO) << strThreadLogFlag << __FUNCTION__ << " networkrecv 4";
                        strFrame = "enWSBinaryFrame";
                        char cbBuffer[SOCKET_BUFFER_LEN] = { 0 };
                        unsigned int wPacketSize = 0;
                        unsigned int wProtocSize = MsgAssist::MSG_HEAD_LENGTH;

                        do
                        {
                            unsigned int wPacketHead = 0;
                            unsigned int wPacketBody = 0;
                            nRet = Utility::WSDecodeFrame((char*)m_cbRecvBuf + wPacketSize, m_wRecvSize - wPacketSize, cbBuffer + wProtocSize, wPacketHead, wPacketBody);
                            DLOG(INFO) << strThreadLogFlag << __FUNCTION__ << " networkrecv 5 len:" << wPacketHead << "  " << wPacketBody << "  " << nRet;
                            wPacketSize = wPacketSize + wPacketHead + wPacketBody;
                            wProtocSize = wProtocSize + wPacketBody;
                        } while (nRet == 0);

                        DLOG(INFO) << strThreadLogFlag << __FUNCTION__ << " networkrecv 5 len:" << wPacketSize << "  " << wProtocSize;

                        if (nRet == enWSErrorFrame)
                        {
                            DLOG(INFO) << strThreadLogFlag << __FUNCTION__ << " networkrecv 5 len:" << nRet;
                            break;
                        }

                        m_dwRecvPacketCount++;
                        MsgHead* pMsgHead = (MsgHead*)(m_cbRecvBuf + wPacketSize);
                        if (pMsgHead->nBodyLen > 0)
                        {
                            if (pMsgHead->wMainCmdID != CMD_CORE_BASE)
                            {
                                m_pIServerItemHook->OnServerRead(this, pMsgHead, (void*)(m_cbRecvBuf + wPacketSize), pMsgHead->nBodyLen);
                            }
                            else
                            {
                                
                            }
                            m_wRecvSize -= wPacketSize;
                            if (m_wRecvSize > 0) memcpy(m_cbRecvBuf, m_cbRecvBuf + wPacketSize, m_wRecvSize);
                        }
                        else
                        {
                            throw(0);
                        }
                    }
                    else if (nRet == enWSClosingFrame)
                    {
                        strFrame = "enWSClosingFrame";
                        char cbRecvBuf[256] = { 0 };
                        unsigned int wPacketHead = 0;
                        unsigned int wPacketBody = 0;
                        Utility::WSDecodeFrame((char*)m_cbRecvBuf, m_wRecvSize, cbRecvBuf, wPacketHead, wPacketBody);
                        m_wRecvSize -= wPacketHead;
                        if (m_wRecvSize > 0) memcpy(m_cbRecvBuf, m_cbRecvBuf + wPacketHead, m_wRecvSize);
                        LOG(INFO) << strThreadLogFlag << __FUNCTION__ << " tcpnetworkclosesocket:9";
                        CloseSocket(m_wRountID);
                        return true;
                    }
                    else if (nRet == enWSPingFrame)
                    {
                        strFrame = "enWSPingFrame";
                        std::string strRecvData;
                        unsigned int wPacketHead = 0;
                        unsigned int wPacketBody = 0;
                        nRet = Utility::WSDecodeFrame((char*)m_cbRecvBuf, nResultCode, strRecvData, wPacketHead, wPacketBody);
                        unsigned short wHeadLen = wPacketHead;
                        std::string strSendData;
                        char cbBuffer[1024] = { 0 };
                        nRet = Utility::WSEncodeFrame(strRecvData, cbBuffer, wPacketHead, enWSPongFrame);
                        nRet = send(m_hSocketHandle, cbBuffer, wPacketHead, 0);
                        m_wRecvSize -= wHeadLen;
                        if (m_wRecvSize > 0) memcpy(m_cbRecvBuf, m_cbRecvBuf + wPacketHead, m_wRecvSize);
                    }
                    else if (nRet == enWSPongFrame)
                    {
                        strFrame = "enWSPongFrame";
                        std::string strRecvData;
                        unsigned int wPacketHead = 0;
                        unsigned int wPacketBody = 0;
                        nRet = Utility::WSDecodeFrame((char*)m_cbRecvBuf, nResultCode, strRecvData, wPacketHead, wPacketBody);
                        unsigned int wHeadLen = wPacketHead;
                        std::string strSendData;
                        char cbBuffer[1024] = { 0 };
                        nRet = Utility::WSEncodeFrame(strRecvData, cbBuffer, wPacketHead, enWSPingFrame);
                        nRet = send(m_hSocketHandle, cbBuffer, wPacketHead, 0);
                        m_wRecvSize -= wHeadLen;
                        if (m_wRecvSize > 0) memcpy(m_cbRecvBuf, m_cbRecvBuf + wPacketHead, m_wRecvSize);
                    }
                }
            }
            catch (...)
            {
                LOG(INFO) << strThreadLogFlag << __FUNCTION__ << " tcpnetworkclosesocket:10";
                CloseSocket(m_wRountID);
                return false;
            }
        }
        else if( m_enConnectType == eConnectType_Unknow )
        {
            do
            {
                if( m_wRecvSize < sizeof(MsgHead) )
                {
                    break;
                }

                std::string strRecvData( ( char * )m_cbRecvBuf );

                if( strRecvData.find( "GET " ) != std::string::npos )
                {
                    std::string strResponse;
                    enWebSocketStatus enWsStatus = Utility::WSHandShake( m_cbRecvBuf, strResponse );

                    if( enWsStatus == enWebSocketStatus_Connect )
                    {
                        m_enConnectType = eConnectType_WebSocket;
                        m_wRecvSize = 0;
                        size_t nSendSize = 0, nTotalSize = 0;;

                        while( nTotalSize < strResponse.length() )
                        {
                            nSendSize = send( m_hSocketHandle, strResponse.c_str(), strResponse.length(), 0 );
                            if( nSendSize < 0 )
                            {
                                LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " tcpnetworkclosesocket:11";
                                CloseSocket( m_wRountID );
                                return true;
                            }
                            else
                            {
                                nTotalSize += nSendSize;
                            }
                        }

                        m_dwRecvPacketCount++;
                    }
                }
                else
                {
                    m_enConnectType = eConnectType_Socket;
                }
            }
            while( false );
        }

        if( m_enConnectType == eConnectType_Socket )
        {
            try
            {
                int nHandSize = 0;

                while(m_wRecvSize >= MsgAssist::MSG_HEAD_LENGTH)
                {
				    MsgHead *pMsgHead = (MsgHead*)(m_cbRecvBuf + nHandSize);
				    int nBodyLen = pMsgHead->nBodyLen;
				    int nPacketSize = MsgAssist::MSG_HEAD_LENGTH + nBodyLen;

				    pMsgHead->nSocketID = GetSocketID();
				    if( pMsgHead->wMainCmdID != CMD_CORE_BASE )
				    {
					    m_pIServerItemHook->OnServerRead( this, pMsgHead, ( void * )( m_cbRecvBuf + MsgAssist::MSG_HEAD_LENGTH + nHandSize ), nBodyLen);
				    }

				    m_dwRecvPacketCount++;
				    m_wRecvSize -= nPacketSize;
				    nHandSize += nPacketSize;
                }
                if( m_wRecvSize > 0 && nHandSize > 0 ) memcpy( m_cbRecvBuf, m_cbRecvBuf + nHandSize, m_wRecvSize );
            }
            catch( ... )
            {
                LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " tcpnetworkclosesocket:12";
                CloseSocket( m_wRountID );
                return false;
            }
        }

        if( bContinueRecvGoto )
        {
            LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << "ContinueRecvGoto";
            goto ContinueRecvGoto;
        }

        return true;
    }

    bool ServerItem::OnCloseCompleted()
    {
        if( m_pIServerItemHook != NULL )
        {
            m_pIServerItemHook->OnServerShut( this );
        }
	
	    LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " tcpnetworkclosesocket:22 ";

        ResetData();
        return false;
    }

    inline void ServerItem::SetClientIpStr( sockaddr_in *pclientaddr )
    {
        char *inaddr;
        inaddr = inet_ntoa( pclientaddr->sin_addr );
        strcpy( m_szIp, inaddr );
    }

    bool ServerItem::SendData( MsgHead* pMsgHead)
    {
	    unsigned short wRountID = Utility::LOWORD(pMsgHead->nSocketID);
        if( !IsValidSocket() )
        {
            LOG( INFO ) << strThreadLogFlag << __FUNCTION__;
            CloseSocket( wRountID );
            return false;
        }

        unsigned int wPacketHead = sizeof( MsgHead );
        COverLappedSend *pOverLappedSend = GetSendOverLapped( wPacketHead );

        if( pOverLappedSend == NULL )
        {
            LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " tcpnetworkclosesocket:13";
            CloseSocket( wRountID );
            return false;
        }

        unsigned int wSourceLen = pOverLappedSend->m_wTail;
        int nPacketSize  = MsgAssist::MSG_HEAD_LENGTH;
        //end

        if( m_enConnectType == eConnectType_WebSocket )
        {
            // unsigned int nLen = 0;
            // Utility::WSEncodeFrame( cbDataBuffer, nPacketSize, ( char * )( pOverLappedSend->m_cbBuffer + wSourceLen ), nLen, enWSBinaryFrame );
            // pOverLappedSend->m_wTail += nLen;
        }
        else
        {
            memcpy( pOverLappedSend->m_cbBuffer + wSourceLen, (char*)pMsgHead, nPacketSize );
            pOverLappedSend->m_wTail += nPacketSize;
        }

        {
            int nRet = SendData( pOverLappedSend );

            if( nRet < 0 )
            {
                LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " tcpnetworkclosesocket:14";
                CloseSocket( m_wRountID );
                return false;
            }
        }
        return true;
    }

    bool ServerItem::SendData( MsgHead *pMsgHead, unsigned char* pData,unsigned short wDataSize)
    {
	    unsigned short wRountID = Utility::LOWORD(pMsgHead->nSocketID);
        assert( wDataSize <= SOCKET_BODY_LEN );

        if( wDataSize > SOCKET_BODY_LEN ) return false;
	
	    LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << "SendDataTest 1:"<<wDataSize;

        if( !IsValidSocket() )
        {
		    LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << "SendDataTest 2:"<<wDataSize;
            CloseSocket( wRountID );
            return false;
        }
	    LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << "SendDataTest 3:"<<wDataSize;

        unsigned int wPacketHead = sizeof( MsgHead ) + wDataSize;
        COverLappedSend *pOverLappedSend = GetSendOverLapped( wPacketHead );

        if( pOverLappedSend == NULL )
        {
            LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " tcpnetworkclosesocket:15";
            CloseSocket( wRountID );
            return false;
        }

        unsigned int wSourceLen = pOverLappedSend->m_wTail;
        if( m_enConnectType == eConnectType_WebSocket )
        {
		    LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << "SendDataTest 4:"<<wDataSize;
		
            unsigned int nLen = 0;
            char *pJsonData = ( char * )( pOverLappedSend->m_cbBuffer + wSourceLen );

            if( wDataSize > 0 )
            {
                assert( pData != NULL );
                Utility::WSEncodeFrame( ( char * )pData + MsgAssist::MSG_HEAD_LENGTH, wDataSize - MsgAssist::MSG_HEAD_LENGTH, pJsonData, nLen, enWSBinaryFrame );
            }

            pOverLappedSend->m_wTail += nLen;
        }
        else
        {
		    LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << "SendDataTest 5:"<<wDataSize;
            //probuffer data have been format into data
            char *buffer = ( char * )( pOverLappedSend->m_cbBuffer + wSourceLen );
		
		    memcpy( buffer, (char*)pMsgHead, MsgAssist::MSG_HEAD_LENGTH );
            pOverLappedSend->m_wTail += MsgAssist::MSG_HEAD_LENGTH;

            assert( pData != NULL );
            memcpy( buffer + MsgAssist::MSG_HEAD_LENGTH, pData, wDataSize );

            pOverLappedSend->m_wTail += wDataSize;
        }

        int nRet = SendData( pOverLappedSend );

        if( nRet < 0 )
        {
            LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " tcpnetworkclosesocket:16 " << errno << " ret:" << nRet;
            CloseSocket( m_wRountID );
            return false;
        }
	
	    LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << "SendDataTest 6:"<<wDataSize << "  "<<nRet;

        return true;
    }

    //success：1，fail；0
    int ServerItem::SendData( COverLappedSend *pOverLappedSend )
    {
        unsigned int wRemainData = pOverLappedSend->m_wTail - pOverLappedSend->m_wHead;
        unsigned char *cbStartData = &pOverLappedSend->m_cbBuffer[pOverLappedSend->m_wHead];
        while( wRemainData > 0 && IsValidSocket() )
        {
            ssize_t wSendSize = send( m_hSocketHandle, cbStartData, wRemainData, 0 );

            if( wSendSize > 0 )
            {
                pOverLappedSend->m_wHead += wSendSize;
                cbStartData += wSendSize;
                wRemainData -= wSendSize;
            }
            else if( 0 == wSendSize || errno == EAGAIN )
            {
                LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " tcpnetworksend:1 " << wSendSize;
                break;
            }
            else if( errno == EINTR )
            {
                LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " tcpnetworksend:2 " << wSendSize;
                continue;
            }
            else
            {
                LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " tcpnetworksend:3 " << wSendSize;
                return -1;
            }
        }
        if( 0 == wRemainData || !IsValidSocket() )
        {
            RestoreOverLapped( pOverLappedSend );
            return 1;
        }
        else
        {
            return 0;
        }
    }

    bool ServerItem::CloseSocket( unsigned short wRountID )
    {
        LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " tcpnetworkclosesocket:20 "<<m_wRountID<<"  "<<wRountID;
        if( m_wRountID != wRountID ) return false;
	    LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " tcpnetworkclosesocket:21 " << m_hSocketHandle;
        if( m_hSocketHandle != -1 )
        {
            close( m_hSocketHandle );
            m_hSocketHandle = INVALID_SOCKET;
        }

        {
            OnCloseCompleted();
        }
        return true;
    }

    COverLappedSend *ServerItem::GetSendOverLapped( unsigned int wPacketHead )
    {
        if( m_OverLappedSendActive.size() > 1 )
        {
            size_t nActiveCount = m_OverLappedSendActive.size();
            COverLappedSend *pOverLappedSend = m_OverLappedSendActive[nActiveCount - 1];
            unsigned int wRemainData = pOverLappedSend->m_wTail - pOverLappedSend->m_wHead;

            if( wRemainData + wPacketHead <= sizeof( pOverLappedSend->m_cbBuffer ) )
            {
                if( pOverLappedSend->m_wTail + wPacketHead > pOverLappedSend->length() )
                {
                    memcpy( &pOverLappedSend->m_cbBuffer[0], &pOverLappedSend->m_cbBuffer[pOverLappedSend->m_wHead], \
                            pOverLappedSend->m_wTail - pOverLappedSend->m_wHead );
                    pOverLappedSend->m_wTail -= pOverLappedSend->m_wHead;
                    pOverLappedSend->m_wHead = 0;
                    return pOverLappedSend;
                }
            }
        }

        if( m_OverLappedSendFree.size() > 0 )
        {
            size_t nFreeCount = m_OverLappedSendFree.size();
            COverLappedSend *pOverLappedSend = m_OverLappedSendFree[nFreeCount - 1];

            pOverLappedSend->clear();
            m_OverLappedSendActive.push_back( pOverLappedSend );
            m_OverLappedSendFree.erase( m_OverLappedSendFree.begin() + m_OverLappedSendFree.size() - 1 );
            return pOverLappedSend;
        }

        try
        {
            COverLappedSend *pOverLappedSend = new COverLappedSend;
            assert( pOverLappedSend != NULL );

            pOverLappedSend->clear();
            m_OverLappedSendActive.push_back( pOverLappedSend );
            return pOverLappedSend;
        }
        catch( ... )
        {
            assert( false );
        }

        return NULL;
    }

    void ServerItem::RestoreOverLapped( COverLappedSend *pOverLappedSend )
    {
        pOverLappedSend->clear();

        for( auto iter = m_OverLappedSendActive.begin(); iter != m_OverLappedSendActive.end(); ++iter )
        {
            if( *iter == pOverLappedSend )
            {
                m_OverLappedSendActive.erase( iter );
                break;
            }
        }

        m_OverLappedSendFree.push_back( pOverLappedSend );
        m_wSurvivalTime = SAFETY_QUOTIETY;
        m_dwSendTickCount = Utility::GetTickCount();
    }

    bool ServerItem::ShutDown( unsigned short wRountID )
    {
        if( m_hSocketHandle == INVALID_SOCKET ) return false;
        if( ( m_wRountID != wRountID ) || ( m_bShutDown == true ) ) return false;

        if( m_enConnectType == eConnectType_WebSocket )
        {
		    MsgHead msgHeadInfo;
		    msgHeadInfo.wMainCmdID =CMD_CORE_BASE;
		    msgHeadInfo.wSubCmdID = SUB_CORE_SHUT_SOCKET;
            SendData( &msgHeadInfo );
        }

        m_wRecvSize = 0;
        m_bShutDown = true;
        m_dwShutDownTickCount = Utility::GetTickCount();
        return true;
    }

    unsigned short MsgServer::GetServicePort()
    {
        return m_wServicePort;
    }

    int MsgServer::GetConnectType( unsigned int dwSocketID )
    {
        unsigned short wIndex = Utility::LOWORD( dwSocketID );
        LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " networkitemindex 3:" << wIndex << " dwSocketID:" << dwSocketID;
        ServerItem *pServerItem = GetNetworkItem( wIndex );
        if( pServerItem == NULL ) return eConnectType_Unknow;
        return pServerItem->m_enConnectType;
    }

    bool MsgServer::SetInfo( unsigned short wServicePort, unsigned short wMaxConnect )
    {
        m_wServicePort = wServicePort;
        m_wMaxConnect = wMaxConnect;
        return true;
    }

    bool MsgServer::SendData( MsgHead *pMsgHead )
    {
        std::unique_lock <std::mutex> lck( m_BufferLocked );
        tagSendDataRequest *pSendDataRequest = ( tagSendDataRequest * )m_cbBuffer;
        pSendDataRequest->wDataSize = 0;
        pSendDataRequest->MsgHeadInfo = *pMsgHead;
        unsigned short wSendSize = sizeof( tagSendDataRequest ) - sizeof( pSendDataRequest->cbSendBuffer );
        //protobuffer start
        return m_AsynEngine.PostAsynData( Asyn_SEND_DATA, m_cbBuffer, wSendSize );
    }

    bool MsgServer::SendData( MsgHead *pMsgHead, void *pData, unsigned int wDataSize )
    {
        assert( ( wDataSize + sizeof( MsgHead ) ) <= SOCKET_BODY_LEN );
        if( ( wDataSize + sizeof( MsgHead ) ) > SOCKET_BODY_LEN ) return false;

        std::unique_lock <std::mutex> lck( m_BufferLocked );
        tagSendDataRequest *pSendDataRequest = ( tagSendDataRequest * )m_cbBuffer;
        pSendDataRequest->wDataSize = wDataSize;
        pSendDataRequest->MsgHeadInfo = *pMsgHead;
        unsigned int wSendSize = sizeof( tagSendDataRequest ) - sizeof( pSendDataRequest->cbSendBuffer ) + wDataSize;

        if( wDataSize > 0 )
        {
            assert( pData != NULL );
            memcpy( pSendDataRequest->cbSendBuffer, pData, wDataSize );
        }

        //end
        return m_AsynEngine.PostAsynData( Asyn_SEND_DATA, m_cbBuffer, wSendSize );
    }

    bool MsgServer::SendDataEx( MsgHead *pMsgHead, void *pData, unsigned int wDataSize )
    {
        assert( ( wDataSize + sizeof( MsgHead ) ) <= SOCKET_BODY_LEN );
        if( ( wDataSize + sizeof( MsgHead ) ) > SOCKET_BODY_LEN ) return false;

        std::unique_lock <std::mutex> lck( m_BufferLocked );
        tagSendDataRequest *pSendDataRequest = ( tagSendDataRequest * )m_cbBuffer;
        pSendDataRequest->wDataSize = wDataSize;
        pSendDataRequest->MsgHeadInfo = *pMsgHead;
        unsigned int wSendSize = sizeof( tagSendDataRequest ) - sizeof( pSendDataRequest->cbSendBuffer ) + wDataSize;

        if( pData && wDataSize > 0 )
        {
            assert( pData != NULL );
            memcpy( pSendDataRequest->cbSendBuffer, pData, wDataSize );
        }

        return m_AsynEngine.PostAsynData( Asyn_SEND_BATCH, m_cbBuffer, wSendSize );
    }

    bool MsgServer::CloseSocket( unsigned int dwSocketID )
    {
        LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " tcpnetworkclosesocket:17";
        std::unique_lock<std::mutex> ThreadLock( m_BufferLocked );
        tagCloseSocket *pCloseSocket = ( tagCloseSocket * )m_cbBuffer;
        pCloseSocket->wIndex = Utility::LOWORD( dwSocketID );
        pCloseSocket->wRountID = Utility::HIWORD( dwSocketID );
        return m_AsynEngine.PostAsynData( Asyn_CLOSE_SOCKET, m_cbBuffer, sizeof( tagCloseSocket ) );
    }

    bool MsgServer::ShutDown( unsigned int dwSocketID )
    {
        std::unique_lock<std::mutex> ThreadLock( m_BufferLocked );
        tagShutDown *pShutDown = ( tagShutDown * )m_cbBuffer;
        pShutDown->wIndex = Utility::LOWORD( dwSocketID );
        pShutDown->wRountID = Utility::HIWORD( dwSocketID );
        return m_AsynEngine.PostAsynData( Asyn_SHUT_DOWN, m_cbBuffer, sizeof( tagShutDown ) );
    }

    bool MsgServer::OnServerReady()
    {
        m_pIServerEvent->OnServerReady();
        return true;
    }

    bool MsgServer::OnServerBind( ServerItem *pServerItem )
    {
        //accept
        assert( m_pIServerEvent != NULL );
        m_pIServerEvent->OnServerBind( pServerItem );
        return true;
    }

    bool MsgServer::OnServerRead( ServerItem *pItem, MsgHead *pMsgHead, void *pData, unsigned int wDataSize )
    {
        assert( m_pIServerEvent != NULL );
        m_pIServerEvent->OnServerRead( pItem, pMsgHead, pData, wDataSize );
        return true;
    }

    bool MsgServer::OnServerShut( ServerItem *pServerItem )
    {
        assert( pServerItem != nullptr );
        assert( m_pIServerEvent != nullptr );

        try
        {
            m_pIServerEvent->OnServerShut( pServerItem );
            FreeNetworkItem( pServerItem );
        }
        catch( ... ) {}

        return true;
    }

    bool MsgServer::OnAsynEngineStart()
    {
        return true;
    }

    bool MsgServer::OnAsynEngineStop()
    {
        return true;
    }

    bool MsgServer::OnAsynEngineData( unsigned short wIdentifier, void *pData, unsigned int wDataSize )
    {
        switch( wIdentifier )
        {
            case Asyn_SEND_DATA:
            {
                tagSendDataRequest *pSendDataRequest = ( tagSendDataRequest * )pData;
                assert( wDataSize >= ( sizeof( tagSendDataRequest ) - sizeof( pSendDataRequest->cbSendBuffer ) ) );
                assert( wDataSize == ( pSendDataRequest->wDataSize + sizeof( tagSendDataRequest ) - sizeof( pSendDataRequest->cbSendBuffer ) ) );

			    unsigned short wIndex = Utility::HIWORD(pSendDataRequest->MsgHeadInfo.nSocketID);
                ServerItem *pServerItem = GetNetworkItem( wIndex );

                if( pServerItem == NULL || !pServerItem->IsValidSocket() ) return false;

                std::unique_lock <std::mutex> lck( pServerItem->GetCriticalSection() );
                pServerItem->SendData( &(pSendDataRequest->MsgHeadInfo), \
                                           pSendDataRequest->cbSendBuffer, pSendDataRequest->wDataSize);
                return true;
            }

            case Asyn_SEND_BATCH:
            {
                tagSendDataRequest *pSendDataRequest = ( tagSendDataRequest * )pData;
                assert( wDataSize >= ( sizeof( tagSendDataRequest ) - sizeof( pSendDataRequest->cbSendBuffer ) ) );
                assert( wDataSize == ( pSendDataRequest->wDataSize + sizeof( tagSendDataRequest ) - sizeof( pSendDataRequest->cbSendBuffer ) ) );

                for( size_t i = 0; i < m_TempNetworkItemArray.size(); i++ )
                {
                    ServerItem *pServerItem = m_TempNetworkItemArray[i];
                    std::unique_lock <std::mutex> ThreadLock( pServerItem->GetCriticalSection() );
                    pServerItem->SendData( &(pSendDataRequest->MsgHeadInfo), pSendDataRequest->cbSendBuffer, pSendDataRequest->wDataSize );
                }

                return true;
            }

            case Asyn_SHUT_DOWN:
            {
                assert( wDataSize == sizeof( tagShutDown ) );
                tagShutDown *pShutDown = ( tagShutDown * )pData;
                LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " networkitemindex 1:" << pShutDown->wIndex;
                ServerItem *pServerItem = GetNetworkItem( pShutDown->wIndex );

                if( pServerItem == NULL || !pServerItem->IsValidSocket() ) return false;

                std::unique_lock<std::mutex> ThreadLock( pServerItem->GetCriticalSection() );
                pServerItem->ShutDown( pShutDown->wRountID );
                return true;
            }

            case Asyn_CLOSE_SOCKET:
            {
                assert( wDataSize == sizeof( tagCloseSocket ) );
                tagCloseSocket *pCloseSocket = ( tagCloseSocket * )pData;
                LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " networkitemindex 5:" << pCloseSocket->wIndex;
                ServerItem *pServerItem = GetNetworkItem( pCloseSocket->wIndex );
                if( pServerItem == NULL || !pServerItem->IsValidSocket() ) return false;
                std::unique_lock<std::mutex> ThreadLock( pServerItem->GetCriticalSection() );
                LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " tcpnetworkclosesocket:18";
                pServerItem->CloseSocket( pCloseSocket->wRountID );
                return true;
            }

            case Asyn_DETECT_SOCKET:
            {
                m_ItemLocked.lock();
                m_TempNetworkItemArray.clear();
                m_TempNetworkItemArray.insert( m_TempNetworkItemArray.begin(), m_NetworkItemActive.begin(), m_NetworkItemActive.end() );
                m_ItemLocked.unlock();
                time_t dwNowTime = time( NULL );

                for( size_t i = 0; i < m_TempNetworkItemArray.size(); i++ )
                {
                    ServerItem *pServerItem = m_TempNetworkItemArray[i];

                    if( pServerItem == NULL )
                    {
                        continue;
                    }

                    std::unique_lock <std::mutex> ThreadLock( pServerItem->GetCriticalSection() );
                    if( pServerItem->IsValidSocket() == false ) continue;

                    if( pServerItem->IsAllowSendData() == true )
                    {
                        switch( pServerItem->m_wSurvivalTime )
                        {
                            case DEAD_QUOTIETY:
                            {
                                LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " DEAD_QUOTIETY";
                                LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " tcpnetworkclosesocket:1";
                                pServerItem->CloseSocket( pServerItem->GetRountID() );
                                break;
                            }

                            case DANGER_QUOTIETY:
                            {
                                LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " DANGER_QUOTIETY";
                                pServerItem->m_wSurvivalTime--;
							    MsgHead msgHeadInfo;
							    msgHeadInfo.wMainCmdID =CMD_CORE_BASE;
							    msgHeadInfo.wSubCmdID = SUB_CORE_HEART;
                                pServerItem->SendData( &msgHeadInfo );
                                break;
                            }

                            default:
                            {
                                //LOG(INFO)<<strThreadLogFlag << __FUNCTION__<<" wSurvivalTime:"<<pServerItem->m_wSurvivalTime;
                                pServerItem->m_wSurvivalTime--;
                                break;
                            }
                        }
                    }
                    else
                    {
                        if( ( pServerItem->GetActiveTime() + 4 ) <= dwNowTime )
                        {
                            LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " active time:" << pServerItem->GetActiveTime() << ",now time:" << dwNowTime;
                            LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " tcpnetworkclosesocket:2";
                            pServerItem->CloseSocket( pServerItem->GetRountID() );
                            continue;
                        }
                    }

                    unsigned int dwShutDownTickCount = pServerItem->GetShutDownTickCount();

                    if( 0 != dwShutDownTickCount && dwShutDownTickCount + 12000 < Utility::GetTickCount() )
                    {
                        LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " dwShutDownTickCount:" << dwShutDownTickCount;
                        LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " tcpnetworkclosesocket:3 " << dwShutDownTickCount;
                        pServerItem->CloseSocket( pServerItem->GetRountID() );
                        continue;
                    }
                }

                return true;
            }
        }

        return true;
    }
    //////////////////////////////////////////////////////////////////////////

    MsgServer::MsgServer()
        : m_wMaxConnect( 4096 ),
          m_nEPollfd( 0 ),
          m_wServicePort( 0 )
    {
        m_bService = false;
        memset( m_cbBuffer, 0, sizeof( m_cbBuffer ) );
        m_pIServerEvent = NULL;
        m_hServerSocket = INVALID_SOCKET;
    }

    MsgServer::~MsgServer()
    {
        close( m_nEPollfd );
        m_nEPollfd = 0;
    }

    bool MsgServer::Start()
    {
        //1、create socket
        m_hServerSocket = socket( AF_INET, SOCK_STREAM, 0 );

        if( m_hServerSocket < 0 )
        {
            DLOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " fail 1:";
            return false;
        }

        //2、addr reuse.
        int opt = SO_REUSEADDR;
        int nRet = setsockopt( m_hServerSocket, SOL_SOCKET, SO_REUSEADDR, ( const void * )&opt, sizeof( opt ) );

        if( nRet < 0 )
        {
            DLOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " fail 2:";
            return false;
        }

        //3、bind
        struct sockaddr_in server_addr = {0};
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons( m_wServicePort );
        nRet = bind( m_hServerSocket, ( const struct sockaddr * )&server_addr, sizeof( server_addr ) );

        if( nRet < 0 )
        {
            DLOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " fail 3:";
            return false;
        }

        nRet = setSocketNoBlocking( m_hServerSocket );

        if( nRet < 0 )
        {
            DLOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " fail 4:";
            return false;
        }

        //4、listen
        nRet = listen( m_hServerSocket, 200 );

        if( nRet < 0 )
        {
            DLOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " fail 5:";
            return false;
        }

        m_AsynEngine.SetAsynHook( dynamic_cast<IAsynEngineHook *>( this ) );;

        if( m_AsynEngine.Start() == false )
        {
            DLOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " fail 6:";
            assert( false );
            return false;
        }
	    //5、epoll create.
        struct epoll_event ev, epoll_events[MAX_EVENT];
        //fail - 1.
        m_nEPollfd = epoll_create1(0);
        if( m_nEPollfd < 0 )
        {
            DLOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " fail 7:";
            return false;
        }

        ev.data.fd = m_hServerSocket;
        //EPOLLET：non-block socket
        //LT（level triggered）
        ev.events = EPOLLIN | EPOLLET;
        //6、epoll_ctl
        nRet = epoll_ctl( m_nEPollfd, EPOLL_CTL_ADD, m_hServerSocket, &ev );

        if( nRet < 0 )
        {
            DLOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " fail 8:";
            return false;
        }

        if( m_ServerAcceptThread.InitThread( m_nEPollfd, m_hServerSocket, this ) == false )
        {
            DLOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " fail 9:";
            return false;
        }
        else
        {
            if( m_ServerAcceptThread.StartThread() == false ) return false;
        }

        if( m_ServerDetectThread.InitThread( this ) == false )
        {
            DLOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " fail 10:";
            return false;
        }
        else
        {
            if( m_ServerDetectThread.StartThread() == false ) return false;
        }

        char    cbThreadCount = 2;
        for( char i = 0; i < cbThreadCount; ++i )
        {
            ServerThreadReadWrite *pNetworkRSThread = new ServerThreadReadWrite();
            if( pNetworkRSThread->InitThread( m_nEPollfd ) == false ) return false;
            m_ServerRWThreadArray.push_back( pNetworkRSThread );
            if( pNetworkRSThread->StartThread() == false ) return false;
        }

        OnServerReady();
        //7、epoll wait.
        int nWaitCount = 0;

        for( ;; )
        {
            /*
                When successful, epoll_wait() returns the number of file descriptors
                ready for the requested I/O, or zero if no file descriptor became
                ready during the requested timeout milliseconds.  When an error
                occurs, epoll_wait() returns -1 and errno is set appropriately.
            */
            do
            {
                nWaitCount = epoll_wait( m_nEPollfd, epoll_events, MAX_EVENT, -1 );
            }
            while( nWaitCount == -1 && errno == EINTR );
            if( nWaitCount == -1 && errno != EINTR )
            {
                break;
            }
            for( int i = 0; i < nWaitCount; ++i )
            {
                uint32_t events = epoll_events[i].events;
                if( events & EPOLLERR || events & EPOLLHUP )
                {
                    close( epoll_events[i].data.fd );
                    continue;
                }
                else if( m_hServerSocket == epoll_events[i].data.fd )
                {
                    g_condAccept.notify_one();
                    continue;
                }
                else
                {
                    ServerItem *pServerItem = ( ServerItem * )epoll_events[i].data.ptr;
                    pServerItem->SetEvents( events );
                    g_mutServerItem.lock();
                    g_TCPNetworkList.push_back( pServerItem );
                    g_mutServerItem.unlock();
                    g_condRW.notify_one();
                }
            }
        }

        return true;
    }

    bool MsgServer::Stop()
    {
        m_bService = false;
        m_ServerDetectThread.StopThread();

        if( m_hServerSocket != INVALID_SOCKET )
        {
            close( m_hServerSocket );
            m_hServerSocket = INVALID_SOCKET;
        }

        m_ServerAcceptThread.StopThread();
        m_AsynEngine.Stop();
        size_t nCount = m_ServerRWThreadArray.size();

        for( size_t i = 0; i < nCount; i++ )
        {
            ServerThreadReadWrite *pSocketThread = m_ServerRWThreadArray[i];
            assert( pSocketThread != NULL );
            pSocketThread->StopThread();
            delete pSocketThread;
        }

        m_ServerRWThreadArray.clear();
        ServerItem *pServerItem = NULL;

        for( size_t i = 0; i < m_NetworkItemActive.size(); i++ )
        {
            pServerItem = m_NetworkItemActive[i];
            std::unique_lock <std::mutex> lck( pServerItem->GetCriticalSection() );
            pServerItem->CloseSocket( pServerItem->GetRountID() );
        }

        m_NetworkItemFree.insert( m_NetworkItemFree.begin(), m_NetworkItemActive.begin(), m_NetworkItemActive.end() );
        m_NetworkItemActive.clear();
        m_TempNetworkItemArray.clear();
        return true;
    }

    bool MsgServer::SetNetServer( IServer *pObject )
    {
        m_pIServerEvent = pObject;

        if( m_pIServerEvent == NULL )
        {
            assert( false );
            return false;
        }

        return true;
    }

    ServerItem *MsgServer::ActiveNetworkItem()
    {
        std::unique_lock <std::mutex> lck( m_ItemLocked );
        ServerItem *pServerItem = NULL;
        if( m_NetworkItemFree.size() > 0 )
        {
            for( auto iter = m_NetworkItemFree.begin(); iter != m_NetworkItemFree.end(); ++iter )
            {
                pServerItem = *iter;
                m_NetworkItemFree.erase( iter );
                m_NetworkItemActive.push_back( pServerItem );
                return pServerItem;
            }
        }

        if( NULL == pServerItem )
        {
            unsigned short wStoreCount = m_NetworkItemStore.size();

            if( wStoreCount < m_wMaxConnect )
            {
                try
                {
                    pServerItem = new ServerItem( wStoreCount, this );

                    if( pServerItem == NULL )
                    {
                        assert( false );
                        return NULL;
                    }

                    m_NetworkItemActive.push_back( pServerItem );
                    m_NetworkItemStore.push_back( pServerItem );
                }
                catch( ... )
                {
                    assert( false );
                    return NULL;
                }
            }
        }

        return pServerItem;
    }

    ServerItem *MsgServer::GetNetworkItem( unsigned short wIndex )
    {
        std::unique_lock <std::mutex> lck( m_ItemLocked );
        assert( wIndex == 0 || wIndex < m_NetworkItemStore.size() );
        if( wIndex >= m_NetworkItemStore.size() ) return NULL;

        ServerItem *pServerItem = m_NetworkItemStore[wIndex];
        return pServerItem;
    }

    bool MsgServer::FreeNetworkItem( ServerItem *pServerItem )
    {
        std::unique_lock <std::mutex> lck( m_ItemLocked );
        assert( pServerItem );
        auto iterEnd = m_NetworkItemActive.end();

        for( auto iter = m_NetworkItemActive.begin(); iter != iterEnd; ++iter )
        {
            if( pServerItem == *iter )
            {
                m_NetworkItemActive.erase( iter );
                m_NetworkItemFree.push_back( pServerItem );
                return true;
            }
        }

        return false;
    }

    bool MsgServer::DetectSocket()
    {
        void *data = NULL;
        return m_AsynEngine.PostAsynData( Asyn_DETECT_SOCKET, data, 0 );
    }
}