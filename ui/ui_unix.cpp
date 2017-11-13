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
#include "nana/gui/widgets/slider.hpp"
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
        cv::resize( face, face
                , cv::Size( fixedWidth, face.rows * fixedWidth / width ) );
        
        nana::paint::image img;
        vector<uchar> vec;
        cv::imencode( ".png", face, vec );
        img.open( vec.data( ), vec.size( ) );
        canvas.load( img );
    }
    auto t1 = clock( );
    time_to_process_one_frame_ = 1000.0 * ( t1 - t0 ) / CLOCKS_PER_SEC;
    return max( 100, 2 * int( time_to_process_one_frame_ ));
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis  Check if use has small eyes.
 *
 * @Param eye
 */
/* ----------------------------------------------------------------------------*/
void setUserHasSmallEyes( bool value )
{
    config_manager_.setUserHasSmallEyes( value );
}

void setUserWearningGlasses( bool value )
{
    config_manager_.setUserWearningGlasses( value );
}

void setShowUserFace( bool value )
{
    config_manager_.setShowUserFace( value );
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

    // Layout of gui.
    layout.div( R"(<vert <horizontal <vert <a><b>> <vert <pic><c>>> 
        <d weight=20 margin=1>>)" );

    // Checkbox. Now add click signal.
    nana::checkbox eye( fm, "I have small eyes" );
    eye.check( config_manager_.getValue<bool>( KEY_USER_HAS_SMALL_EYESS ));
    eye.events( ).click( [&eye]( ) { setUserHasSmallEyes( eye.checked( ) ); } );

    // Checkbox. Now add if wearning glasses.
    nana::checkbox glass( fm, "I am wearning glasses" );
    glass.check( config_manager_.getValue<bool>( KEY_USER_WEARING_GLASSES ) );
    glass.events( ).click( [&glass]( ) { 
            setUserWearningGlasses( glass.checked( ) ); 
            });

    nana::checkbox showMyFace( fm, "Show my face" );
    showMyFace.check( config_manager_.getValue<bool>( KEY_SHOW_USER_FACE ) );
    showMyFace.events( ).click( [&showMyFace]( ) { 
            setShowUserFace( showMyFace.checked( ) ); 
            } );

    // Add slider.
    nana::slider thres(fm );
    thres.maximum( 12 );
    thres.move_step( true );
    thres.vernier( [](unsigned maximum, unsigned val ) {
            return std::to_string( 10 + val ) + " blinks per min";
            } );

    nana::picture canvas( fm, true );
    canvas.load( nana::paint::image( config_manager_.getIconpath() ));

    layout[ "a" ] << eye;
    layout[ "b" ] << glass;
    layout[ "c" ] << showMyFace;
    layout[ "d" ] << thres;
     
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
