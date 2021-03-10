/*
*   RedisClient.h
*
*   hiredis
*
*   Created on: 2019-12-25
*   Author:
*   All rights reserved.
*/
#ifndef __RedisClient_H__
#define __RedisClient_H__

#include <hiredis/hiredis.h>
#include <string>

namespace Firefly
{
    class RedisClient
    {
    public:
        RedisClient();
        ~RedisClient();

    public:
        bool Connect(const char* ip, int port, const char* pwd);
        bool Execute(const std::string& command);
        bool Close();

    private:
        //redisContext is not thread-safe.
        redisContext* m_pRedisContext;    //Context for a connection to Redis
        redisReply* m_pRedisReply;
    };
}

#endif