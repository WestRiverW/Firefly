#include "Utility.h"
#include <sstream>
#include <memory.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <external/sha1/SHA1.h>
#include <external/json/cJSON.h>
#include <curl/curl.h>
#include <glog/logging.h>
#include "AESCipher.h"

namespace Firefly
{
    static const std::string Base64Chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    Utility::Utility()
    {
    }

    Utility::~Utility()
    {
    }

    std::string Utility::Base64Encode(unsigned char const* BytesToEncode, unsigned int size)
    {
        std::string ret;
        int i = 0;
        int j = 0;
        unsigned char char_array_3[3];
        unsigned char char_array_4[4];

        while (size--)
        {
            char_array_3[i++] = *(BytesToEncode++);

            if (i == 3)
            {
                char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
                char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
                char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
                char_array_4[3] = char_array_3[2] & 0x3f;

                for (i = 0; (i < 4); i++)
                    ret += Base64Chars[char_array_4[i]];

                i = 0;
            }
        }

        if (i)
        {
            for (j = i; j < 3; j++)
                char_array_3[j] = '\0';

            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for (j = 0; (j < i + 1); j++)
                ret += Base64Chars[char_array_4[j]];

            while ((i++ < 3))
                ret += '=';
        }

        return ret;
    }

    std::string Utility::Base64Decode(std::string const& EncodedString)
    {
        int in_len = (int)EncodedString.size();
        int i = 0;
        int j = 0;
        int in_ = 0;
        unsigned char char_array_4[4], char_array_3[3];
        std::string ret;

        while (in_len-- && (EncodedString[in_] != '=') && IsBase64(EncodedString[in_]))
        {
            char_array_4[i++] = (unsigned char)EncodedString[in_];
            in_++;

            if (i == 4)
            {
                for (i = 0; i < 4; i++)
                    char_array_4[i] = (unsigned char)Base64Chars.find(char_array_4[i]);

                char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
                char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
                char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

                for (i = 0; (i < 3); i++)
                    ret += char_array_3[i];

                i = 0;
            }
        }

        if (i)
        {
            for (j = i; j < 4; j++)
                char_array_4[j] = (unsigned char)0;

            for (j = 0; j < 4; j++)
                char_array_4[j] = (unsigned char)Base64Chars.find(char_array_4[j]);

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
        }

        return ret;
    }

    std::string Utility::SHA1(const std::string& EncodedString)
    {
        CSHA1 sha1;
        sha1.Update((unsigned char*)EncodedString.c_str(), EncodedString.length());
        sha1.Final();
        unsigned char chSha1[20] = "";
        sha1.GetHash(chSha1);
        std::string strResult;
        strResult.assign((char*)chSha1);
        return strResult;
    }

    void Utility::GetPacketHeadByKey(const std::string& content, std::vector<int>& list)
    {
        cJSON* pRoot = cJSON_Parse(content.c_str());
        list.clear();

        if (pRoot)
        {
            cJSON* pHeadChild = cJSON_GetObjectItem(pRoot, "head");

            if (pHeadChild)
            {
                int         nLength = cJSON_GetArraySize(pHeadChild);

                for (int i = 0; i < nLength; ++i)
                {
                    list.push_back(cJSON_GetArrayItem(pHeadChild, i)->valueint);
                }
            }
        }
    }

