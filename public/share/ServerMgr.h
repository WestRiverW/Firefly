#ifndef __SVR_REGISTER_H__
#define __SVR_REGISTER_H__

#include <net/MsgAssist.h>
#include <net/MsgServer.h>
#include <lua/LuaConfig.h>
#include <game.pb.h>
#include <common.pb.h>

using namespace Firefly;

struct ServerResigterData
{
    ServerItem             *pNetItem;
    pb::ServerInfo          ServerInfo;
};

class ServerMgr
{
public:
    ServerMgr();
    ~ServerMgr();
    
    bool Append( const pb::ServerInfo &info, ServerItem *pNetItem );
    bool Erase( ServerItem *pNetItem );
    ServerItem *Find( int nMainType, int nServerID );
    pb::ServerInfo *GetConfig( int nMainType );

public:
    bool ParseConfig(LuaConfig* pLuaConfig);

private:
    std::map<int, pb::ServerInfo>                       m_mapConfigInfo;
    std::map<int, map<int, ServerResigterData>>         m_mapServerInfo;      
};

#endif