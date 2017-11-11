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

#ifdef WITH_NANA
#include "nana/gui.hpp"
#include "nana/gui/widgets/label.hpp"
#include "nana/gui/widgets/checkbox.hpp"
#include "nana/gui/widgets/picture.hpp"
#include "nana/gui/timer.hpp"
#endif

#include "plog/Log.h"
#include <boost/filesystem.hpp>

#include <iostream>
#include <ctime>

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
    time_to_process_one_frame_ = 1000.0 * ( t1 - t0 ) * CLOCKS_PER_SEC;
    return true;
}

void nana_callback( const nana::arg_elapse& arg  )
{
    auto t0 = std::clock( );
    process_frame( );
    auto t1 = clock( );
    time_to_process_one_frame_ = 1000.0 * ( t1 - t0 ) * CLOCKS_PER_SEC;
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
int unix_ui_gtk( int argc, char* argv[] )
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

int unix_ui( int argc, char* argv[ ] )
{

#ifdef WITH_NANA


    nana::form fm;
    fm.caption( "EyesThatBlink" );
    nana::place layout( fm );
    layout.div( R"(<vertical abc weight=120><pic>)" );

    nana::checkbox eye( fm, "Small Eyes" );
    nana::checkbox glass( fm, "Using Glasses" );
    nana::checkbox showEyes( fm, "Show my eyes" );
    nana::picture face( fm, true );

    face.load( nana::paint::image( config_manager_.getIconpath() ));

    layout[ "abc" ] << eye << glass << showEyes;
    layout[ "pic" ] << face;
    layout.collocate( );

    // Add nana timer.
    nana::timer mainThread;
    mainThread.interval( 1000 );
    mainThread.elapse( &nana_callback );
    mainThread.start( );

    fm.show( );
    nana::exec( );

    return 0;

#else
    return unix_ui_gtk( argc, argv );
#endif

}
