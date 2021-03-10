#ifndef __COMMON_DEFINE_H__
#define __COMMON_DEFINE_H__

//////////////INSTANCE DEFINE/////////////////
#define SESSION_TYPE_BEGIN             -1
#define CLI_TYPE                        0
#define SERVE_TYPE_GATE                 1
#define SERVE_TYPE_HALL                 2
#define SERVE_TYPE_GAME                 3
#define SERVE_TYPE_MQ                   4
#define SERVE_TYPE_DB                   5
#define SERVE_TYPE_WEB                  6
#define SERVE_TYPE_CENTER               7
#define SESSION_TYPE_END                8

//////////////ERROR DEFINE/////////////////
#define OP_SUCCESE                      0
#define OP_FAILD                       -1

#define ERROR_DO_NOT_SUPORT_SVT_TYPE    100


//////////////TASK DEFINE/////////////////

#define CONNECT_TIMER                   0x00000001
#define LOGIN_RESP_TIMER                0x00000002
#define LOGIN_SWITCH_TIMER              0x00000005

//////////////CMD DEFINE/////////////////
#define PING_DATA_BASE_TIMER            0x00000006
#define MQ_HEART_BEAT_TIMER             0x00000007

#define PING_DATA_BASE_INTERVAL         3600
#define PING_MQ_INTERVAL                120

#define     CUSTOMIZE_EVENT_CONNECT     1000
#define     CUSTOMIZE_EVENT_PING_DB     1001


#endif

