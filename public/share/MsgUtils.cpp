#include "MsgUtils.h"
#include <arpa/inet.h>
#include <glog/logging.h>

//const unsigned int MsgUtils::MSG_HEAD_LENGTH = sizeof(Firefly::MsgHead);

void MsgUtils::encode(pb::MsgBody* pMsgBody, char* pBuffer, int& nPacketSize)
{
	nPacketSize = pMsgBody->ByteSizeLong();
	if(!pMsgBody->SerializeToArray(pBuffer, nPacketSize))
	{
		nPacketSize = 0;
	}
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