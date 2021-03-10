/*
*   BaseDefine.h
*
*   Macro define.
*
*   Created on: 2018-11-12
*   Author:
*   All rights reserved.
*/
#ifndef __BaseDefine_H__
#define __BaseDefine_H__

namespace Firefly
{
	#define		CMD_CORE_BASE		100	//core cmd
	#define		CMD_COMMON_BASE 	200	//public cmd
	#define		CMD_CENTER_BASE 	300	//center cmd
	#define		CMD_HALL_BASE 		400	//hall cmd
	#define		CMD_GAME_BASE 		500	//game cmd
	#define		CMD_DB_BASE 		600	//db cmd
	#define		CMD_WEB_BASE 		700	//web cmd
	
	#define		SUB_CORE_HEART        		1
	#define		SUB_CORE_SHUT_SOCKET     	3
	
	#define TIMES_INFINITY          0xFFFFFFFF

	#define CONNECT_SUCCESS             0
	#define CONNECT_FAILURE             1
	#define CONNECT_EXCEPTION           2

	#define MSG_TIMER                 0x0001
	#define MSG_CONTROL               0x0002
	#define MSG_DATABASE              0x0003

	#define MSG_CLIENT_READ       0x0004
	#define MSG_CLIENT_SHUT       0x0005
	#define MSG_CLIENT_LINK       0x0006

	#define MSG_SERVER_ACCEPT    0x0007
	#define MSG_SERVER_READ      0x0008
	#define MSG_SERVER_SHUT      0x0009
	#define MSG_SERVER_READY     0x0010

	#define SOCKET_IDLE          0
	#define SOCKET_WAIT          1
	#define SOCKET_CONNECT       2

	#define INVALID_SOCKET              0
	#define SOCKET_ERROR                (-1)

	#define SafeDelete(p) { try { delete p; } catch (...) { assert(false); } p=nullptr; }
	#define SafeDeleteArray(p)  { try { delete []p; } catch (...) { assert(false); } p=nullptr; }

	#define SOCKET_BUFFER_LEN           614400
	#define SOCKET_BODY_LEN           	(SOCKET_BUFFER_LEN-sizeof(MsgHead))
	#define ASYN_DATA_LEN       		614400

	enum eConnectType
	{
		eConnectType_Unknow,
		eConnectType_WebSocket,
		eConnectType_Socket,
	};
	
	#pragma pack(1)

	struct MsgHead
	{
		unsigned short			wMainCmdID;
		unsigned short			wSubCmdID;
		unsigned short			wGateIndex;
		unsigned short			wHallGateIndex;
		unsigned int 			nBodyLen;
		unsigned int			nSocketID;
	};

	#pragma  pack()
}

#endif