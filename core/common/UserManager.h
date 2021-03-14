#ifndef __UserManager_H__
#define __UserManager_H__

#include <map>
#include <list>
#include <time.h>
#include <vector>
#include <string>
#include <stdio.h>
#include "GameCore.h"

namespace Firefly
{
    class UserItem : public IUserItem
    {
        friend class UserManager;
    protected:
        tagUserInfo             m_UserInfo;
    protected:
        std::string             m_szLogonPass;

    protected:
        UserItem();
        virtual ~UserItem();
    public:
        virtual void Release()
        {
            delete this;
        }
    public:
        virtual tagUserInfo* GetUserInfo()
        {
            return &m_UserInfo;
        }

    public:
        virtual int GetUserID()
        {
            return m_UserInfo.nUserID;
        }
        virtual std::string GetAccounts()
        {
            std::string  szAccounts;
            //szAccounts.assign(m_UserInfo.Accounts, strlen(m_UserInfo.Accounts));
            return szAccounts;
        }

    private:
        void ResetUserItem();
    };

    //////////////////////////////////////////////////////////////////////////////////

    class UserManager : public IUserManager
    {
    protected:
        std::map<int, UserItem*>            m_UserIDMap;
        std::vector<UserItem*>              m_UserItemActive;
        std::vector<UserItem*>              m_UserItemStore;
        std::vector<UserItem*>              m_UserWaitArray;
    public:
        UserManager();
        virtual ~UserManager();

    public:
        virtual void Release()
        {
            return;
        }
    public:
        virtual IUserItem* EnumUserItem(int wEnumIndex);
        virtual IUserItem* SearchUserItem(int nUserID);
        virtual IUserItem* EnumUserWaitItem(int wEnumIndex);
        std::map<int, UserItem*> GetOnLineUserMap()
        {
            return m_UserIDMap;
        }
    public:
        virtual int GetUserItemCount()
        {
            return (int)m_UserIDMap.size();
        }
        virtual int GetUserOnLine(unsigned char byClientType);
        virtual int GetUserWaitItemCount()
        {
            return (int)m_UserWaitArray.size();
        }
    public:
        virtual bool DeleteUserItem();
        virtual bool DeleteUserItem(IUserItem* pIUserItem);
        virtual bool InsertUserItem(IUserItem** pIServerUserResult, tagUserInfo& UserInfo);
        virtual bool DeleteUserWaitItem(IUserItem* pIUserItem);
        virtual int  DeleteUserWaitItem(int wEnumIndex);
        int  SearchUserGate(int nUserID);
        bool DeleteUserGate(int nUserID);
    };
}

#endif