    enWebSocketStatus Utility::WSHandShake(unsigned char pcbDataBuffer[], std::string& response)
    {
        enWebSocketStatus enWSStatus = enWebSocketStatus_UnConnect;
        std::istringstream stream((char*)pcbDataBuffer);
        std::string::size_type pos = 0;
        std::string reqType;
        std::getline(stream, reqType);

        if (reqType.substr(0, 4) != "GET ")
        {
            return enWSStatus;
        }
        pos = reqType.find("token=", 0);
        if (pos == std::string::npos)
        {
            return enWSStatus;
        }
        else
        {
            auto epos = reqType.find(" HTTP/", 0);
            if (epos == std::string::npos || epos <= pos + 6)
            {
                return enWSStatus;
            }

            auto token = reqType.substr(pos + 6, epos - pos - 6);
            token = Utility::Base64Decode(token);

            if (token.size() <= 0)
            {
                return enWSStatus;
            }

            const unsigned char key[] = "78JsY$IU#@98p*sM";
            cipher::AesEcbCipher enAesEcb(key, false);
            unsigned char code_buf[1024] = { 0 };
            uint32_t lenth = token.size();
            uint32_t dest_len = lenth;
            auto ret = enAesEcb.decode((const unsigned char*)token.c_str(), lenth, code_buf, dest_len);

            if (ret != 0 || dest_len != 16 || lenth != 32)
            {
                return enWSStatus;
            }
        }

        std::string header;
        std::string websocketKey;

        while (std::getline(stream, header) && header != "\r")
        {
            header.erase(header.end() - 1);
            pos = header.find(": ", 0);

            if (pos != std::string::npos)
            {
                std::string key = header.substr(0, pos);
                std::string value = header.substr(pos + 2);

                if (key == "Sec-WebSocket-Key")
                {
                    enWSStatus = enWebSocketStatus_Connect;
                    websocketKey = value;
                    break;
                }
            }
        }

        if (enWSStatus != enWebSocketStatus_Connect)
        {
            return enWSStatus;
        }

        response = "HTTP/1.1 101 Switching Protocols\r\n";
        response += "Upgrade: websocket\r\n";
        response += "Connection: upgrade\r\n";
        response += "Sec-WebSocket-Version: 13\r\n";
        response += "Sec-WebSocket-Accept: ";
        const std::string magicKey("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
        std::string serverKey = websocketKey + magicKey;
        unsigned char shaHash[20];
        memset(shaHash, 0, sizeof(shaHash));
        std::string strSHA1 = Utility::SHA1(serverKey);
        memcpy(shaHash, strSHA1.c_str(), sizeof(shaHash));
        serverKey = Utility::Base64Encode(shaHash, sizeof(shaHash)) + "\r\n\r\n";
        response += serverKey;
        //int nRet = send(m_hSocketHandle, response.c_str(), response.length(), 0);
        return enWSStatus;
    }


    int Utility::WSEncodeFrame(const std::string& inMessage, char cbBuffer[], unsigned int& wLength, enWSFrameType frameType /* = enWSTextFrame */)
    {
        int ret = enWSErrorFrame;
        const unsigned int messageLength = inMessage.size();
        char payloadFieldExtraBytes = 0;
        if (messageLength <= 0x7d)
        {
            payloadFieldExtraBytes = 0;
        }
        else if (messageLength < 65536)
        {
            payloadFieldExtraBytes = 2;
        }
        else
        {
            payloadFieldExtraBytes = 8;
        }
        char frameHeaderSize = 2 + payloadFieldExtraBytes;
        char* frameHeader = new char[frameHeaderSize];
        memset(frameHeader, 0, frameHeaderSize);
        frameHeader[0] = static_cast<char>(0x80 | frameType);

        if (messageLength <= 0x7d)
        {
            frameHeader[1] = static_cast<char>(messageLength);
        }
        else if (messageLength < 65536)
        {
            frameHeader[1] = 0x7e;
            unsigned short len = htons(messageLength);
            memcpy(&frameHeader[2], &len, payloadFieldExtraBytes);
        }
        else
        {
            frameHeader[1] = 0x7f;
            memset(&frameHeader[2], 0, 4);
            unsigned int len = htonl(messageLength);
            memcpy(&frameHeader[6], &len, payloadFieldExtraBytes - 4);
        }

        unsigned int frameSize = frameHeaderSize + messageLength;
        wLength = frameSize;
        char* frame = new char[wLength];
        memcpy(frame, frameHeader, frameHeaderSize);
        memcpy(frame + frameHeaderSize, inMessage.c_str(), messageLength);
        memcpy(cbBuffer, frame, wLength);
        delete[] frame;
        delete[] frameHeader;
        return ret;
    }

    int Utility::WSEncodeFrame(char cbInBuf[], unsigned int wInLen, char cbOutBuf[], unsigned int& wOutLen, enWSFrameType frameType /* = enWSTextFrame */)
    {
        int ret = enWSErrorFrame;
        const unsigned int messageLength = wInLen;
        char payloadFieldExtraBytes = 0;

        if (messageLength <= 0x7d)
        {
            payloadFieldExtraBytes = 0;
        }
        else if (messageLength < 65536)
        {
            payloadFieldExtraBytes = 2;
        }
        else
        {
            payloadFieldExtraBytes = 8;
        }

        char frameHeaderSize = 2 + payloadFieldExtraBytes;
        char* frameHeader = new char[frameHeaderSize];
        memset(frameHeader, 0, frameHeaderSize);
        frameHeader[0] = static_cast<char>(0x80 | frameType);

        if (messageLength <= 0x7d)
        {
            frameHeader[1] = static_cast<char>(messageLength);
        }
        else if (messageLength < 65536)
        {
            frameHeader[1] = 0x7e;
            unsigned short len = htons(messageLength);
            memcpy(&frameHeader[2], &len, payloadFieldExtraBytes);
        }
        else
        {
            frameHeader[1] = 0x7f;
            memset(&frameHeader[2], 0, 4);
            unsigned int len = htonl(messageLength);
            memcpy(&frameHeader[6], &len, payloadFieldExtraBytes - 4);
        }

        unsigned int frameSize = frameHeaderSize + messageLength;
        wOutLen = frameSize;
        char* frame = new char[wOutLen];
        memcpy(frame, frameHeader, frameHeaderSize);
        memcpy(frame + frameHeaderSize, cbInBuf, wInLen);
        memcpy(cbOutBuf, frame, wOutLen);
        delete[] frame;
        delete[] frameHeader;
        return ret;
    }
    /******************************WebSocket**********************
    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    +-+-+-+-+-------+-+-------------+-------------------------------+
    |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
    |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
    |N|V|V|V|       |S|             |   (if payload len==126/127)   |
    | |1|2|3|       |K|             |                               |
    +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
    |     Extended payload length continued, if payload len == 127  |
    + - - - - - - - - - - - - - - - +-------------------------------+
    |                               |Masking-key, if MASK set to 1  |
    +-------------------------------+-------------------------------+
    | Masking-key (continued)       |          Payload Data         |
    +-------------------------------- - - - - - - - - - - - - - - - +
    :                     Payload Data continued ...                :
    + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
    |                     Payload Data continued ...                |
    +---------------------------------------------------------------+
    */

    int Utility::WSGetFrameSize(const char* frameData, int frameLength)
    {
        if (frameLength < 2 || frameData == NULL)
        {
            return -1;
        }

        unsigned int payloadLength = static_cast<unsigned int>(frameData[1] & 0x7f);

        if (payloadLength == 0x7e && frameLength >= 4)
        {
            unsigned short payloadLength16b = 0;
            memcpy(&payloadLength16b, &frameData[2], 2);
            payloadLength = ntohs(payloadLength16b);
            payloadLength = payloadLength + 2;
        }
        else if (payloadLength == 0x7f && frameLength >= 10)
        {
            unsigned int payloadLength32b = 0;
            memcpy(&payloadLength32b, &frameData[6], 4);
            payloadLength = ntohl(payloadLength32b);
            payloadLength = payloadLength + 8;
        }

        return payloadLength;
    }

    int Utility::WSGetFrameType(const char* frameData, int frameLength)
    {
        if (frameLength < 2)
        {
            return enWSErrorFrame;
        }

        if ((frameData[0] & 0x70) != 0x0)
        {
            return enWSErrorFrame;
        }

        if ((frameData[1] & 0x80) != 0x80)
        {
            return enWSErrorFrame;
        }

        if ((frameData[0] & 0x80) != 0x80 && (frameData[0] & 0x0f) != 0)
        {
            unsigned int payloadLength = Utility::WSGetFrameSize(frameData, frameLength);

            if ((unsigned int)frameLength < payloadLength + 2 + 4)
            {
                return enWSContinueFrame;
            }
        }
        else if ((frameData[0] & 0x80) == 0x80 && (frameData[0] & 0x0f) == 0) // 分片中的最后一片
        {
            unsigned int payloadLength = Utility::WSGetFrameSize(frameData, frameLength);

            if ((unsigned int)frameLength < payloadLength + 2 + 4)
            {
                return enWSContinueFrame;
            }
        }
        else if ((frameData[0] & 0x80) != 0x80)
        {
            unsigned int payloadLength = Utility::WSGetFrameSize(frameData, frameLength);

            if ((unsigned int)frameLength > payloadLength + 2 + 4)
            {
                auto nRet = Utility::WSGetFrameType(frameData + payloadLength + 6, frameLength - payloadLength - 6);
                if (nRet == enWSErrorFrame || nRet == enWSContinueFrame)
                {
                    return nRet;
                }
            }
            else
            {
                return enWSContinueFrame;
            }
        }

        char opcode = static_cast<char>(frameData[0] & 0x0f);
        return opcode;
    }

    int Utility::WSDecodeFrame(const char* frameData, unsigned int frameLength, std::string& outMessage, unsigned int& wHeadLen, unsigned int& wContentLen)
    {
        int ret = enWSErrorFrame;
        unsigned int payloadLength = 0;
        char payloadFieldExtraBytes = 0;
        payloadLength = static_cast<unsigned int>(frameData[1] & 0x7f);

        if (payloadLength == 0x7e)
        {
            unsigned short payloadLength16b = 0;
            payloadFieldExtraBytes = 2;
            memcpy(&payloadLength16b, &frameData[2], payloadFieldExtraBytes);
            payloadLength = ntohs(payloadLength16b);
        }
        else if (payloadLength == 0x7f)
        {
            unsigned int payloadLength32b = 0;
            payloadFieldExtraBytes = 8;
            memcpy(&payloadLength32b, &frameData[6], 4);
            payloadLength = ntohl(payloadLength32b);
        }

        wHeadLen = 2 + payloadFieldExtraBytes + 4;

        if (payloadLength > 0)
        {
            const char* maskingKey = &frameData[2 + payloadFieldExtraBytes];
            char* payloadData = new char[payloadLength + 1];
            memset(payloadData, 0, payloadLength + 1);
            memcpy(payloadData, &frameData[wHeadLen], payloadLength);

            for (unsigned int i = 0; i < payloadLength; i++)
            {
                payloadData[i] = payloadData[i] ^ maskingKey[i % 4];
            }

            outMessage = payloadData;
            delete[] payloadData;
            wContentLen = payloadLength;
        }

        if (frameLength < wHeadLen + wContentLen)
        {
            ret = enWSErrorFrame;
        }
        else
        {
            ret = ((frameData[0] & 0x80) == 0x80 ? 1 : 0);
        }

        return ret;
    }

    int Utility::WSDecodeFrame(const char* frameData, unsigned int frameLength, char outFrameData[], unsigned int& wHeadLen, unsigned int& wContentLen)
    {
        int ret = enWSErrorFrame;

        if (frameLength < 2)
        {
            return ret;
        }

        unsigned int payloadLength = 0;
        char payloadFieldExtraBytes = 0;
        //
        payloadLength = static_cast<unsigned int>(frameData[1] & 0x7f);

        if (payloadLength == 0x7e) //126
        {
            unsigned short payloadLength16b = 0;
            payloadFieldExtraBytes = 2;
            memcpy(&payloadLength16b, &frameData[2], payloadFieldExtraBytes);
            payloadLength = ntohs(payloadLength16b);
        }
        else if (payloadLength == 0x7f) //127
        {
            unsigned int payloadLength32b = 0;
            payloadFieldExtraBytes = 8;

            if (frameLength >= 10)
            {
                memcpy(&payloadLength32b, &frameData[6], 4);
                payloadLength = ntohl(payloadLength32b);
            }
            else
            {
                return ret;
            }
        }

        wHeadLen = 2 + payloadFieldExtraBytes + 4;

        if (payloadLength > 0 && frameLength >= wHeadLen + payloadLength)
        {
            const char* maskingKey = &frameData[2 + payloadFieldExtraBytes];
            char* payloadData = new char[payloadLength + 1];
            memset(payloadData, 0, payloadLength + 1);
            memcpy(payloadData, &frameData[wHeadLen], payloadLength);

            for (unsigned int i = 0; i < payloadLength; i++)
            {
                payloadData[i] = payloadData[i] ^ maskingKey[i % 4];
            }

            memcpy(outFrameData, payloadData, payloadLength);
            delete[] payloadData;
            wContentLen = payloadLength;
        }
        else
        {
            return ret;
        }

        if (frameLength < wHeadLen + wContentLen)
        {
            ret = enWSErrorFrame;
        }
        else
        {
            ret = ((frameData[0] & 0x80) == 0x80 ? 1 : 0);
        }

        return ret;
    }

    unsigned int Utility::GetTickCount()
    {
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
    }

    unsigned long long Utility::GetElapseTime()
    {
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        return (ts.tv_sec * 1000 * 1000 + ts.tv_nsec / 1000);
    }

    std::string Utility::GetSystemTime()
    {
        time_t timep;
        time(&timep);
        return asctime(gmtime(&timep));
    }

    unsigned int Utility::MAKELONG(unsigned short a, unsigned short b)
    {
        unsigned int nTempValue = ((unsigned int)a) << 16;
        nTempValue = nTempValue | b;
        return nTempValue;
    }

    unsigned short Utility::HIWORD(unsigned int a)
    {
        unsigned short wTempValue = (a >> 16) & 0xffff;
        return wTempValue;
    }

    unsigned short Utility::LOWORD(unsigned int a)
    {
        unsigned short wTempValue = (unsigned short)(a & 0xffff);
        return wTempValue;
    }

    unsigned int Utility::TranslateAddress(const char* szServerIP)
    {
        const char* ServerAddr(szServerIP);
        unsigned int dwServerIP = inet_addr(ServerAddr);

        if (dwServerIP == INADDR_NONE)
        {
            hostent* lpHost = gethostbyname(ServerAddr);

            if (lpHost == NULL) return INADDR_NONE;

            if (lpHost->h_length > 0)
            {
                dwServerIP = inet_addr(lpHost->h_addr_list[0]);
            }
        }

        return dwServerIP;
    }

    std::string Utility::UrlEncode(const std::string& szToEncode)
    {
        std::string src = szToEncode;
        char hex[] = "0123456789ABCDEF";
        std::string dst;

        for (size_t i = 0; i < src.size(); ++i)
        {
            unsigned char cc = src[i];

            if ((cc >= 'A' && cc <= 'Z')
                || (cc >= 'a' && cc <= 'z')
                || (cc >= '0' && cc <= '9')
                || cc == '.'
                || cc == '_'
                || cc == '-'
                || cc == '*')
            {
                if (cc == ' ')
                {
                    dst += "+";
                }
                else
                    dst += cc;
            }
            else
            {
                unsigned char c = static_cast<unsigned char>(src[i]);
                dst += '%';// dst += '2'; dst += '5';
                dst += hex[c / 16];
                dst += hex[c % 16];
            }
        }

        return dst;
    }

    std::string Utility::UrlDecode(const std::string& szToDecode)
    {
        std::string result;
        long hex = 0;

        for (size_t i = 0; i < szToDecode.length(); ++i)
        {
            switch (szToDecode[i])
            {
            case '+':
                result += ' ';
                break;

            case '%':
                if (isxdigit(szToDecode[i + 1]) && isxdigit(szToDecode[i + 2]))
                {
                    std::string hexStr = szToDecode.substr(i + 1, 2);
                    hex = strtol(hexStr.c_str(), 0, 16);

                    if (!((hex >= 48 && hex <= 57) || //0-9
                        (hex >= 97 && hex <= 122) || //a-z
                        (hex >= 65 && hex <= 90)    //A-Z
                        || hex == 0x5F               // _
                        || hex == 0x2D               // -
                        || hex == 0x2E               // .
                        || hex == 0x7E               // ~
                        /*
                        ! $ & ' ( )
                        * + , - . /
                        : ; = ? @ _
                        */
                        /*
                        || hex == 0x21 || hex == 0x24 || hex == 0x26 || hex == 0x27 || hex == 0x28 || hex == 0x29
                        || hex == 0x2A || hex == 0x2B || hex == 0x2C || hex == 0x2D || hex == 0x2E || hex == 0x2F
                        || hex == 0x3A || hex == 0x3B || hex == 0x3D || hex == 0x3F || hex == 0x40 || hex == 0x5F
                        */
                        ))
                    {
                        result += char(hex);
                        i += 2;
                    }
                    else
                    {
                        result += '%';
                    }
                }
                else
                {
                    result += '%';
                }

                break;

            default:
                result += szToDecode[i];
                break;
            }
        }

        return result;
    }
}