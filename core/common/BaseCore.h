/*
*   BaseCore.h
*
*   Define some data structure and class interface.
*
*   Created on: 2018-11-07
*   Author:
*   All rights reserved.
*/
#ifndef __BaseCore_H__
#define __BaseCore_H__

#include <string>

#include "BaseDefine.h"
#include <utils/DataQueue.h>

using namespace std;

namespace Firefly
{
    extern thread_local std::string strThreadLogFlag;

    class ServerItem;
    class ClientItem;
	
	struct tagUserInfo
    {
        int                 nUserID;
        int                 nGameID;
        char                Accounts[32];
        char                NickName[32];
    };
    
    struct tagControl
    {
        unsigned short          wControlID;
        void                   *pData;
    };

    //timer
    struct tagTimer
    {
        unsigned int            unTimerID;
        unsigned int            unParam;
    };

    //DB
    struct tagDataBase
    {
        unsigned short          wRequestID;
        MsgHead		            MsgHeadInfo;
    };

    struct tagClientRead
    {
        unsigned int            wDataSize;
        unsigned int            nServerID;
        MsgHead		            MsgHeadInfo;
    };

    struct tagClientShut
    {
        unsigned int            nServerID;
        char                    cbShutReason;
    };

    struct tagClientLink
    {
        int                     nErrorCode;
        unsigned int            nServerID;
    };

    struct tagServerAccept
    {
        ServerItem* pServerItem;
    };

    struct tagServerRead
    {
        unsigned int            wDataSize;
        MsgHead             	MsgHeadInfo;
	    void                   	*pData;
    };

    struct tagServerShut
    {
	    void                   	*pData;
    };

    struct tagServerReady
    {
        unsigned int            dwSocketID;
    };

    class CDBResult
    {
    public:
        int             m_nErrorCode = 0;
        std::string     m_strErrorMsg = "";
        int             m_nUserData = -1;
    };

    class FFObject
    {
    public:
        FFObject() {}
        virtual ~FFObject() {}
    };
	
    class IUserItem : public FFObject
    {
    public:
        virtual tagUserInfo* GetUserInfo() = 0;
        virtual std::string GetAccounts() = 0;
    };

    class IUserManager : public FFObject
    {
    public:
        virtual IUserItem* EnumUserItem(int wEnumIndex) = 0;
        virtual IUserItem* SearchUserItem(int nUserID) = 0;
    public:
        virtual int GetUserItemCount() = 0;
        virtual int GetUserOnLine(unsigned char byClientType) = 0;
    public:
        virtual bool DeleteUserItem() = 0;
        virtual bool DeleteUserItem(IUserItem* pIUserItem) = 0;
        virtual bool InsertUserItem(IUserItem** pIServerUserResult, tagUserInfo& UserInfo) = 0;
    };

    class IServiceModule : public FFObject
    {
    public:
        virtual bool Start() = 0;
        virtual bool Stop() = 0;
    };

    class IBridgeHook : public FFObject
    {
    public:
        virtual bool OnBridgeStart( FFObject *pObject ) = 0;
        virtual bool OnBridgeStop( FFObject *pObject ) = 0;

    public:
        virtual bool OnEventControl( unsigned short wIdentifier, void *pData, unsigned int wDataSize ) = 0;
        virtual bool OnBridgeData( unsigned short wRequestID, void *pData, unsigned int wDataSize ) = 0;

    public:
        virtual bool OnTimer( unsigned int unTimerID, unsigned int unMsgID ) = 0;
        virtual bool OnDataBase( unsigned short wRequestID, MsgHead *pMsgHead, void *pData, unsigned int wDataSize ) = 0;

    public:
        virtual bool OnClientLink( unsigned int nSocketID, int nErrorCode ) = 0;
        virtual bool OnClientShut( unsigned int nSocketID, char cbShutReason ) = 0;
        virtual bool OnClientRead( unsigned int nServerID, MsgHead *pMsgHead, void *pData, unsigned int wDataSize ) = 0;

    public:
        virtual bool OnServerReady() = 0;
        virtual bool OnServerBind( ServerItem *pItem ) = 0;
        virtual bool OnServerShut( ServerItem *pItem ) = 0;
        virtual bool OnServerRead( ServerItem *pItem, MsgHead *pMsgHead, void *pData, unsigned int wDataSize ) = 0;
    };

    class IServer : public FFObject
    {
    public:
        virtual bool OnServerReady() = 0;
        virtual bool OnServerBind( ServerItem *pItem ) = 0;
        virtual bool OnServerShut( ServerItem *pServerItem ) = 0;
        virtual bool OnServerRead( ServerItem *pItem, MsgHead *pMsgHead, void *pData, unsigned int wDataSize ) = 0;
    };
    class IMsgServer;
    class IBridgeHook;

    class IBridge : public IServiceModule
    {
    public:
        virtual bool SetServer( IMsgServer *pObject ) = 0;
        virtual bool SetBridgeHook( IBridgeHook *pObject ) = 0;
    };

    class IMsgServer : public IServiceModule
    {
    public:
        virtual unsigned short GetServicePort() = 0;
        virtual int GetConnectType( unsigned int dwSocketID ) = 0;

    public:
        virtual bool SetNetServer( IServer *pObject ) = 0;
        virtual bool SetInfo( unsigned short wServicePort, unsigned short wMaxConnect ) = 0;

