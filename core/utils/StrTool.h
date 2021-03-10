#ifndef __STRING_TOOL_H__
#define __STRING_TOOL_H__
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
namespace strtool
{
    std::string trim( const std::string &str );
    int split( const std::string &str, std::vector<std::string> &ret_, const std::string &sep );
    std::string replace( const std::string &str, const std::string &src, const std::string &dest );

    std::string getCurDateTimeStr( const std::string strFormat );
}
#endif