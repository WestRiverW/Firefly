#include "BridgeHook.h"
#include <memory.h>
#include <assert.h>
#include <glog/logging.h>
#include <external/json/cJSON.h>
#include <share/ServerCmd.h>
#include <share/CommonDefine.h>
#include "Launch.h"

BridgeHook::BridgeHook()
    : m_bCfgIsUpdated( false )
{
    m_pLuaConfig->init( "ServerConfig.lua" );
    m_nUpTime = 0;
    m_vecOnLine.clear();
}

BridgeHook::~BridgeHook()
{
}


bool BridgeHook::OnTimer( unsigned int unTimerID, unsigned int unMsgID )
{
    return true;
}

bool BridgeHook::OnClientLink( unsigned int nServerID, int nErrorCode )
{
    LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << "websocket link nServerID:" << nServerID << ",nErrorCode:" << nErrorCode;

    if( nErrorCode == 0 )
    {
        switch( nServerID )
        {
            case SERVE_TYPE_CENTER:
            {
                if( !m_bCfgIsUpdated )
                {
                    m_pContactCenter->PullCfgInfo( SERVE_TYPE_WEB);
                }
                else
                {
                    //m_pContactCenter->RegisterServer( m_ServerInfo );
                }

                break;
            }

            case SERVE_TYPE_DB:
            {
                //m_pIMsgClient->SendData( nServerID, CMD_CORE_BASE, SUB_CORE_HEART );
                break;
            }
        }
    }
    else
    {
        m_pTimer->SetTimer( CONNECT_TIMER, 3, 1, nServerID );
    }

    return true;
}

bool BridgeHook::OnClientShut( unsigned int nServerID, char cbShutReason )
{
    LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " websocket shut " << nServerID ;
    m_pTimer->SetTimer( CONNECT_TIMER, 3, 1, nServerID );
    return true;
}

bool BridgeHook::OnClientRead( unsigned int nServerID, MsgHead *pMsgHead, void *pData, unsigned int nDataSize )
{
    LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " websocket read mainid:" << pMsgHead->wMainCmdID << ",subid:" << pMsgHead->wSubCmdID << ",nDataSize:" << nDataSize;

    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool BridgeHook::OnServerReady()
{
    LOG( INFO ) << strThreadLogFlag << __FUNCTION__;

    if( m_bCfgIsUpdated )
    {
       // m_pContactCenter->RegisterServer( m_ServerInfo );
    }

    return true;
}

bool BridgeHook::OnServerBind( ServerItem *pItem )
{
    return true;
}

bool BridgeHook::OnServerShut( ServerItem *pItem )
{
    LOG( INFO ) << strThreadLogFlag << __FUNCTION__;

    if( pItem )
    {
        pItem->ResetData();
    }

    return true;
}

bool BridgeHook::OnServerRead(ServerItem *pItem, MsgHead *pMsgHead, void *pData, unsigned int nDataSize )
{
    LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " webread main cmd id:" << pMsgHead->wMainCmdID << ",sub Cmd id:" << pMsgHead->wSubCmdID << ",nDataSize:" << nDataSize;

   /* switch( pMsgHead->wMainCmdID )
    {
        case CMD_WEB_BASE:
        {
            return OnHttpRequest( pData, nDataSize );
        }
    }*/

    return true;
}

bool BridgeHook::OnHttpRequest( void *pData, unsigned int nDataSize )
{
    LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " OnHttpRequest" << pData;
    struct evhttp_request *req = ( struct evhttp_request * )( *( long long * )pData );
    const char *uri = evhttp_request_get_uri( req );
    char *decode_uri = NULL;

    if( uri != NULL )
    {
        decode_uri = evhttp_decode_uri( uri );
    }

    LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " decode_uri " << decode_uri;
    struct evbuffer *buf = evhttp_request_get_input_buffer( req );
    int dataLen = 0;

    if( buf != NULL )
    {
        dataLen = evbuffer_get_length( buf );
    }

    LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " dataLen " << dataLen;

    if( decode_uri == NULL && buf == NULL )
    {
        LOG( ERROR ) << strThreadLogFlag << __FUNCTION__ << " NULL DATA";
        return true;
    }

    std::string CurrerUrl = decode_uri;

    if( CurrerUrl == "/" )
    {
        LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " SUCC_CurrerUrl: " << CurrerUrl;
        std::string strJsonData;
        char *buffer = NULL;

        if( dataLen > 0 )
        {
            buffer = new char[dataLen + 1];
            memset( buffer, 0, dataLen + 1 );
            evbuffer_remove( buf, buffer, dataLen );
            strJsonData = buffer;
        }

        strJsonData = Utility::UrlDecode( strJsonData );
        LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " strJsonData " << strJsonData;
        cJSON *root = cJSON_Parse( strJsonData.c_str() );

        if( root )
        {
            LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " root exist !!!";
            //SendWebMsgToClient( root, req );
            cJSON_Delete( root );
        }
        else
        {
            LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " fail";
            cJSON *resp = cJSON_CreateObject();
            cJSON_AddItemToObject( resp, "Mark", cJSON_CreateString( "fail" ) );
            evbuffer *response = evbuffer_new();
            evbuffer_add_printf( response, "%s", cJSON_Print( resp ) );
            evhttp_send_reply( req, HTTP_OK, "OK", response );
            evbuffer_free( response );
            cJSON_Delete( resp );
        }

        if( buffer != NULL )
        {
            delete buffer;
        }
    }
    else
    {
        LOG( ERROR ) << strThreadLogFlag << __FUNCTION__ << " FAIL_CurrerUrl: " << CurrerUrl;
    }

    if( decode_uri != NULL )
    {
        delete decode_uri;
        decode_uri = NULL;
    }

    return true;
}