    public:
	    virtual bool SendData( MsgHead *pMsgHead ) = 0;
        virtual bool SendData( MsgHead *pMsgHead, void *pData, unsigned int wDataSize ) = 0;
        virtual bool SendDataEx( void *pData = NULL, unsigned int wDataSize = 0 ){return true;}

    public:
        virtual bool CloseSocket( unsigned int dwSocketID ) = 0;
        virtual bool ShutDown( unsigned int dwSocketID ) = 0;
    };

    class IAsynEngine : public IServiceModule
    {
    public:
        virtual bool SetAsynHook( FFObject *pObject ) = 0;
        virtual bool PostAsynData( unsigned short wIdentifier, void *pData, unsigned int wDataSize ) = 0;
    };

    class IAsynEngineHook : public FFObject
    {
    public:
        virtual bool OnAsynEngineStart() = 0;
        virtual bool OnAsynEngineStop() = 0;
        virtual bool OnAsynEngineData( unsigned short wIdentifier, void *pData, unsigned int wDataSize ) = 0;
    };

    class IDataBase : public FFObject
    {
    public:
        virtual void Open() = 0;
        virtual void Close() = 0;
        virtual bool SetInfo( const char *szDBAddr, unsigned short wPort, const char *szDBName, const char *szUser, const char *szPassword ) = 0;

    public:
        virtual bool MoveToNext( CDBResult &result ) = 0;
        virtual bool MoveToNextEx( CDBResult &result ) = 0;
        virtual bool IsRecordsetEnd() = 0;
	
    public:
        virtual CDBResult ExecuteProcess( const char *pszSPName ) = 0;
        virtual CDBResult ExecuteProcessEx( const char *pszSPName ) = 0;
        virtual void ConverToByte( char **pResult, size_t &nResultLen, const char *pSrcData, unsigned int nSrcLen ) = 0;
    };

    class IDBEngineHook;
    class IDBEngine : public IServiceModule
    {
    public:
        virtual bool SetDBEngineHook( IDBEngineHook *pObject, unsigned short wHookCount ) = 0;

    public:
        virtual bool PostDBControl( unsigned short wControlID, void *pData, unsigned int wDataSize ) = 0;
        virtual bool PostDBRequest( unsigned short wRequestID, MsgHead *pMsgHead, void *pData, unsigned int wDataSize ) = 0;
    };

    class IDBEngineHook : public FFObject
    {
    public:
        virtual bool OnDBEngineStart( IAsynEngineHook *pAsynEngineHook ) = 0;
        virtual bool OnDBEngineStop( IAsynEngineHook *pAsynEngineHook ) = 0;

    public:
        virtual bool OnDBEngineTimer( unsigned int unTimerID, unsigned int unParam ) = 0;
        virtual bool OnDBEngineControl( unsigned short wControlID, void *pData, unsigned int wDataSize ) = 0;
        virtual bool OnDBEngineRequest( unsigned short wRequestID, MsgHead *pMsgHead, void *pData, unsigned int wDataSize ) = 0;
    };

    class ITimerEvent : public FFObject
    {
    public:
        virtual bool OnTimer( unsigned int unTimerID, unsigned int unParam ) = 0;
    };

    class IClient : public FFObject
    {
    public:
        virtual bool OnClientLink( unsigned int nSocketID, int nErrorCode ) = 0;
        virtual bool OnClientShut( unsigned int nSocketID, char cbShutReason ) = 0;
        virtual bool OnClientRead( MsgHead *pMsgHead, void *pData, unsigned int wDataSize ) = 0;
    };

    class ITimer : public IServiceModule
    {
    public:
        virtual bool SetTimer( ITimerEvent *pObject ) = 0;
        virtual bool SetTimer( unsigned int unTimerID, unsigned int dwElapse, unsigned int dwRepeat, unsigned int unParam ) = 0;
        virtual bool KillTimer( unsigned int unTimerID ) = 0;
        virtual bool KillAllTimer() = 0;
    };

    class IMsgClient : public IServiceModule
    {
    public:
        virtual bool SetClientEvent( IClient *pObject ) = 0;

    public:
        virtual bool OnClientLink( unsigned int dwServerID, int nErrorCode ) = 0;
        virtual bool OnClientShut( unsigned int dwServerID, char cbShutReason ) = 0;
        virtual bool OnClientRead( MsgHead *pMsgHead, void *pData, unsigned int wDataSize ) = 0;

    public:
        virtual bool CloseSocket( unsigned int dwServerID ) = 0;
        virtual bool Connect( unsigned int dwServerID, unsigned int dwServerIP, unsigned short wPort ) = 0;
        virtual bool Connect( unsigned int dwServerID, const char *szServerIP, unsigned short wPort ) = 0;
        virtual bool SendData( unsigned int dwServerID, MsgHead *pMsgHead ) = 0;
        virtual bool SendData( unsigned int dwServerID, MsgHead *pMsgHead, void *pData, unsigned int wDataSize ) = 0;
    };

    class IDBEngineEvent : public FFObject
    {
    public:
        virtual bool OnDBResult( unsigned short wRequestID, MsgHead *pMsgHead, void *pData, unsigned int wDataSize ) = 0;
    };

}

#endif