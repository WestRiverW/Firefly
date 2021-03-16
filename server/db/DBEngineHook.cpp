#include "DBEngineHook.h"
#include <net/MsgAssist.h>
#include <utils/StrTool.h>

std::string             strLostCfg;
std::vector<int>        vecLostBase;
std::vector<double>     vecLostRate;

DBEngineHook::DBEngineHook()
{
    //m_pLuaConfig->init( "ServerConfig.lua" );
}

DBEngineHook::~DBEngineHook()
{
}

bool DBEngineHook::OnDBEngineStart( IAsynEngineHook *pAsynEngineHook )
{
    /*std::string strDBAddr = m_pLuaConfig->getConfigByFun( "GetServerConfig", "dbip" );
    unsigned short sPort = atoi( m_pLuaConfig->getConfigByFun( "GetServerConfig", "dbport" ).c_str() );
    std::string strDBUser = m_pLuaConfig->getConfigByFun( "GetServerConfig", "dbuser" );
    std::string strDBPwd = m_pLuaConfig->getConfigByFun( "GetServerConfig", "dbpwd" );
    m_AccountsDB.SetInfo( strDBAddr.c_str(), sPort, "accounts", strDBUser.c_str(), strDBPwd.c_str() );
    m_AccountsDB.Open();
    m_TreasureDB.SetInfo( strDBAddr.c_str(), sPort, "treasure", strDBUser.c_str(), strDBPwd.c_str() );
    m_TreasureDB.Open();
    m_RecordDB.SetInfo( strDBAddr.c_str(), sPort, "record", strDBUser.c_str(), strDBPwd.c_str() );
    m_RecordDB.Open();
    m_PlatformDB.SetInfo( strDBAddr.c_str(), sPort, "platform", strDBUser.c_str(), strDBPwd.c_str() );
    m_PlatformDB.Open();
    m_Timer->SetTimer( PING_DATA_BASE_TIMER, 5, 1, 0 );*/
    return true;
}

bool DBEngineHook::OnDBEngineStop( IAsynEngineHook *pAsynEngineHook )
{
    return true;
}

bool DBEngineHook::OnDBEngineTimer( unsigned int unTimerID, unsigned int unParam )
{
    return true;
}

bool DBEngineHook::OnDBEngineControl( unsigned short wControlID, void *pData, unsigned int nDataSize )
{
    LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " eventID:" << wControlID << ",nDataSize:" << nDataSize;

    if( CUSTOMIZE_EVENT_PING_DB == wControlID )
    {
    }
    else
    {
    }

    return true;
}

bool DBEngineHook::OnDBEngineRequest( unsigned short wRequestID, MsgHead *pMsgHead, void *pData, unsigned int nDataSize )
{
    LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " wRequestID:" << wRequestID << ",nDataSize:" << nDataSize;

    return true;
}

inline void DBEngineHook::DoMysqlQuery( DataBase *pDataBase, FFDBResult &result, std::vector<std::vector<std::string>> &rows, std::vector<std::string> &row, const char *strProcess, const char *strSqlRet )
{
    LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " process sql:" << strProcess;
    result = pDataBase->ExecuteProcess( strProcess );
    rows = pDataBase->GetResultRow();

    if( rows.size() > 0 )
    {
        row = rows[0];
    }
    else
    {
        if( 0 == result.m_nErrorCode )
        {
            if( strSqlRet && strlen( strSqlRet ) > 0 )
            {
                result = pDataBase->ExecuteProcess( strSqlRet );
                rows = pDataBase->GetResultRow();

                if( rows.size() > 0 )
                {
                    row = rows[0];
                }
            }
            else
            {
                row.push_back( "0" );
                row.push_back( "" );
                rows.push_back( row );
            }
        }
    }

    LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " errno:" << result.m_nErrorCode << ",errorMsg:" << result.m_strErrorMsg << " row size:" << row.size();
}

inline void DBEngineHook::DoMysqlQueryEx( DataBase *pDataBase, FFDBResult &result, std::vector<std::vector<std::string>> &rows, std::vector<std::string> &row, const char *strProcess, const char *strSqlRet )
{
    LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " process sql:" << strProcess;
    result = pDataBase->ExecuteProcessEx( strProcess );
    rows = pDataBase->GetResultRow();

    if( rows.size() > 0 )
    {
        row = rows[0];
    }
    else
    {
        if( 0 == result.m_nErrorCode )
        {
            if( strSqlRet && strlen( strSqlRet ) > 0 )
            {
                result = pDataBase->ExecuteProcessEx( strSqlRet );
                rows = pDataBase->GetResultRow();

                if( rows.size() > 0 )
                {
                    row = rows[0];
                }
            }
            else
            {
                row.push_back( "0" );
                row.push_back( "" );
                rows.push_back( row );
            }
        }
    }

    LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " errno:" << result.m_nErrorCode << ",errorMsg:" << result.m_strErrorMsg << " row size:" << row.size();
}

bool DBEngineHook::OnMobileLogonAccounts( MsgHead *pMsgHead, void *pData, unsigned int nDataSize )
{
    return true;
}

bool DBEngineHook::OnMobileLeaveAccounts( MsgHead *pMsgHead, void *pData, unsigned int nDataSize )
{
    return true;
}

bool DBEngineHook::OnMBRegister( MsgHead *pMsgHead, void *pData, unsigned int nDataSize )
{
    return true;
}
