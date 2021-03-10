#include "RedisClient.h"
#include <glog/logging.h>

namespace Firefly
{
    RedisClient::RedisClient()
    {
    }

    RedisClient::~RedisClient()
    {
    }

    bool RedisClient::Connect(const char* ip, int port, const char* pwd)
    {
        LOG(INFO) << __FUNCTION__ << " Redis Start Connect:" << ip << "  port:" << port;
        //Synchronous API
        m_pRedisContext = redisConnect(ip, port);

        //err /* Error flags, 0 when there is no error */
        if (m_pRedisContext == NULL || m_pRedisContext->err)
        {
            if (m_pRedisContext)
            {
                redisFree(m_pRedisContext);
            }

            LOG(INFO) << __FUNCTION__ << " Redis Start Connect:" << ip << "  port:" << port << " fail." << m_pRedisContext << "  " << m_pRedisContext->err;
            return false;
        }

        //set connect pwd，fail return NULL
        char command[32] = { 0 };
        sprintf(command, "AUTH %s", pwd);
        m_pRedisReply = (redisReply*)redisCommand(m_pRedisContext, command);

        if (m_pRedisReply == NULL || m_pRedisContext->err)
        {
            LOG(INFO) << __FUNCTION__ << " Redis Start Connect:" << ip << "  port:" << port << " fail password.";
            return false;
        }

        LOG(INFO) << __FUNCTION__ << " Redis Start Connect:" << ip << "  port:" << port << " success." << m_pRedisContext->err;
        return true;
    }

    bool RedisClient::Execute(const std::string& command)
    {
        m_pRedisReply = (redisReply*)redisCommand(m_pRedisContext, command.c_str());

        if (m_pRedisReply || m_pRedisContext->err)
        {
            return false;
        }

        return true;
    }

    bool RedisClient::Close()
    {
        redisFree(m_pRedisContext);
        m_pRedisContext = NULL;

        if (m_pRedisReply)
        {
            freeReplyObject(m_pRedisReply);
            m_pRedisReply = NULL;
        }

        return true;
    }
}