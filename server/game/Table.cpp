#include "Table.h"
#include <assert.h>
#include <glog/logging.h>


Table::Table()
{
    m_pITimer = NULL;
    m_iTableID = 0;
    m_wChairCount = 10;
    memset( m_TableUserItemArray, 0, sizeof( m_TableUserItemArray ) );
}

Table::~Table()
{
}

void Table::init( ITableHook *TableHook, tagTableParameter &param )
{
    m_pITableUserAction = dynamic_cast<ITableUserAction *>( TableHook );
    m_pITableHook = TableHook;
    m_iTableID = param.nTableID;
    m_pITimer = param.pITimer;
    m_pIMsgClient = param.pIMsgClient;
    m_pIMainServiceFrame = param.pIDataTrans;
    return ;
}

int Table::GetTableID()
{
    return m_iTableID;
}

int Table::GetChairCount()
{
    return 1;
}

unsigned char Table::GetStatus()
{
    return m_cbStatus;
}

void Table::SetStatus( unsigned char bStatus )
{
    m_cbStatus = bStatus;
}

IUserItem *Table::GetUserItem( int wChairID )
{
    if( wChairID >= m_wChairCount ) return NULL;
    return m_TableUserItemArray[wChairID];
}

bool Table::SetTimer( int unTimerID, int dwElapse, int dwRepeat, int unParam )
{
    return true;
}

bool Table::KillTimer( int unTimerID )
{
    return true;
}

bool Table::SendTableData( int wChairID, int wSubCmdID )
{
    //m_pIMainServiceFrame->SendData( wChairID, CMD_GAME_BASE, wSubCmdID, NULL, 0 );
    return true;
}

bool Table::SendTableData( int wChairID, int wSubCmdID, void *pData, int nDataSize, int wMainCmdID )
{
    //m_pIMainServiceFrame->SendData( wChairID, wMainCmdID, wSubCmdID, pData, nDataSize );
    return true;
}

bool Table::PerformStandUpAction( IUserItem *pIUserItem )
{
    m_pITableUserAction->OnUserStandUp( 0, pIUserItem, false );
    return true;
}

bool Table::PerformSitDownAction( int wChairID, IUserItem *pIUserItem, std::string lpszPassint )
{
    m_pITableUserAction->OnUserSitDown( wChairID, pIUserItem, false );
    return true;
}

bool Table::OnUserOffLine( IUserItem *pIUserItem )
{
    return true;
}

bool Table::OnTimer( int unTimerID, int unParam )
{
    return true;
}

bool Table::OnEventSocketGame( MsgHead *pMsgHead, void *pData, int nDataSize, IUserItem *pIUserItem )
{
    //LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " wSubCmdID:" << rCommand.wSubCmdID << ",nDataSize:" << nDataSize;
    assert( m_pITableHook != NULL );

    return m_pITableHook->OnGameMessage( pMsgHead, pData, nDataSize, pIUserItem );
}

bool Table::OnLoadCfgMessage( void *pData, int nDataSize, IUserItem *pIUserItem )
{
    LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << ",nDataSize:" << nDataSize;
    assert( m_pITableHook != NULL );
    return m_pITableHook->OnLoadCfgMessage( pData, nDataSize, pIUserItem );
}

IMsgClient *Table::GetMsgClient()
{
    return m_pIMsgClient;
}
