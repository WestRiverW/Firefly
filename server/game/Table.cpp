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
    //LOG(INFO)<<strThreadLogFlag << __FUNCTION__<<"TFSetGameTimer unTimerID 1:" << unTimerID<<",dwRepeat:" << dwRepeat <<"  "<<unParam;
    if( m_pITimer != NULL )
    {
        //  LOG(INFO)<<strThreadLogFlag << __FUNCTION__<<"TFSetGameTimer unTimerID 2:" << unTimerID<<",dwRepeat:" << dwRepeat<<"  "<<unParam;;
        int  dwEngineTimerID = IDI_TABLE_MODULE_START + m_iTableID * TIME_TABLE_MODULE_RANGE;
        LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << "	TFSetGameTimer unTimerID 3:" << unTimerID << ",dwRepeat:" << dwRepeat << "  " << unParam << "  " << dwEngineTimerID << "	dwElapse:" << dwElapse;
        m_pITimer->SetTimer( dwEngineTimerID + unTimerID, dwElapse, dwRepeat, unParam );
    }

    return true;
}

bool Table::KillTimer( int unTimerID )
{
    int dwEngineTimerID = IDI_TABLE_MODULE_START + m_iTableID * TIME_TABLE_MODULE_RANGE;

    if( m_pITimer != NULL )
    {
        LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << "	KillTimer unTimerID :" << unTimerID;
        m_pITimer->KillTimer( dwEngineTimerID + unTimerID );
    }

    return true;
}

bool Table::SendTableData( int wChairID, int wSubCmdID )
{
    //m_pIMainServiceFrame->SendData( wChairID, CMD_GAME_BASE, wSubCmdID, NULL, 0 );
    return true;
}

bool Table::SendTableData( int wChairID, int wSubCmdID, void *pData, int wDataSize, int wMainCmdID )
{
    //m_pIMainServiceFrame->SendData( wChairID, wMainCmdID, wSubCmdID, pData, wDataSize );
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
    //    LOG(INFO)<<strThreadLogFlag << __FUNCTION__<<" Table unTimerID 1 :" << unTimerID << "  " << unParam;
    if( ( unTimerID >= 0 ) && ( unTimerID < TIME_TABLE_Hook_RANGE ) )
    {
        //LOG(INFO)<<strThreadLogFlag << __FUNCTION__<<" Table unTimerID 2 :" << unTimerID << "  " << unParam << "  " <<m_pITableHook;
        if( m_pITableHook != NULL )
        {
            LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " Table unTimerID 3 :" << unTimerID << "  " << unParam;
            return m_pITableHook->OnTimer( unTimerID, unParam );
        }
    }

    return true;
}

bool Table::OnEventSocketGame( MsgHead *pMsgHead, void *pData, int wDataSize, IUserItem *pIUserItem )
{
    //LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " wSubCmdID:" << rCommand.wSubCmdID << ",wDataSize:" << wDataSize;
    assert( m_pITableHook != NULL );

    return m_pITableHook->OnGameMessage( pMsgHead, pData, wDataSize, pIUserItem );
}

bool Table::OnLoadCfgMessage( void *pData, int wDataSize, IUserItem *pIUserItem )
{
    LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << ",wDataSize:" << wDataSize;
    assert( m_pITableHook != NULL );
    return m_pITableHook->OnLoadCfgMessage( pData, wDataSize, pIUserItem );
}

IMsgClient *Table::GetMsgClient()
{
    return m_pIMsgClient;
}
