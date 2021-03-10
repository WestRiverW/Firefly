/*
*   DBEngine.h
*
*   Data Process.
*
*   Created on: 2018-11-07
*   Author:
*   All rights reserved.
*/
#ifndef __DBEngine_H__
#define __DBEngine_H__

#include <string>
#include <vector>
#include <mutex>
#include <mysql.h>

#include <common/BaseCore.h>
#include <common/AsynEngine.h>
#include <utils/Timer.h>

namespace Firefly
{
    class DataBase : public IDataBase
    {
    public:
        DataBase();
        virtual ~DataBase();

    public:
        virtual void Open();
        virtual void Close();
        virtual bool SetInfo(const char* szDBAddr, unsigned short wPort, const char* szDBName, const char* szUser, const char* szPassword);

        virtual bool MoveToNext(CDBResult& result);
        virtual bool MoveToNextEx(CDBResult& result);
        virtual bool IsRecordsetEnd();

    public:
        virtual CDBResult ExecuteProcess(const char* pszSPName);
        virtual CDBResult ExecuteProcessEx(const char* pszSPName);
        std::vector<std::vector<std::string>>& GetResultRow();
        void ConverToByte(char** pResult, size_t& nResultLen, const char* pSrcData, unsigned int nSrcLen);
        void PingDataBase();

    private:
        MYSQL* m_pMySQL;
        MYSQL_RES* m_pMySQLRes;
        MYSQL_ROW       m_MySQLRow;
        int             m_nRowLen;
        std::vector<std::vector<std::string>>   m_vecResult;

        std::string     m_strDBAddr;
        unsigned short  m_wPort;
        std::string     m_strDBName;
        std::string     m_strUserName;
        std::string     m_strPwd;
    };

    class DBEngine : public IDBEngine, public IAsynEngineHook
    {
    public:
        DBEngine();
        virtual ~DBEngine();

    public:
        virtual bool Start();
        virtual bool Stop();
        void PingDataBase();

    public:
        virtual bool SetDBEngineHook(IDBEngineHook* pObject, unsigned short wHookCount);

    public:
        virtual bool PostDBControl(unsigned short wControlID, void* pData, unsigned int wDataSize);
        virtual bool PostDBRequest(unsigned short wRequestID, MsgHead* pMsgHead, void* pData, unsigned int wDataSize);
    public:
        virtual bool OnAsynEngineStart();
        virtual bool OnAsynEngineStop();
        virtual bool OnAsynEngineData(unsigned short wIdentifier, void* pData, unsigned int wDataSize);

    protected:
        std::mutex              m_CriticalLocker;
        AsynEngine      m_AsynEngine;

    protected:
        IDBEngineHook* m_pIDBEngineHook;
    protected:
        char             m_cbBuffer[ASYN_DATA_LEN];
    };
}
#endif