#include "MsgAssist.h"
#include <arpa/inet.h>
#include <glog/logging.h>

namespace Firefly
{
	const unsigned int MsgAssist::MSG_HEAD_LENGTH = sizeof(MsgHead);

	void MsgAssist::encode(MsgHead* pMsgHead, void* pData, unsigned int nDataSize, char* pDataBuffer, int& nPacketSize)
	{
		nPacketSize = MsgAssist::MSG_HEAD_LENGTH;
		memcpy(pDataBuffer, pMsgHead, nPacketSize);

		memcpy(pDataBuffer + nPacketSize, pData, nDataSize);
		nPacketSize += nDataSize;
	}
	/*
	void MsgAssist::encode(protocol::MsgBody* pMsgBody, void* data, int& nBodyLen)
	{
		nBodyLen = pMsgBody->ByteSizeLong();
		if (!pMsgBody->SerializeToArray(data, nBodyLen))
		{
			nBodyLen = 0;
		}
	}

	bool MsgAssist::decode(unsigned char* data, unsigned int nDataSize, protocol::MsgBody* pMsgBody)
	{
		if (data && nDataSize > 0)
		{
			if (!pMsgBody->ParseFromArray(data, nDataSize))
			{
				return false;
			}
		}
		return true;
	}*/
}