/*
*   MsgAssist.h
*
*   Network Assist Class.
*
*   Created on: 2018-11-07
*   Author:
*   All rights reserved.
*/
#ifndef __MsgUtils_H__
#define __MsgUtils_H__

#include <string>
#include <common/BaseDefine.h>
#include <common.pb.h>

class MsgUtils
{
public:
	static void encode( pb::MsgBody* pMsgBody, char* pBuffer, int& nPacketSize);
	//static void encode(protocol::MsgBody* pMsgBody, void* data, int& nBodyLen);
	//static bool decode(unsigned char* data, unsigned int nBodyLen, protocol::MsgBody* pMsgBody);
	//static void encode(MsgHead* pMsgHead, MsgBody* pMsgBody, char* cbBuffer, int& nPackageSize);

public:
	static const unsigned int MSG_HEAD_LENGTH;
};

#endif