#ifndef __LuaConfig_H__
#define __LuaConfig_H__

#include<string>
extern "C" 
{
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
};

namespace Firefly
{
    class LuaConfig
    {
    public:
        LuaConfig();
        ~LuaConfig();

        int init(const char* file = "test.lua");

        std::string getConfigByFun(const std::string& strFun);
        std::string getConfigByFun(const std::string& strFun, const std::string& param);
        std::string getSubGame(const std::string& strFun, const std::string& param, int nIndex);

    private:
        lua_State* m_pState;
    };
}

#endif