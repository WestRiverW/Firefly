#include <assert.h>
#include <memory.h>
#include <glog/logging.h>

#include "DBEngine.h"

namespace Firefly
{
    DataBase::DataBase()
        : m_pMySQL(NULL),
        m_pMySQLRes(NULL),
        m_nRowLen(0),
        m_strDBAddr(""),
        m_wPort(0),
        m_strDBName(""),
        m_strUserName(""),
        m_strPwd("")
    {
    }

    DataBase::~DataBase()
    {
        Close();
    }

    void DataBase::Open()
    {
        do
        {
            LOG(INFO) << strThreadLogFlag << __FUNCTION__;
            m_pMySQL = mysql_init(NULL);

            if (!m_pMySQL)
            {
                LOG(ERROR) << strThreadLogFlag << __FUNCTION__ << " init faild";
                break;
            }

            LOG(INFO) << strThreadLogFlag << __FUNCTION__ << " databaseconnect addr:" << m_strDBAddr.c_str() << " port:" << m_wPort << " dbname:" << m_strDBName.c_str() << " username:" << m_strUserName.c_str() << " pwd:" << m_strPwd.c_str();

            if (!mysql_real_connect(m_pMySQL, m_strDBAddr.c_str(), m_strUserName.c_str(), \
                m_strPwd.c_str(), m_strDBName.c_str(), m_wPort, NULL, 0))
            {
                LOG(ERROR) << strThreadLogFlag << __FUNCTION__ << " connect faild:" << mysql_error(m_pMySQL);
                break;
            }
            else
            {
                char value = 1;
                mysql_options(m_pMySQL, MYSQL_OPT_RECONNECT, (char*)&value);
                LOG(INFO) << strThreadLogFlag << __FUNCTION__ << " databaseconnect OK";
            }
            LOG(INFO) << strThreadLogFlag << __FUNCTION__ << " databaseconnect addr:2";
        } while (false);
    }

    void DataBase::Close()
    {
        if (m_pMySQL)
        {
            mysql_close(m_pMySQL);
        }
    }

    bool DataBase::SetInfo(const char* szDBAddr, unsigned short wPort, const char* szDBName, const char* szUser, const char* szPassword)
    {
        m_strDBAddr = szDBAddr;
        m_wPort = wPort;
        m_strDBName = szDBName;
        m_strUserName = szUser;
        m_strPwd = szPassword;
        return true;
    }

    FFDBResult DataBase::ExecuteProcess(const char* pszSPName)
    {
        FFDBResult result;
        m_nRowLen = 0;
        LOG(INFO) << strThreadLogFlag << __FUNCTION__ << " sql:" << pszSPName;
        //int mysql_query(MYSQL *mysql, const char *stmt_str)
        //Zero for success. Nonzero if an error occurred.
        mysql_query(m_pMySQL, "set names utf8");

        if (mysql_query(m_pMySQL, pszSPName) != 0)
        {
            result.m_nErrorCode = mysql_errno(m_pMySQL);
            result.m_strErrorMsg = mysql_error(m_pMySQL);
            LOG(ERROR) << strThreadLogFlag << __FUNCTION__ << " exe sql:" << pszSPName << " faild errono:" << result.m_nErrorCode << " erro msg:" << result.m_strErrorMsg;
            return result;
        }

        m_vecResult.clear();

        do
        {
            // LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " try to get result now size:" << m_vecResult.size();
            m_pMySQLRes = mysql_store_result(m_pMySQL);

            if (NULL == m_pMySQLRes)
            {
                int nCode = mysql_errno(m_pMySQL);

                if (0 != nCode)
                {
                    result.m_nErrorCode = nCode;
                    result.m_strErrorMsg = mysql_error(m_pMySQL);
                    LOG(ERROR) << strThreadLogFlag << __FUNCTION__ << " get result faild:" << nCode;
                }

                break;
            }

            while (MoveToNext(result));

            if (NULL != m_pMySQLRes)
            {
                mysql_free_result(m_pMySQLRes);
            }
        } while (!mysql_next_result(m_pMySQL));

        return result;
    }



