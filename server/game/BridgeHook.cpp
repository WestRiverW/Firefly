#include <dlfcn.h>
#include <memory.h>
#include <assert.h>
#include <glog/logging.h>

#include <net/MsgAssist.h>
#include <utils/StrTool.h>
#include <utils/Utility.h>
#include <net/MsgServer.h>
#include <share/ServerCmd.h>
#include <share/CommonDefine.h>
#include <share/GameFactory.h>
#include <common.pb.h>
#include "Launch.h"
#include "BridgeHook.h"

#define  	TABLE_INIT_COUNT		30
#define		TABLE_MAX_COUNT			500

BridgeHook::BridgeHook()
    : m_bCfgIsUpdated( false )
{
    m_nLoadSoNumber = 1;
}

BridgeHook::~BridgeHook()
{
    for( size_t i = 0; i < m_vecModuleHandle.size(); ++i )
    {
        dlclose( m_vecModuleHandle[i] );
    }
}

bool BridgeHook::OnEventControl( unsigned short wIdentifier, void *pData, unsigned int wDataSize )
{
    if( CUSTOMIZE_EVENT_CONNECT == wIdentifier )
    {
        /*protocol::ConnetCtrInfo msg;
        if( msg.ParseFromArray( ( char * )pData, wDataSize ) )
        {
            int unMsgID = msg.nsocketid();
            if( SERVE_TYPE_DB == unMsgID )
            {
                LOG( INFO ) << __FUNCTION__ << " eventcontrol ip=" << m_ServerDB.szip().c_str() << " port=" << m_ServerDB.nport();
                m_pIMsgClient->Connect( unMsgID, m_ServerDB.szip().c_str(), ( unsigned short )m_ServerDB.nport() );
            }
        }*/
    }

    return true;
}

bool BridgeHook::OnTimer( unsigned int unTimerID, unsigned int nBindParam )
{
    if( ( unTimerID >= IDI_TABLE_MODULE_START ) && ( unTimerID <= IDI_TABLE_MODULE_FINISH ) )
    {
        int dwTableTimerID = unTimerID - IDI_TABLE_MODULE_START;
        std::map<int, ITable *>::iterator iter = m_mapGameType2Table.find( nBindParam );

        if( iter != m_mapGameType2Table.end() )
        {
            iter->second->OnTimer( dwTableTimerID % TIME_TABLE_MODULE_RANGE, ( int )nBindParam );
        }
    }

    switch( unTimerID )
    {
        case CONNECT_TIMER:
        {
            switch( nBindParam )
            {
                case SERVE_TYPE_CENTER:
                {
                    m_pContactCenter->Connect( nBindParam, m_strCenterIP.c_str(), m_usCenterPort );
                    break;
                }

                case SERVE_TYPE_DB:
                {
                    //m_pContactCenter->Connect( nBindParam, m_ServerDB.szip().c_str(), ( unsigned short )m_ServerDB.nport() );
                    break;
                }
            }
            break;
        }
        default:
            break;
    }

    return true;
}

