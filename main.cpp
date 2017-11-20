/**
*       Filename:  main.cpp
*
*    Description:  Entry point of the application.
*
*        Version:  0.0.1
*        Created:  2017-04-16
*       Revision:  none
*
*         Author:  Dilawar Singh <dilawars@ncbs.res.in>
*   Organization:  NCBS Bangalore
*
 *        License:  GNU GPL2
 */


#include <iostream>

#include "core/globals.h"
#include "core/main_loop.h"
#include "plog/Log.h"
#include "config.h"
#include "core/helpers.h"
#include "ui/ui_unix.h"

#include <boost/filesystem.hpp>
#include <plog/Appenders/ConsoleAppender.h>

using namespace std;

/**
 * @function main
 */
int main( int argc, char* argv[] )
{
    // No log should be written to stdout.
    boost::filesystem::path logpath( expand_user( LOG_FILE_PATH ) );
    static plog::RollingFileAppender<plog::CsvFormatter> 
        fileAppender( logpath.c_str( ), 8000, 3 );

#ifndef NDEBUG
    static plog::ConsoleAppender<plog::TxtFormatter> consoleAppender;
    plog::init(plog::debug, &consoleAppender);
#endif

    LOG_INFO << "Initializing ";

    init_camera( );

    unix_ui( argc, argv );

    return 0;
}