    FFDBResult DataBase::ExecuteProcessEx(const char* pszSPName)
    {
        FFDBResult result;
        m_nRowLen = 0;
        LOG(INFO) << strThreadLogFlag << __FUNCTION__ << " sql:" << pszSPName;
        int len = strlen(pszSPName);

        if (mysql_real_query(m_pMySQL, pszSPName, len) != 0)
        {
            result.m_nErrorCode = mysql_errno(m_pMySQL);
            result.m_strErrorMsg = mysql_error(m_pMySQL);
            LOG(ERROR) << strThreadLogFlag << __FUNCTION__ << " exe sql:" << pszSPName << " faild errono:" << result.m_nErrorCode << " erro msg:" << result.m_strErrorMsg;
            return result;
        }

        m_vecResult.clear();

        do
        {
            //   LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " try to get result now size:" << m_vecResult.size();
            m_pMySQLRes = mysql_store_result(m_pMySQL);

            if (NULL == m_pMySQLRes)
            {
                int nCode = mysql_errno(m_pMySQL);

                if (0 != nCode)
                {
                    result.m_nErrorCode = nCode;
                    result.m_strErrorMsg = mysql_error(m_pMySQL);
                    LOG(ERROR) << strThreadLogFlag << __FUNCTION__ << " get result faild:" << nCode;
                }

                break;
            }

            while (MoveToNextEx(result));

            if (NULL != m_pMySQLRes)
            {
                mysql_free_result(m_pMySQLRes);
            }
        } while (!mysql_next_result(m_pMySQL));

        return result;
    }

    void DataBase::ConverToByte(char** pResult, size_t& nResultLen, const char* pSrcData, unsigned int nSrcLen)
    {
        nResultLen = mysql_real_escape_string(m_pMySQL, *pResult, pSrcData, nSrcLen);
        // LOG( ERROR ) << strThreadLogFlag << __FUNCTION__ << " nResultLen:" << nResultLen;
    }

    //
    bool DataBase::MoveToNext(FFDBResult& result)
    {
        int nRowIndex = m_vecResult.size();
        //A MYSQL_ROW structure for the next row.
        //NULL if there are no more rows to retrieve or if an error occurred.
        m_MySQLRow = mysql_fetch_row(m_pMySQLRes);

        if (m_MySQLRow == NULL)
        {
            return false;
        }

        m_nRowLen = mysql_num_fields(m_pMySQLRes);

        //  LOG(INFO)<<strThreadLogFlag << __FUNCTION__ <<" parse row result m_nRowLen:" <<m_nRowLen<<" nRowIndex:"<<nRowIndex;
        if (0 < m_nRowLen && ((nRowIndex + 1) > (int)m_vecResult.size()))
        {
            m_vecResult.push_back(std::vector<std::string>());
        }

        for (int i = 0; i < m_nRowLen; ++i)
        {
            if (m_MySQLRow[i])
            {
                m_vecResult[nRowIndex].push_back(m_MySQLRow[i]);
            }
            else
            {
                m_vecResult[nRowIndex].push_back("0");
            }

            //  LOG(INFO)<<strThreadLogFlag << __FUNCTION__ <<" index:"<<i << " valude:"<< m_MySQLRow[i];
        }

        //  LOG(INFO)<<strThreadLogFlag << __FUNCTION__ <<" row result m_nRowLen:"<<m_nRowLen<<" errno:"<<result.m_nErrorCode<<",errorMsg:"<<result.m_strErrorMsg<<",user data:" <<result.m_nUserData;
        return true;
    }



