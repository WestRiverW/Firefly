#ifndef __GameCore_H__
#define __GameCore_H__

//////////////////////////////////////////////////////////////////////////////////
#include <string.h>
#include "BaseCore.h"

namespace Firefly
{
    const static int    			CLIENT_KIND_UNKOWN = -1;
    const static unsigned short    	INVALID_TABLE = -1;
    const static unsigned short    	INVALID_CHAIR = -1;

    class ITable;
    class ITableHook;
    class IDataTransition;

	#define IDI_TABLE_MODULE_START      10000
	#define IDI_TABLE_MODULE_FINISH     50000

	#define TIME_TABLE_Hook_RANGE       30
	#define TIME_TABLE_MODULE_RANGE     50

    struct tagTableParameter
    {
        int                     nTableID;
        ITimer* pITimer;
        IDataTransition* pIDataTrans;
        IMsgClient* pIMsgClient;
    };

    class ITableUserAction : public FFObject
    {
    public:
        virtual bool OnUserOffLine(int wChairID, IUserItem* pIUserItem) = 0;
        virtual bool OnUserSitDown(int wChairID, IUserItem* pIUserItem, bool bLookonUser) = 0;
        virtual bool OnUserStandUp(int wChairID, IUserItem* pIUserItem, bool bLookonUser) = 0;
    };

    class ITable : public FFObject
    {
    public:
        virtual void init(ITableHook* TableHook, tagTableParameter& param) = 0;
    public:
        virtual int GetTableID() = 0;
        virtual int GetChairCount() = 0;
        virtual ITableHook* GetTableHook() = 0;

    public:
        virtual unsigned char GetStatus() = 0;
        virtual void SetStatus(unsigned char bStatus) = 0;

    public:
        virtual IUserItem* GetUserItem(int wChairID) = 0;

    public:
        virtual bool SetTimer(int unTimerID, int dwElapse, int dwRepeat, int unParam) = 0;
        virtual bool KillTimer(int unTimerID) = 0;

    public:
        virtual bool SendTableData(int wChairID, int wSubCmdID) = 0;
        virtual bool SendTableData(int wChairID, int wSubCmdID, void* pData, int wDataSize, int wMainCmdID) = 0;
        virtual bool PerformStandUpAction(IUserItem* pIUserItem) = 0;
        virtual bool PerformSitDownAction(int wChairID, IUserItem* pIUserItem, std::string lpszPassint = "") = 0;
        virtual bool OnUserOffLine(IUserItem* pIUserItem) = 0;

    public:
        virtual bool OnTimer(int unTimerID, int unParam) = 0;
        virtual bool OnEventSocketGame(MsgHead* pMsgHead, void* pData, int wDataSize, IUserItem* pIUserItem) = 0;
        virtual bool OnLoadCfgMessage(void* pData, int wDataSize, IUserItem* pIUserItem) = 0;
        virtual  IMsgClient* GetNetworkClient() = 0;
    };

    //////////////////////////////////////////////////////////////////////////////////
    class ITableHook : public FFObject
    {
    public:
        virtual void Reset() = 0;
        virtual bool Init(ITable* pITable) = 0;

    public:
        virtual bool OnGameStart() = 0;
        virtual bool OnGameStop(int wChairID, IUserItem* pIUserItem, unsigned char cbReason) = 0;

    public:
        virtual bool OnTimer(int unTimerID, int unParam) = 0;

    public:
        virtual bool OnGameMessage(MsgHead* pMsgHead, void* pData, int wDataSize, IUserItem* pIUserItem) = 0;
        virtual bool OnFrameMessage(int wSubCmdID, void* pData, int wDataSize, IUserItem* pIUserItem) = 0;
        virtual bool OnLoadCfgMessage(void* pData, int wDataSize, IUserItem* pIUserItem) = 0;
    };

    //////////////////////////////////////////////////////////////////////////////////
    class IDataTransition : public FFObject
    {
    public:
        virtual bool SendData(MsgHead* pMsgHead, void* pData, int wDataSize) = 0;
    };
}

#endif