bool BridgeHook::OnClientLink( unsigned int nServerID, int nErrorCode )
{
    LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " gamesocket link nServerID:" << nServerID << ",nErrorCode:" << nErrorCode;
    if( nErrorCode == 0 )
    {
        switch( nServerID )
        {
            case SERVE_TYPE_CENTER:
            {
                if( !m_bCfgIsUpdated )
                {
                    m_pContactCenter->PullCfgInfo( SERVE_TYPE_GAME);
                }
                else
                {
                    //m_pContactCenter->Register( m_ServerInfo);
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
    LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " gamesocketshut " << nServerID;
    m_pTimer->SetTimer( CONNECT_TIMER, 3, 1, nServerID );
    return true;
}

bool BridgeHook::OnClientRead( unsigned int nServerID, MsgHead *pMsgHead, void *pData, unsigned int wDataSize )
{
    LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " gamesocket read cmd id:" << pMsgHead->wMainCmdID << ",sub Cmd id:" << pMsgHead->wSubCmdID << ",wDataSize:" << wDataSize;

    return true;
}

bool BridgeHook::OnClientCenterRegResp( void *pData, unsigned int wDataSize )
{
    return true;
}

////////////////////////////////////////////////////////////////////////////////////

bool BridgeHook::OnServerReady()
{
    OnServerReadyDelay();
    //m_pContactCenter->RegisterServer( m_ServerInfo, &m_SubGameIDList );
    //m_pContactCenter->PullServerInfo( SERVE_TYPE_DB );
    return true;
}
void BridgeHook::SetLoadSoNumber( int  nLoadNumber )
{
    m_nLoadSoNumber = nLoadNumber;
}

bool BridgeHook::OnServerReadyDelay()
{
    //LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " serverregisterstart111 3:" << m_bCfgIsUpdated;

    //if( m_bCfgIsUpdated )
    //{
    //    LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " -----m_nLoadSoNumber:" << m_nLoadSoNumber;
    //    std::string strSubGame = m_pLuaConfig->getSubGame( "GetSubGame", "subGame", m_nLoadSoNumber );
    //    LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " subGameParam:" << strSubGame;
    //    std::vector<std::string> vecGameModuleItems;
    //    //strtool::split( strSubGame, vecGameModuleItems, "|" );

    //    for( size_t i = 0; i < vecGameModuleItems.size(); ++i )
    //    {
    //        char *error = NULL;
    //        void *module_handle = dlopen( vecGameModuleItems[i].c_str(), RTLD_LAZY );

    //        if( module_handle == NULL )
    //        {
    //            error = dlerror();
    //            return false;
    //        }
    //        else
    //        {
    //            dlerror();
    //            int ( *create_func )( ITableHook *&, int );
    //            create_func = ( int ( * )( ITableHook *&, int ) )dlsym( module_handle, "CreatObj" );

    //            if( ( error = dlerror() ) != NULL )
    //            {
    //                return false;
    //            }

    //            ITableHook *tableHook;
    //            int nGameID = ( *create_func )( tableHook, 0 );
    //            LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " subGameParam:" << vecGameModuleItems[i].c_str() << " gameid:" << nGameID;

    //            if( NULL != tableHook )
    //            {
				//	for(int i = 0; i < TABLE_INIT_COUNT; ++i)
				//	{
				//		Table *pTable = ActiveTable(i);
				//		tagTableParameter param;
				//		param.nTableID = i;
				//		param.pITimer = m_pTimer;
				//		param.pIDataTrans = this;
				//		param.pIMsgClient = m_pIMsgClient;
				//		pTable->init( tableHook, param );
				//	}
    //            }

    //            m_vecModuleHandle.push_back( module_handle );
    //        }
    //    }
    //}

    return true;
}

bool BridgeHook::OnServerShut( ServerItem *pItem )
{
    if( pItem )
    {
        pItem->ResetData();
    }

    return true;
}

bool BridgeHook::OnServerRead(ServerItem *pItem, MsgHead *pMsgHead, void *pData, unsigned int wDataSize )
{
    LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " gameread cmd id:" << pMsgHead->wMainCmdID << ",sub Cmd id:" << pMsgHead->wSubCmdID << ",wDataSize:" << wDataSize;

    switch( pMsgHead->wMainCmdID )
    {
        case CMD_HALL_BASE:
        {
            return OnSubNetworkHallMsg( pMsgHead, pData, wDataSize );
        }

        case CMD_GAME_BASE:
        {
            return OnSubNetworkGameMsg( pMsgHead, pData, wDataSize );
        }
    }

    return true;
}

bool BridgeHook::OnSubNetworkHallMsg( MsgHead *pMsgHead, void *pData, unsigned int wDataSize )
{
    return true;
}

bool BridgeHook::OnSubNetworkGameMsg( MsgHead *pMsgHead, void *pData, unsigned int wDataSize )
{
    return true;
}

void BridgeHook::OnLoginResp( void *pData, unsigned int wDataSize )
{
}

bool BridgeHook::SendData( MsgHead *pMsgHead, void *pData, int wDataSize )
{
    // auto nGateID = m_UserManager.SearchUserGate( cbSendMask );

    // if( nGateID >= 0 )
    // {
        // m_pIMsgServer->SendData( nGateID, wMainCmdID, wSubCmdID, pData, wDataSize );
    // }
    // else if( cbSendMask == 0 )
    // {
        // m_pIMsgServer->SendDataEx( wMainCmdID, wSubCmdID, pData, wDataSize );
    // }
    // else
    // {
        // LOG( ERROR ) << strThreadLogFlag << __FUNCTION__ << " BridgeHook::SendData11: " << cbSendMask;
    // }

    return true;
}

Table * BridgeHook::ActiveTable(int nTableID)
{
	Table * pTable = NULL;
	if (m_mapTableFree.size() > 0)
	{
		auto iter = m_mapTableFree.begin();
		pTable = iter->second;
		assert(pTable != NULL);
		m_mapTableFree.erase(iter);
		m_mapTableActive[nTableID] = pTable;
	}

	if (pTable == NULL)
	{
		size_t wStorageCount = m_mapTableStore.size();
		if (wStorageCount < TABLE_MAX_COUNT)
		{
			try
			{
				pTable = new Table();
				if (pTable == NULL) return NULL;
				m_mapTableStore[nTableID] = pTable;
				m_mapTableActive[nTableID] = pTable;
			}
			catch (...)
			{
				return NULL;
			}
		}
	}

	return pTable;
}

Table * BridgeHook::GetTable(int nTableID)
{
	if (m_mapTableStore.find(nTableID) != m_mapTableStore.end())
	{
		Table * pTable = m_mapTableStore[nTableID];
		assert(pTable != NULL);
		return pTable;
	}
	return NULL;
}

bool BridgeHook::FreeTable(int nTableID)
{
	assert(nTableID < TABLE_MAX_COUNT);

	auto iter = m_mapTableActive.find(nTableID);
	if(iter != m_mapTableActive.end())
	{
		m_mapTableActive.erase(iter);
		m_mapTableFree[nTableID] = iter->second;
		return true;
	}

	assert(false);
	return false;
}

//////////////////////////////////////////////////////////////////////////
