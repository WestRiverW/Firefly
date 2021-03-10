#include "HttpServer.h"
#include <assert.h>
#include <glog/logging.h>

IBridgeHook *HttpServer::m_pIBridgeHook = NULL;

HttpServer::HttpServer()
{
}

HttpServer::~HttpServer()
{
    if( m_pBase != NULL )
    {
        evhttp_del_accept_socket( m_pHttp, m_pSocketHandle );
        event_base_loopexit( m_pBase, NULL );
        evhttp_free( m_pHttp );
        m_pHttp = NULL;
        event_base_free( m_pBase );
        m_pBase = NULL;
    }
}

bool HttpServer::InitThread( IBridgeHook *pIBridgeHook, unsigned short port )
{
    m_pIBridgeHook = pIBridgeHook;
    m_usPort = port;
    return true;
}

bool HttpServer::OnRun()
{
    m_pBase = event_base_new();

    if( !m_pBase )
    {
        LOG( ERROR ) << strThreadLogFlag << __FUNCTION__ << "Couldn't create an event_base: exiting";
        return 1;
    }

    /* Create a new evhttp object to handle requests. */
    m_pHttp = evhttp_new( m_pBase );

    if( !m_pHttp )
    {
        LOG( ERROR ) << strThreadLogFlag << __FUNCTION__ << "couldn't create evhttp. Exiting.";
        return 1;
    }

    /* The /dump URI will dump all requests to stdout and say 200 ok. */
    //evhttp_set_cb(http, "/dump", dump_request_cb, NULL);
    /* We want to accept arbitrary requests, so we need to set a "generic"
    * cb.  We can also add callbacks for specific paths. */
    LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << "HTTP Port " << m_usPort;
    evhttp_set_gencb( m_pHttp, &HttpServer::HandleHttpRequest, NULL );
    /* Now we tell the evhttp what port to listen on */
    m_pSocketHandle = evhttp_bind_socket_with_handle( m_pHttp, "0.0.0.0", m_usPort );

    if( !m_pSocketHandle )
    {
        LOG( ERROR ) << strThreadLogFlag << __FUNCTION__ << "couldn't create evhttp. Exiting.";
        return false;
    }

    try
    {
        LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << "HTTP Port " << m_usPort;
        event_base_dispatch( m_pBase );
    }
    catch( ... )
    {
        assert( false );
        return false;
    }

    return true;
}

void HttpServer::HandleHttpRequest( struct evhttp_request *req, void *arg )
{
    MsgHead msgHeadInfo;
    msgHeadInfo.wMainCmdID = CMD_WEB_BASE;
    msgHeadInfo.wSubCmdID = 0;
    long long ptr = ( long long )req;
    LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << "HTTP AAAAAAA";
    m_pIBridgeHook->OnServerRead( nullptr, &msgHeadInfo, &ptr, sizeof( ptr ));
}