#define GLOG_NO_ABBREVIATED_SEVERITIES

#include <stdio.h>
#include <fstream>
#include <glog/logging.h>
#include <gflags/gflags.h>
#include "Launch.h"

void SignalHandle( const char *data, int size )
{
    std::string str = std::string( data, size );
    LOG( ERROR ) << str;
}

int main( int argc, char *argv[] )
{
    strThreadLogFlag = "[mainThread] ";
    google::ParseCommandLineFlags( &argc, &argv, true );
    google::InitGoogleLogging( argv[0] );
    google::InstallFailureSignalHandler();
    google::InstallFailureWriter( &SignalHandle );
    LOG( INFO ) << strThreadLogFlag << __FUNCTION__ << " server starting......";
    Launch m_Launch;
    m_Launch.Start();
    google::ShutdownGoogleLogging();
    google::ShutDownCommandLineFlags();
    //google::protobuf::ShutdownProtobufLibrary();
    return 0;
}
