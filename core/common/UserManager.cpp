#include "UserManager.h"
#include <glog/logging.h>
#include <assert.h>

namespace Firefly
{
    UserItem::UserItem()
    {
    }

    UserItem::~UserItem()
    {
    }

    void UserItem::ResetUserItem()
    {
        memset(&m_UserInfo, 0, sizeof(m_UserInfo));
    }

    //////////////////////////////////////////////////////////////////////////////////

    UserManager::UserManager()
    {
        return;
    }

    UserManager::~UserManager()
    {
        for (size_t i = 0; i < m_UserItemStore.size(); i++) m_UserItemStore[i]->Release();

        m_UserIDMap.clear();
        m_UserItemStore.clear();
        return;
    }

    IUserItem* UserManager::EnumUserItem(int wEnumIndex)
    {
        return NULL;
    }

    IUserItem* UserManager::SearchUserItem(int nUserID)
    {
        return m_UserIDMap[nUserID];
    }

    bool UserManager::DeleteUserItem()
    {
        for (auto& item : m_UserIDMap)
        {
            m_UserItemStore.push_back(item.second);
        }

        m_UserWaitArray.clear();
        m_UserIDMap.clear();
        return true;
    }

    bool UserManager::DeleteUserItem(IUserItem* pIUserItem)
    {
        LOG(INFO) << strThreadLogFlag << __FUNCTION__ << pIUserItem;

        if (NULL == pIUserItem)
        {
            return false;
        }

        auto nUserID = pIUserItem->GetUserInfo()->nUserID;

        if (nUserID > 0)
        {
            DeleteUserWaitItem(pIUserItem);
            auto iter = m_UserIDMap.find(nUserID);

            if (iter != m_UserIDMap.end())
            {
                m_UserItemStore.push_back(iter->second);
                iter->second->ResetUserItem();
                m_UserIDMap.erase(iter);
            }

            LOG(INFO) << strThreadLogFlag << __FUNCTION__ << " DeleteUserItem--nUserID: " << nUserID << " : " << m_UserIDMap.size();
            return true;
        }

        //assert( false );
        return false;
    }

    bool UserManager::InsertUserItem(IUserItem** pIServerUserResult, tagUserInfo& UserInfo)
    {
        UserItem* pServerUserItem = NULL;

        if (m_UserItemStore.size() > 0)
        {
            int nItemPostion = m_UserItemStore.size() - 1;
            pServerUserItem = m_UserItemStore[nItemPostion];
            m_UserItemStore.pop_back();
            LOG(INFO) << strThreadLogFlag << __FUNCTION__ << " ItemPostion: " << nItemPostion;
        }
        else
        {
            try
            {
                pServerUserItem = new UserItem();
            }
            catch (...)
            {
                assert(false);
                return false;
            }
        }

        *pIServerUserResult = pServerUserItem;
        pServerUserItem->m_UserInfo = UserInfo;
        auto iter = m_UserIDMap.find(UserInfo.nUserID);

        if (iter != m_UserIDMap.end())
        {
            LOG(INFO) << strThreadLogFlag << __FUNCTION__ << " InsertUserItem--Item: " << iter->second << " : " << pServerUserItem;

            if (iter->second && iter->second != pServerUserItem)
            {
                iter->second->ResetUserItem();
                m_UserItemStore.push_back(iter->second);
            }
        }

        m_UserIDMap[UserInfo.nUserID] = pServerUserItem;
        return true;
    }

    int UserManager::GetUserOnLine(unsigned char byClientType)
    {
        int wCount = m_UserIDMap.size();
        /*
        for( auto item : m_UserIDMap )
        {
            if( item.second->GetClientKind() == byClientType )
            {
                wCount++;
            }
        }
        */
        return wCount;
    }

    IUserItem* UserManager::EnumUserWaitItem(int wEnumIndex)
    {
        if (wEnumIndex >= (int)m_UserWaitArray.size()) return NULL;

        return m_UserWaitArray[wEnumIndex];
    }

    bool UserManager::DeleteUserWaitItem(IUserItem* pIUserItem)
    {
        UserItem* pTempUserItem = NULL;

        for (size_t i = 0; i < m_UserWaitArray.size(); i++)
        {
            pTempUserItem = m_UserWaitArray[i];

            if (pIUserItem != pTempUserItem) continue;

            DeleteUserWaitItem(i);
            return true;
        }

        return false;
    }

    int UserManager::DeleteUserWaitItem(int wEnumIndex)
    {
        if (wEnumIndex >= (int)m_UserWaitArray.size())
        {
            return m_UserWaitArray.size();
        }

        m_UserWaitArray.erase(m_UserWaitArray.begin() + wEnumIndex);
        return wEnumIndex;
    }

    int UserManager::SearchUserGate(int nUserID)
    {
        IUserItem* pISvrUserItem = SearchUserItem(nUserID);

        if (pISvrUserItem)
        {
            auto dwGateID = 0;
            LOG(INFO) << strThreadLogFlag << __FUNCTION__ << " SearchUserGate GateID:" << dwGateID;
            return dwGateID;
        }

        LOG(INFO) << strThreadLogFlag << __FUNCTION__ << " SearchUserGate Error User:" << nUserID;
        return -1;
    }

    bool UserManager::DeleteUserGate(int nUserID)
    {
        IUserItem* pISvrUserItem = SearchUserItem(nUserID);
        LOG(INFO) << strThreadLogFlag << __FUNCTION__ << " DeleteUserGate---UserID:" << nUserID;

        if (pISvrUserItem)
        {
            auto dwGateID = 0;
            LOG(INFO) << strThreadLogFlag << __FUNCTION__ << " DeleteUserGate-Gate:" << dwGateID;
            return DeleteUserItem(pISvrUserItem);
        }

        return false;
    }
}