    bool DataBase::MoveToNextEx(FFDBResult& result)
    {
        int nRowIndex = m_vecResult.size();
        m_MySQLRow = mysql_fetch_row(m_pMySQLRes);

        if (m_MySQLRow == NULL)
        {
            return false;
        }

        m_nRowLen = mysql_num_fields(m_pMySQLRes);
        unsigned long* row_len = mysql_fetch_lengths(m_pMySQLRes);

        //  LOG(INFO)<<strThreadLogFlag << __FUNCTION__ <<" parse row result m_nRowLen:" <<m_nRowLen<<" nRowIndex:"<<nRowIndex;
        if (0 < m_nRowLen && ((nRowIndex + 1) > (int)m_vecResult.size()))
        {
            m_vecResult.push_back(std::vector<std::string>());
        }

        string tmp;

        for (int i = 0; i < m_nRowLen; ++i)
        {
            if (m_MySQLRow[i])
            {
                tmp.clear();
                tmp.assign(m_MySQLRow[i], row_len[i]);
                m_vecResult[nRowIndex].push_back(tmp);
            }
            else
            {
                m_vecResult[nRowIndex].push_back("0");
            }

            //  LOG(INFO)<<strThreadLogFlag << __FUNCTION__ <<" index:"<< i << " valude:"<< tmp;
        }

        //  LOG(INFO)<<strThreadLogFlag << __FUNCTION__ <<" row result m_nRowLen:"<<m_nRowLen<<" errno:"<<result.m_nErrorCode<<",errorMsg:"<<result.m_strErrorMsg<<",user data:" <<result.m_nUserData;
        return true;
    }

    bool DataBase::IsRecordsetEnd()
    {
        if (m_MySQLRow)
        {
            return true;
        }
        else
        {
            mysql_free_result(m_pMySQLRes);
            m_pMySQLRes = nullptr;
            return false;
        }
    }

    std::vector<std::vector<std::string>>& DataBase::GetResultRow()
    {
        /*
            LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " m_nRowSize:" << m_vecResult.size();

            for( size_t i = 0; i < m_vecResult.size(); ++i )
            {
                LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " row index:" << i << " size:" << m_vecResult[i].size();

                for( size_t j = 0; j < m_vecResult[i].size(); ++j )
                {
                    LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " index:" << j << " valude:" << m_vecResult[i][j];
                }
            }
        */
        return m_vecResult;
    }


    void DataBase::PingDataBase()
    {
        bool reinit = false;
        {
            if (m_pMySQL != NULL)
            {
                unsigned long olderId = mysql_thread_id(m_pMySQL);
                mysql_ping(m_pMySQL);
                unsigned long newId = mysql_thread_id(m_pMySQL);
                reinit = (olderId != newId);
            }

            if (reinit)
            {
                if (m_pMySQL != NULL)
                {
                    mysql_close(m_pMySQL);
                    m_pMySQL = NULL;
                }
            }
        }

        if (reinit)
        {
            Open();
        }

        //   LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " reinit:" << reinit;
    }

    //////////////////////////////////////////////////////////////////////////
    DBEngine::DBEngine()
    {
        m_pIDBEngineHook = NULL;
        memset(m_cbBuffer, 0, sizeof(m_cbBuffer));
    }

    DBEngine::~DBEngine()
    {
    }

    bool DBEngine::Start()
    {
        if (m_AsynEngine.SetAsynHook(dynamic_cast<IAsynEngineHook*>(this)) == false)
        {
            assert(false);
            return false;
        }

        if (m_AsynEngine.Start() == false)
        {
            assert(false);
            return false;
        }

        return true;
    }

    bool DBEngine::Stop()
    {
        m_AsynEngine.Stop();
        return true;
    }

    void DBEngine::PingDataBase()
    {
    }

    bool DBEngine::SetDBEngineHook(IDBEngineHook* pObject, unsigned short wHookCount)
    {
        assert(pObject != NULL);
        if (pObject == NULL) return false;
        assert(wHookCount > 0);
        if (wHookCount == 0) return false;

        m_pIDBEngineHook = dynamic_cast<IDBEngineHook*>(pObject);

        if (m_pIDBEngineHook == NULL)
        {
            assert(false);
            return false;
        }

        return true;
    }

