/***
 *       Filename:  ui_linux.cpp
 *
 *    Description:  UI in linux.
 *
 *        Version:  0.0.1
 *        Created:  2017-10-06
 *       Revision:  none
 *
 *         Author:  Dilawar Singh <dilawars@ncbs.res.in>
 *   Organization:  NCBS Bangalore
 *
 *        License:  GNU GPL2
 */


#include "ui_unix.h"

#include "../core/main_loop.h"
#include "../core/ConfigManager.h"
#include "../actions/linux.h"
#include "../external/plog/include/plog/Log.h"

#include "etbapplication.h"
#include <boost/filesystem.hpp>

#include <iostream>
#include <ctime>
#include <thread>
#include <chrono>
#include <memory>

using namespace std;

extern bool show_user_face_;
extern bool finished_;

extern double time_to_process_one_frame_;

extern ConfigManager config_manager_;

unique_ptr<ETBApplication> pApp_;

static bool callback_started_ = false;

bool callback( int arg  )
{
    // If callback_started_ is still true that means previous call is not
    // complete yet. Don't do anything till previous call returns.
    if( callback_started_ )
    {
        cout << '|';
        cout.flush( );
        return true;
    }

    callback_started_ = true;
    auto t0 = std::chrono::system_clock::now( );
    process_frame( );
    auto t1 = std::chrono::system_clock::now( );
    time_to_process_one_frame_ = diff_in_ms( t1, t0 );

    std::this_thread::sleep_for(std::chrono::milliseconds( max(10, 100 - (int)time_to_process_one_frame_ ) ));
    callback_started_ = false;

    return true;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis This function starts UI. Must be called after camera is initialized
 * properly.
 *
 * @Param argc
 * @Param argv[]
 *
 * @Returns Infinite loop.  
 */
/* ----------------------------------------------------------------------------*/
int unix_ui( int argc, char* argv[] )
{
    LOG_INFO << "Constructing applet" << endl;

    string iconPath = config_manager_.getIconpath( );
    LOG_INFO << "Using icon path " << iconPath;

    // Add a callback function.
    sigc::slot<bool> loop_slot = sigc::bind( sigc::ptr_fun( callback ), 0 );

    // Call every 100 ms and no earlier.
    sigc::connection conn = Glib::signal_timeout().connect( loop_slot, 150 );

    pApp_.reset(ETBApplication::create());

    pApp_->run(pApp_->getWindow());

    return 1;

}

// Show user face in main window.
bool show_user_face( const cv::Mat& gray )
{
    pApp_->show_user_face( gray );
    return true;
}

bool show_icon( )
{
    pApp_->show_icon( );
    return true;
}
