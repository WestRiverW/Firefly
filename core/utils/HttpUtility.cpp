#include "HttpUtility.h"
#include <sstream>
#include <glog/logging.h>

int HttpUtility::m_count = 0;
void HttpUtility::Set_curl_global_init()
{
    if( m_count <= 0 )
    {
        m_count = 1;
        curl_global_init( CURL_GLOBAL_ALL );
    }
    else
    {
        m_count++;
    }
}
void HttpUtility::Set_curl_global_cleanup()
{
    if( m_count <= 0 )
    {
        curl_global_cleanup();
        m_count = 0;
    }
    else
    {
        m_count--;
    }
}


size_t OnCallBackFunc( void *ptr, size_t size, size_t nmemcb, void *stream );
size_t OnCallBackFunc( void *ptr, size_t size, size_t nmemcb, void *stream )
{
    std::string *pStrResponse = dynamic_cast<std::string *>( ( std::string * )stream );

    if( NULL == pStrResponse || NULL == ptr )
    {
        return 0;
    }

    char *pData = ( char * )ptr;

    if( pData != NULL )
    {
        try
        {
            pStrResponse->append( pData, size * nmemcb );
        }
        catch( ... )
        {
            LOG( INFO )  << __FUNCTION__ << " -----OnCallBackFunc---size:" << size *nmemcb << " pData:" << pData;
        }
    }

    return nmemcb;
}

HttpUtility::HttpUtility()
{
    m_curl.clear();
    m_multi_handle = NULL;
    m_nConnectTimeout = 5000;
    m_nTimeout = 5000;
}

HttpUtility::~HttpUtility()
{
}

void HttpUtility::SetConnectTimeout( int nTimeout )
{
    m_nConnectTimeout = nTimeout;
}

void HttpUtility::SetTimeout( int nTimeout )
{
    m_nTimeout = nTimeout;
}


int HttpUtility::Post( const std::string &strURL, const std::string &strParams, std::string &strResponse )
{
    if( m_count <= 0 )
    {
        m_count = 1;
        curl_global_init( CURL_GLOBAL_ALL );
    }

    CURLcode result = CURLE_OK;

    try
    {
        CURL *curl = curl_easy_init();
        curl_easy_setopt( curl, CURLOPT_POST, 1 );
        curl_easy_setopt( curl, CURLOPT_POSTFIELDS, strParams.c_str() );
        curl_easy_setopt( curl, CURLOPT_URL, strURL.c_str() );
        curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, OnCallBackFunc );
        curl_easy_setopt( curl, CURLOPT_WRITEDATA, ( void * )&strResponse );
        curl_easy_setopt( curl, CURLOPT_NOSIGNAL, 1 );
        curl_easy_setopt( curl, CURLOPT_CONNECTTIMEOUT_MS, m_nConnectTimeout );
        curl_easy_setopt( curl, CURLOPT_TIMEOUT_MS, m_nTimeout );
        curl_easy_setopt( curl, CURLOPT_SSL_VERIFYPEER, 0 );
        curl_easy_setopt( curl, CURLOPT_SSL_VERIFYHOST, 0 );
        result = curl_easy_perform( curl );
        curl_easy_cleanup( curl );
    }
    catch( ... )
    {
        //ASSERT(FALSE);
        LOG( INFO )  << __FUNCTION__ << " -----Post--catch" ;
    }

    return result;
}

int HttpUtility::Get( const std::string &strURL, std::string &strResponse )
{
    if( m_count <= 0 )
    {
        m_count = 1;
        curl_global_init( CURL_GLOBAL_ALL );
    }

    CURLcode result = CURLE_OK;

    try
    {
        CURL *curl = curl_easy_init();
        curl_easy_setopt( curl, CURLOPT_URL, strURL.c_str() );
        curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, OnCallBackFunc );
        curl_easy_setopt( curl, CURLOPT_WRITEDATA, ( void * )&strResponse );
        curl_easy_setopt( curl, CURLOPT_NOSIGNAL, 1 );
        curl_easy_setopt( curl, CURLOPT_CONNECTTIMEOUT_MS, m_nConnectTimeout );
        curl_easy_setopt( curl, CURLOPT_TIMEOUT_MS, m_nTimeout );
        curl_easy_setopt( curl, CURLOPT_SSL_VERIFYPEER, 0 );
        curl_easy_setopt( curl, CURLOPT_SSL_VERIFYHOST, 0 );
        result = curl_easy_perform( curl );
        curl_easy_cleanup( curl );
    }
    catch( ... )
    {
        //ASSERT(FALSE);
        LOG( INFO )  << __FUNCTION__ << " -----Get--catch" ;
    }

    return result;
}

