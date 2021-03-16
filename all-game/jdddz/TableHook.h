/*
*   TableHook.h
*
*   
*
*   Created on: 2019-1-29
*   Author:
*   All rights reserved.
*/
#ifndef __TableHook_H__
#define __TableHook_H__

#include <common/GameCore.h>

using namespace std;
using namespace Firefly;

class TableHook : public ITableHook, public ITableUserAction
{
public:
    TableHook();
    virtual ~TableHook();
public:
    virtual void Reset();
    virtual bool Init( ITable *pITable );

public:
    virtual bool OnGameStart();
    virtual bool OnGameStop( int wChairID, IUserItem *pIUserItem, unsigned char cbReason );

public:
    virtual bool OnTimer( int unTimerID, int unParam );
public:
    virtual bool OnUserOffLine( int wChairID, IUserItem *pIUserItem );
    virtual bool OnUserSitDown( int wChairID, IUserItem *pIUserItem, bool bLookonUser );
    virtual bool OnUserStandUp( int wChairID, IUserItem *pIUserItem, bool bLookonUser );

public:
    virtual bool OnGameMessage( MsgHead *pMsgHead, void *pData, int nDataSize, IUserItem *pIUserItem);

public:
    ITable  		*m_pITable;
};

#endif