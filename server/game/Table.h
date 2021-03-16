/*
*   Table.h
*
*   
*
*   Created on: 2019-1-29
*   Author:
*   All rights reserved.
*/
#ifndef __Table_H__
#define __Table_H__

#include <map>
#include <common/GameCore.h>

using namespace std;
using namespace Firefly;

typedef IUserItem *CTableUserItemArray[500];

class Table : public ITable
{
public:
    Table();
    virtual ~Table();
public:
    virtual void init( ITableHook *TableHook, tagTableParameter &param );
public:
    virtual int GetTableID();
    virtual int GetChairCount();
    virtual ITableHook *GetTableHook()
    {
        return m_pITableHook;
    }

public:
    virtual unsigned char GetStatus();
    virtual void SetStatus( unsigned char bStatus );

public:
    virtual IUserItem *GetUserItem( int wChairID );

public:
    virtual bool SetTimer( int unTimerID, int dwElapse, int dwRepeat, int unParam );
    virtual bool KillTimer( int unTimerID );

public:
    virtual bool SendTableData( int wChairID, int wSubCmdID );
    virtual bool SendTableData( int wChairID, int wSubCmdID, void *pData, int nDataSize, int wMainCmdID );
    virtual bool PerformStandUpAction( IUserItem *pIUserItem );
    virtual bool PerformSitDownAction( int wChairID, IUserItem *pIUserItem, std::string lpszPassint = "" );

    bool OnUserOffLine( IUserItem *pIUserItem );
public:
    bool OnTimer( int unTimerID, int unParam );
    bool OnEventSocketGame( MsgHead *pMsgHead, void *pData, int nDataSize, IUserItem *pIUserItem);
    bool OnLoadCfgMessage( void *pData, int nDataSize, IUserItem *pIUserItem );
    virtual  IMsgClient *GetMsgClient();

protected:
    IUserManager             *m_pIServerUserManager;
    CTableUserItemArray             m_TableUserItemArray;
    int                             m_wChairCount;
    unsigned char                   m_cbStatus;
protected:
    ITableUserAction               *m_pITableUserAction;
protected:
    IMsgClient              *m_pIMsgClient;
    ITimer                   *m_pITimer;                
    ITableHook                *m_pITableHook;
    IDataTransition              *m_pIMainServiceFrame;
    unsigned int                    m_iTableID;
};

#endif