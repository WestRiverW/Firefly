/*
*   Utility.h
*
*   
*
*   Created on: 2018-11-07
*   Author:
*   All rights reserved.
*/
#ifndef __Utility_H__
#define __Utility_H__

#include <string>
#include <vector>
//RFC 6455

/*  Control Frames
    Currently defined opcodes for control frames include 0x8 (Close),
    0x9 (Ping), and 0xA (Pong). Opcodes 0xB-0xF are reserved for further
    control frames yet to be defined.
*/

/*  Data Frames
    Currently defined
    opcodes for data frames include 0x1 (Text), 0x2 (Binary). Opcodes
    0x3-0x7 are reserved for further non-control frames yet to be
    defined.
*/
namespace Firefly
{
#define LEN_MD5                     33

    //websocket
    enum enWSFrameType
    {
        enWSContinueFrame = 0x00,
        //Data Frames
        enWSTextFrame = 0x01,
        enWSBinaryFrame = 0x02,
        //Control Frames
        enWSClosingFrame = 0x08,
        enWSPingFrame = 0x09,
        enWSPongFrame = 0x0A,
        //
        enWSErrorFrame = 0x10,
    };

    enum enWebSocketStatus
    {
        enWebSocketStatus_UnConnect,
        enWebSocketStatus_Connect,
    };

    class Utility
    {
    public:
        Utility();
        virtual ~Utility();

    public:
        static inline bool IsBase64(unsigned char c)
        {
            return (isalnum(c) || (c == '+') || (c == '/'));
        }
        static std::string Base64Encode(unsigned char const* BytesToEncode, unsigned int size);
        static std::string Base64Decode(std::string const& EncodedString);
        static std::string SHA1(const std::string& EncodedString);
        static std::string UrlEncode(const std::string& szToEncode);
        //URL Encode Unicode
        static std::string UrlDecode(const std::string& szToDecode);

    public:
        static void GetPacketHeadByKey(const std::string& content, std::vector<int>& list);
        static enWebSocketStatus WSHandShake(unsigned char pcbDataBuffer[], std::string& response);
        static int WSEncodeFrame(const std::string& inMessage, char cbBuffer[], unsigned int& wLength, enWSFrameType frameType = enWSTextFrame);
        static int WSEncodeFrame(char cbInBuf[], unsigned int wInLen, char cbOutBuf[], unsigned int& wOutLen, enWSFrameType frameType = enWSTextFrame);
        static int WSDecodeFrame(const char* frameData, unsigned int frameLength, std::string& outMessage, unsigned int& wHeadLen, unsigned int& wContentLen);
        static int WSDecodeFrame(const char* frameData, unsigned int frameLength, char outFrameData[], unsigned int& wHeadLen, unsigned int& wContentLen);
        static int WSGetFrameType(const char* frameData, int frameLength);
        static int WSGetFrameSize(const char* frameData, int frameLength);

    public:
        static unsigned int GetTickCount();
        static unsigned long long GetElapseTime();
        static std::string GetSystemTime();
        static unsigned int MAKELONG(unsigned short a, unsigned short b);
        static unsigned short LOWORD(unsigned int a);
        static unsigned short HIWORD(unsigned int a);

        static unsigned int TranslateAddress(const char* szServerIP);

    };
}
#endif