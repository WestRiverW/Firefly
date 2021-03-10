#include "TableHook.h"
#include <ctime>
#include <cmath>
#include <glog/logging.h>
#include <utils/Utility.h>
#include <external/json/cJSON.h>

TableHook::TableHook()
{
    m_pITable = nullptr;
    LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " ---- construct addr:" << this;
    Reset();
}

TableHook::~TableHook()
{
    Reset();
}

void TableHook::Reset()
{
}

bool TableHook::Init( ITable *pITable )
{
    m_pITable = pITable;

    if( m_pITable == NULL )
    {
        LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " --------m_pITable == NULL" << this;
        return false;
    }
    return true;
}

bool TableHook::OnGameStart()
{
    return true;
}

bool TableHook::OnGameStop( int wChairID, IUserItem *pIUserItem, unsigned char cbReason )
{
    return true;
}

bool TableHook::OnTimer( int unTimerID, int unParam )
{
    LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << "---- TableHook----OnTimer--------" << unTimerID;

    return true;
}

bool TableHook::OnUserOffLine( int wChairID, IUserItem *pIUserItem )
{
    LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " ----OnUserOffLine---- ";

    if( pIUserItem != NULL )
    {
    }

    return true;
}

bool TableHook::OnUserSitDown( int wChairID, IUserItem *pIUserItem, bool bLookonUser )
{
    bool res = true;

    if( pIUserItem != NULL )
    {
    }

    return res;
}

bool TableHook::OnUserStandUp( int wChairID, IUserItem *pIUserItem, bool bLookonUser )
{
    LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " ------1--------OnUserStandUp----------- ";

    if( pIUserItem != NULL )
    {
    }

    return true;
}

bool TableHook::OnGameMessage( MsgHead *pMsgHead, void *pData, int wDataSize, IUserItem *pIUserItem)
{
    return true;
}

bool TableHook::OnFrameMessage( int wSubCmdID, void *pData, int wDataSize, IUserItem *pIUserItem )
{
    return true;
}
