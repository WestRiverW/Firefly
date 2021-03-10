#ifndef __HttpServer_H__
#define __HttpServer_H__

#include <event2/event.h>
#include <event2/http.h>
#include <event2/buffer.h>
#include <event2/util.h>
#include <event2/thread.h>
#include <event2/keyvalq_struct.h>
#include <utils/Thread.h>
#include <common/BaseDefine.h>
#include <common/BaseCore.h>
#include <common/AsynEngine.h>

using namespace Firefly;

class HttpServer : public Thread
{
public:
    HttpServer();
    ~HttpServer();

public:
    bool InitThread( IBridgeHook *pIBridgeHook, unsigned short port );

public:
    virtual bool OnRun();

    static  void HandleHttpRequest( struct evhttp_request *req, void *arg );

private:
    unsigned short               m_usPort;

    struct evhttp               *m_pHttp;
    struct event_base           *m_pBase;
    struct evhttp_bound_socket  *m_pSocketHandle;
    static IBridgeHook  *m_pIBridgeHook;
};


#endif