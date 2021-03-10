#ifndef __CenterLogic_H__
#define __CenterLogic_H__

#include <utils/Timer.h>
#include <net/MsgServer.h>
#include <common/BaseCore.h>
#include "../../pb/common.pb.h"

using namespace Firefly;

class ContactCenter
{
public:
    ContactCenter(){}
    ~ContactCenter(){}

    void SetMsgClient( IMsgClient *socketService );
    void Connect( unsigned int dwServerID, const char *szServerIP, unsigned short wPort );
    void Register( pb::ServerInfo &cfgInfo);
    void PullCfgInfo( int nMainType);
    /*void PullServerInfo( int nMainType );*/
protected:
    IMsgClient      *m_pIMsgClient;
};

#endif