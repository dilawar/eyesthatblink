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
#include "etbapplication.h"

#include "plog/Log.h"
#include <boost/filesystem.hpp>

#include <iostream>
#include <ctime>
#include <thread>
#include <chrono>

using namespace std;

extern bool show_user_face_;
extern bool finished_;

extern double time_to_process_one_frame_;

extern ConfigManager config_manager_;

#ifdef WITH_GTK3
Glib::RefPtr<ETBApplication> pApp_ ;
#elif WITH_GTK2
ETBApplication* pApp_;
#endif

bool callback( int arg  )
{
    auto t0 = std::clock( );
    process_frame( );
    auto t1 = clock( );
    time_to_process_one_frame_ = 1000.0 * ( t1 - t0 ) / CLOCKS_PER_SEC;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
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
    // Call this function every 1.5 times it takes to process one frame. This
    // must not slow down the system. 
    sigc::slot<bool> loop_slot = sigc::bind( sigc::ptr_fun( callback ), 0 );
    sigc::connection conn = Glib::signal_timeout().connect( 
            loop_slot
            , 1.5 * time_to_process_one_frame_ 
            );

#ifdef WITH_GTK2
    Gtk::Main initGui( argc, argv );
#endif


#ifdef WITH_GTK3
    pApp_ = ETBApplication::create( );
    Gtk::Main::run( *pApp_ );
#elif WITH_GTK2
    pApp_ = new ETBApplication( );
    Gtk::Main::run( *pApp_ );
#endif

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