int HttpUtility::Gets( const std::string &strUrl, std::string &strResponse, struct curl_slist *cookies, bool getCookies, const char *pCaPath )
{
    if( m_count <= 0 )
    {
        m_count = 1;
        curl_global_init( CURL_GLOBAL_ALL );
    }

    CURLcode res = CURLE_OK;

    try
    {
        CURL *curl = curl_easy_init();

        if( NULL == curl )
        {
            res = CURLE_FAILED_INIT;
            return res;
        }

        curl_easy_setopt( curl, CURLOPT_URL, strUrl.c_str() );
        curl_easy_setopt( curl, CURLOPT_READFUNCTION, NULL );
        curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, OnCallBackFunc );
        curl_easy_setopt( curl, CURLOPT_WRITEDATA, ( void * )&strResponse );
        curl_easy_setopt( curl, CURLOPT_NOSIGNAL, 1 );

        if( NULL == pCaPath )
        {
            curl_easy_setopt( curl, CURLOPT_SSL_VERIFYPEER, false );
            curl_easy_setopt( curl, CURLOPT_SSL_VERIFYHOST, false );
        }
        else
        {
            curl_easy_setopt( curl, CURLOPT_SSL_VERIFYPEER, true );
            curl_easy_setopt( curl, CURLOPT_CAINFO, pCaPath );
        }

        curl_easy_setopt( curl, CURLOPT_CONNECTTIMEOUT, m_nConnectTimeout );
        curl_easy_setopt( curl, CURLOPT_TIMEOUT, m_nTimeout );

        if( getCookies )
        {
            curl_easy_setopt( curl, CURLOPT_COOKIEFILE, "cookies.txt" );
            curl_easy_setopt( curl, CURLOPT_COOKIEJAR, "cookies.txt" );
        }

        res = curl_easy_perform( curl );
        curl_easy_cleanup( curl );
    }
    catch( ... )
    {
        //ASSERT(FALSE);
        LOG( INFO )  << __FUNCTION__ << " -----Gets--catch" ;
    }

    return res;
}


bool HttpUtility::set_multi_init()
{
    m_multi_handle = curl_multi_init();

    if( m_multi_handle == NULL )
    {
        return false;
    }

    return true;
}

bool HttpUtility::set_curl_multi_perform()
{
    bool res = true;
    int running_handle_count;

    while( CURLM_CALL_MULTI_PERFORM == curl_multi_perform( m_multi_handle, &running_handle_count ) )
    {
        continue;
    }

    while( running_handle_count )
    {
        timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        int max_fd;
        fd_set fd_read;
        fd_set fd_write;
        fd_set fd_except;
        FD_ZERO( &fd_read );
        FD_ZERO( &fd_write );
        FD_ZERO( &fd_except );
        curl_multi_fdset( m_multi_handle, &fd_read, &fd_write, &fd_except, &max_fd );
        int return_code = select( max_fd + 1, &fd_read, &fd_write, &fd_except, &tv );

        if( -1 == return_code )
        {
            //cerr << "select error." << endl;
            res = false;
            break;
        }
        else
        {
            while( CURLM_CALL_MULTI_PERFORM == curl_multi_perform( m_multi_handle, &running_handle_count ) )
            {
                continue;
            }
        }
    }

    return res;
}

bool HttpUtility::add_EasyToMulti( std::string sUrl, std::string &data )
{
    bool res = true;

    try
    {
        CURL *easyHandle = NULL;
        easyHandle = curl_easy_init();

        if( easyHandle != NULL )
        {
            curl_easy_setopt( easyHandle, CURLOPT_URL, sUrl.c_str() );
            curl_easy_setopt( easyHandle, CURLOPT_WRITEFUNCTION, OnCallBackFunc );
            curl_easy_setopt( easyHandle, CURLOPT_WRITEDATA, ( void * )&data );
            curl_easy_setopt( easyHandle, CURLOPT_NOSIGNAL, 1 );
            curl_easy_setopt( easyHandle, CURLOPT_CONNECTTIMEOUT_MS, m_nConnectTimeout );
            curl_easy_setopt( easyHandle, CURLOPT_TIMEOUT_MS, m_nTimeout );
            curl_easy_setopt( easyHandle, CURLOPT_SSL_VERIFYPEER, 0 );
            curl_easy_setopt( easyHandle, CURLOPT_SSL_VERIFYHOST, 0 );
            m_curl.push_back( easyHandle );

            if( m_multi_handle != NULL )
            {
                curl_multi_add_handle( m_multi_handle, easyHandle );
            }
            else
            {
                res = false;
            }
        }
        else
        {
            res = false;
        }
    }
    catch( ... )
    {
        //ASSERT(FALSE);
        LOG( INFO )  << __FUNCTION__ << " -----add_EasyToMulti--catch" ;
    }

    return res;
}

bool HttpUtility::clean_curl_multi_perform()
{
    for( auto Item = m_curl.begin(); Item != m_curl.end(); ++Item )
    {
        if( *Item != NULL )
        {
            curl_easy_cleanup( *Item );
        }
    }

    curl_multi_cleanup( m_multi_handle );
    return true;
}