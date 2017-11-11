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

#include "nana/gui.hpp"
#include "nana/gui/widgets/label.hpp"
#include "nana/gui/widgets/checkbox.hpp"
#include "nana/gui/widgets/picture.hpp"
#include "nana/gui/timer.hpp"

#include "plog/Log.h"
#include <boost/filesystem.hpp>

#include <iostream>
#include <ctime>

using namespace std;

extern bool show_user_face_;
extern bool finished_;

extern double time_to_process_one_frame_;

extern ConfigManager config_manager_;

int nana_callback(  nana::picture& canvas )
{
    auto t0 = std::clock( );
    int fixedWidth = 200;
    cv::Mat face;
    bool draw = process_frame( face );
    if( draw  )
    {
        int width = face.cols;
        cv::resize( face, face, cv::Size( fixedWidth, face.rows * fixedWidth / width ) );
        cv::imwrite( "/tmp/a.png", face );

        uchar* data = face.data;
        int size = face.rows * face.cols;
        
#if 0
        std::cout << "Draw face: nelems " << size <<  std::endl;
        nana::paint::image img;
        img.open( data, size );
        canvas.load( img );
        // for (size_t i = 0; i < size; i++)
        // {
            // cout << data[ i ] << ' ';
        // }
        // cout << endl;
#else
        canvas.load( nana::paint::image( "/tmp/a.png" ) );

#endif
    }
    
    auto t1 = clock( );
    time_to_process_one_frame_ = 1000.0 * ( t1 - t0 ) / CLOCKS_PER_SEC;
    return max( 100, 2 * int( time_to_process_one_frame_ ));
}


// Show user face in main window.
bool show_user_face( const cv::Mat& gray )
{
    //pApp_->show_user_face( gray );
    return true;
}

bool show_icon( )
{
    //pApp_->show_icon( );
    return true;
}

int unix_ui( int argc, char* argv[ ] )
{

    nana::form fm;
    fm.caption( "EyesThatBlink" );
    nana::place layout( fm );
    layout.div( R"(<vertical abc weight=120><pic>)" );

    nana::checkbox eye( fm, "Small Eyes" );
    nana::checkbox glass( fm, "Using Glasses" );
    nana::checkbox showEyes( fm, "Show my eyes" );
    nana::picture canvas( fm, true );
    canvas.load( nana::paint::image( config_manager_.getIconpath() ));
    // canvas.stretchable( true );

    layout[ "abc" ] << eye << glass << showEyes;
    layout[ "pic" ] << canvas;
    layout.collocate( );

    // Add nana timer.
    nana::timer mainThread;
    mainThread.interval( 1000 );
    mainThread.elapse( 
            [&mainThread, &canvas](const nana::arg_elapse& a) {
            int timeout = nana_callback( canvas );
            mainThread.interval( timeout );
            }
        );

    mainThread.start( );

    fm.show( );
    nana::exec( );

    return 0;

}
