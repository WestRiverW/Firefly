#ifndef __HttpUtility_H__
#define __HttpUtility_H__
#include <curl/curl.h>
#include <string>
#include <iostream>
#include <vector>
using namespace std;

class HttpUtility
{
public:
    HttpUtility();
    ~HttpUtility();
public:
    static void Set_curl_global_init();
    static void Set_curl_global_cleanup();
    void SetConnectTimeout( int nTimeout );
    void SetTimeout( int nTimeout );
    int Post( const std::string &strURL, const std::string &strParams, std::string &strResponse );
    int Get( const std::string &strURL, std::string &strResponse );
    int Gets( const std::string &strUrl, std::string &strResponse, struct curl_slist *cookies, bool getCookies, const char *pCaPath = NULL );
    bool set_multi_init();
    bool set_curl_multi_perform();
    bool add_EasyToMulti( std::string sUrl, std::string &data );
    bool clean_curl_multi_perform();

public:
    static int m_count;
    CURLM *m_multi_handle;
    std::vector<CURL *>   m_curl;
    int m_nConnectTimeout;
    int m_nTimeout;
};

#endif