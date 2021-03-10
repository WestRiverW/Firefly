/*
*   MsgAssist.h
*
*   Network Assist Class.
*
*   Created on: 2018-11-07
*   Author:
*   All rights reserved.
*/
#ifndef __MsgAssist_H__
#define __MsgAssist_H__

#include <string>
#include <common/BaseDefine.h>

namespace Firefly
{
	class MsgAssist
	{
	public:
		static void encode(MsgHead* pMsgHead, void* pData, unsigned int wDataSize, char* pDataBuffer, int& nPacketSize);
		//static void encode(protocol::MsgBody* pMsgBody, void* data, int& nBodyLen);
		//static bool decode(unsigned char* data, unsigned int nBodyLen, protocol::MsgBody* pMsgBody);

	public:
		static const unsigned int MSG_HEAD_LENGTH;
	};
}

#endif