    bool DBEngine::PostDBControl(unsigned short wControlID, void* pData, unsigned int wDataSize)
    {
        std::unique_lock<std::mutex> ThreadLock(m_CriticalLocker);
        tagControl* pControlEvent = (tagControl*)m_cbBuffer;
        pControlEvent->wControlID = wControlID;
        m_AsynEngine.PostAsynData(MSG_CONTROL, m_cbBuffer, sizeof(tagControl));
        return true;
    }

    bool DBEngine::PostDBRequest(unsigned short wRequestID, MsgHead* pMsgHead, void* pData, unsigned int wDataSize)
    {
        LOG(INFO) << strThreadLogFlag << __FUNCTION__ << " wRequestID:" << wRequestID << ",wDataSize:" << wDataSize;
        assert((wDataSize + sizeof(tagDataBase)) <= ASYN_DATA_LEN);
        if ((wDataSize + sizeof(tagDataBase)) > ASYN_DATA_LEN) return false;

        std::unique_lock<std::mutex> ThreadLock(m_CriticalLocker);
        tagDataBase* pDataBaseEvent = (tagDataBase*)m_cbBuffer;
        pDataBaseEvent->wRequestID = wRequestID;
        pDataBaseEvent->MsgHeadInfo = *pMsgHead;

        if (wDataSize > 0)
        {
            assert(pData != NULL);
            memcpy(m_cbBuffer + sizeof(tagDataBase), pData, wDataSize);
        }

        m_AsynEngine.PostAsynData(MSG_DATABASE, m_cbBuffer, sizeof(tagDataBase) + wDataSize);
        return true;
    }

    bool DBEngine::OnAsynEngineStart()
    {
        assert(m_pIDBEngineHook != NULL);
        if (m_pIDBEngineHook == NULL) return false;
        return m_pIDBEngineHook->OnDBEngineStart(dynamic_cast<IAsynEngineHook*>(this));
    }

    bool DBEngine::OnAsynEngineStop()
    {
        assert(m_pIDBEngineHook != NULL);
        if (m_pIDBEngineHook == NULL) return false;
        return m_pIDBEngineHook->OnDBEngineStop(dynamic_cast<IAsynEngineHook*>(this));
    }

    bool DBEngine::OnAsynEngineData(unsigned short wIdentifier, void* pData, unsigned int wDataSize)
    {
        switch (wIdentifier)
        {
        case MSG_TIMER:
        {
            assert(wDataSize == sizeof(tagTimer));
            if (wDataSize != sizeof(tagTimer)) return false;
            tagTimer* pTimerEvent = (tagTimer*)pData;
            try
            {
                assert(m_pIDBEngineHook != NULL);
                return m_pIDBEngineHook->OnDBEngineTimer(pTimerEvent->unTimerID, pTimerEvent->unParam);
            }
            catch (...)
            {
                assert(false);
                return false;
            }

            return true;
        }

        case MSG_CONTROL:
        {
            assert(wDataSize >= sizeof(tagControl));
            if (wDataSize < sizeof(tagControl)) return false;
            tagControl* pControlEvent = (tagControl*)pData;

            try
            {
                assert(m_pIDBEngineHook != NULL);
                m_pIDBEngineHook->OnDBEngineControl(pControlEvent->wControlID, pControlEvent + 1, wDataSize - sizeof(tagControl));
            }
            catch (...)
            {
                assert(false);
                return false;
            }

            return true;
        }

        case MSG_DATABASE:
        {
            assert(wDataSize >= sizeof(tagDataBase));
            if (wDataSize < sizeof(tagDataBase)) return false;

            tagDataBase* pDataBaseEvent = (tagDataBase*)pData;

            try
            {
                assert(m_pIDBEngineHook != NULL);
                return m_pIDBEngineHook->OnDBEngineRequest(pDataBaseEvent->wRequestID, &pDataBaseEvent->MsgHeadInfo,
                    pDataBaseEvent + 1, wDataSize - sizeof(tagDataBase));
            }
            catch (...)
            {
                assert(false);
                return false;
            }

            return true;
        }
        }

        assert(false);
        return false;
    }
}