#include "LuaConfig.h"
#include <glog/logging.h>

namespace Firefly
{
    LuaConfig::LuaConfig()
        : m_pState(NULL)
    {
    }

    LuaConfig::~LuaConfig()
    {
        lua_close(m_pState);
    }

    int LuaConfig::init(const char* file)
    {
        m_pState = luaL_newstate();

        if (m_pState == NULL)
        {
            return -1;
        }

        luaL_openlibs(m_pState);
        const std::string lua_path = "/root/projects/lib/config/";
        std::string script = lua_path + file;
        LOG(INFO) << __FUNCTION__ << " script :" << script;
        int ret = luaL_dofile(m_pState, script.c_str());

        if (ret)
        {
            LOG(ERROR) << __FUNCTION__ << " dofile faild:" << script << ",reasion:" << lua_tostring(m_pState, -1);
            return -2;
        }

        return 0;
    }

    std::string LuaConfig::getConfigByFun(const std::string& strFun)
    {
        lua_getglobal(m_pState, strFun.c_str());
        lua_pcall(m_pState, 0, 1, 0);

        if (lua_isstring(m_pState, -1))
        {
            return lua_tostring(m_pState, -1);
        }
        else
        {
            return "";
        }
    }

    std::string LuaConfig::getConfigByFun(const std::string& strFun, const std::string& strparam)
    {
        lua_getglobal(m_pState, strFun.c_str());
        lua_pushstring(m_pState, strparam.c_str());
        lua_pcall(m_pState, 1, 1, 0);

        if (lua_isstring(m_pState, -1))
        {
            return lua_tostring(m_pState, -1);
        }
        else
        {
            return "";
        }
    }

    std::string LuaConfig::getSubGame(const std::string& strFun, const std::string& param, int nIndex)
    {
        lua_getglobal(m_pState, strFun.c_str());
        lua_pushstring(m_pState, param.c_str());
        lua_pushnumber(m_pState, nIndex);
        lua_pcall(m_pState, 2, 1, 0);

        if (lua_isstring(m_pState, -1))
        {
            return lua_tostring(m_pState, -1);
        }
        else
        {
            return "";
        }
    }
}