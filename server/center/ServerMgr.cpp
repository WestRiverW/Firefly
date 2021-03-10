#include "ServerMgr.h"
#include <glog/logging.h>
#include <lua/LuaConfig.h>
#include <external/json/cJSON.h>
#include <share/CommonDefine.h>

ServerMgr::ServerMgr()
{
}

ServerMgr::~ServerMgr()
{
}

bool ServerMgr::ParseConfig(LuaConfig* pLuaConfig)
{
    string jsonServerConfig = pLuaConfig->getConfigByFun( "ServerConfig" );
    cJSON *jsRoot = cJSON_Parse( jsonServerConfig.c_str() );

    if( jsRoot == NULL )
    {
        return false;
    }
    cJSON *jsServerConfigArray = cJSON_GetObjectItem( jsRoot, "configs" );
    if( jsServerConfigArray == NULL )
    {
        return false;
    }

    if( jsServerConfigArray->type != cJSON_Array )
    {
        return false;
    }

    int arraySize = cJSON_GetArraySize( jsServerConfigArray );

    for( int i = 0; i < arraySize; ++i )
    {
        cJSON *jsServerCfgItem = cJSON_GetArrayItem( jsServerConfigArray, i );
        pb::ServerInfo serverInfo;
        cJSON *temp = cJSON_GetObjectItem( jsServerCfgItem, "server" );

        int nMainType = temp->valueint;
        if( temp != NULL )
        {
            serverInfo.set_maintype(nMainType);
        }
        
        temp = cJSON_GetObjectItem( jsServerCfgItem, "startport" );
        if( temp != NULL )
        {
            serverInfo.set_port( temp->valueint );
        }

        temp = cJSON_GetObjectItem( jsServerCfgItem, "maxconnection" );
        if( temp != NULL )
        {
            serverInfo.set_maxconnect( temp->valueint );
        }
        m_mapConfigInfo[nMainType] = serverInfo;
    }

    cJSON_Delete( jsRoot );
    jsRoot = NULL;
    return true;
}

bool ServerMgr::Append(const pb::ServerInfo& info, ServerItem* pNetItem)
{
    bool bSuccess = false;
    auto iterMainType = m_mapServerInfo.find( info.maintype() );

    if( iterMainType == m_mapServerInfo.end() )
	{
        bSuccess = true;
    }
	else
    {
        auto iterSubType = iterMainType->second.find( info.serverid() );
        if( iterSubType == iterMainType->second.end() )
        {
            bSuccess = true;
        }
    }
	if(bSuccess)
	{
        ServerResigterData data;
        data.pNetItem = pNetItem;
        data.ServerInfo = info;
        m_mapServerInfo[info.maintype()][info.serverid()] = data;
	}
    return bSuccess;
}

bool ServerMgr::Erase( ServerItem *pNetItem )
{
    auto iterEnd = m_mapServerInfo.end();
    for (auto iter = m_mapServerInfo.begin(); iter != iterEnd; ++iter)
    {
        for (auto it = iter->second.begin(); it != iter->second.end(); ++it)
        {
            if (it->second.pNetItem == pNetItem)
            {
                iter->second.erase(it);
                if (0 == iter->second.size())
                {
                    m_mapServerInfo.erase(iter);
                }
                return true;
            }
        }
    }
    return false;
}

ServerItem *ServerMgr::Find(int nMainType, int nServerID)
{
    ServerItem *pItem = NULL;
	auto iterMainType = m_mapServerInfo.find( nMainType );

	if( iterMainType != m_mapServerInfo.end() )
	{
		auto iterSubType = iterMainType->second.find(nServerID);

		if( iterSubType != iterMainType->second.end() )
		{
			pItem = iterSubType->second.pNetItem;
		}
	}

    return pItem;
}

pb::ServerInfo* ServerMgr::GetConfig( int nMainType )
{
	auto iterMainType = m_mapConfigInfo.find( nMainType );

	if( iterMainType != m_mapConfigInfo.end() )
	{
		return &( iterMainType->second );
	}
    return nullptr;
}