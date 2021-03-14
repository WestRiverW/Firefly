/*
*   DBEngineHook.h
*
*   
*
*   Created on: 2018-11-13
*   Author:
*   All rights reserved.
*/
#ifndef __DBEngineHook_H__
#define __DBEngineHook_H__

#include <glog/logging.h>
#include <lua/LuaConfig.h>
#include <utils/Timer.h>
#include <data/DBEngine.h>
#include <common/BaseCore.h>
#include <share/ServerCmd.h>
#include <share/CommonDefine.h>
#include <db.pb.h>

using namespace Firefly;

struct tagUserLogin
{
    int     nUserID;
    int     nChannelID;
    int     nSockItemID;
    int     nClientPort;
    string  strClientIP;
    string  strAccounts;
};

typedef std::map<int, tagUserLogin> MapUserLogin;

class DBEngineHook : public IDBEngineHook
{
    friend class Launch;

protected:
    LuaConfig              m_cfg;
    DataBase                m_RecordDB;
    DataBase                m_AccountsDB;
    DataBase                m_TreasureDB;
    DataBase                m_PlatformDB;

    Timer            *m_Timer;
    IDBEngine   *m_pIDBEngineEvent;
public:
    DBEngineHook();
    virtual ~DBEngineHook();

public:
    virtual bool OnDBEngineStart( IAsynEngineHook *pAsynEngineHook );
    virtual bool OnDBEngineStop( IAsynEngineHook *pAsynEngineHook );

public:
    virtual bool OnDBEngineTimer( unsigned int unTimerID, unsigned int unParam );
    virtual bool OnDBEngineControl( unsigned short wControlID, void *pData, unsigned int wDataSize );
    virtual bool OnDBEngineRequest( unsigned short wRequestID, MsgHead *pMsgHead, void *pData, unsigned int wDataSize );

protected:
    inline void DoMysqlQuery( DataBase *pDataBase, FFDBResult &result, std::vector<std::vector<std::string>> &rows, std::vector<std::string> &row, const char *strProcess, const char *strSqlRet );
    inline void DoMysqlQueryEx( DataBase *pDataBase, FFDBResult &result, std::vector<std::vector<std::string>> &rows, std::vector<std::string> &row, const char *strProcess, const char *strSqlRet );

protected:
    bool OnMobileLogonAccounts( MsgHead *pMsgHead, void *pData, unsigned int wDataSize );
    bool OnMobileLeaveAccounts( MsgHead *pMsgHead, void *pData, unsigned int wDataSize );
    bool OnMBRegister( MsgHead *pMsgHead, void *pData, unsigned int wDataSize );
};

#endif
