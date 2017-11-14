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
#include "nana/gui/screen.hpp"
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
    static bool iconfileIsShown = false;
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
        iconfileIsShown = false;
    }
    else
    {
        // Put icon.
        if( ! iconfileIsShown )
        {
            canvas.load( nana::paint::image( ICONFILE_PATH ) );
            iconfileIsShown = true;
        }
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

void setBlinkThreshold( int val )
{
    config_manager_.setBlinkThreshold( val );
}

int unix_ui( int argc, char* argv[ ] )
{
#if 0
    // Check the display.
    auto s = nana::screen( );
    cout << "Display index " << s.get_primary( ).get_index( ) << endl;
#endif 

    nana::form fm( {0,0,400,200} );
    fm.caption( "EyesThatBlink" );
    nana::place layout( fm );

    // Layout of gui.
    layout.div( R"(
        <vert gap=5
            <vert gap=5
                <<SmallEye><WearningGlass>> 
                <<Label weight=20%> <Thres weight=80%> > 
            > 
            < <ShowEyes weight=40%> <Canvas weight=60%> >
        >
        )" );


    // Checkbox. Now add click signal.
    nana::checkbox eye( fm, "Small eyes" );
    eye.check( config_manager_.getValue<bool>( KEY_USER_HAS_SMALL_EYESS ));
    eye.events( ).click( [&eye]( ) { setUserHasSmallEyes( eye.checked( ) ); } );

    // Checkbox. Now add if wearning glasses.
    nana::checkbox glass( fm, "I wear glasses" );
    glass.check( config_manager_.getValue<bool>( KEY_USER_WEARING_GLASSES ) );
    glass.events( ).click( [&glass]( ) { 
            setUserWearningGlasses( glass.checked( ) ); 
            });

    nana::checkbox showMyFace( fm, "Show face" );
    showMyFace.check( config_manager_.getValue<bool>( KEY_SHOW_USER_FACE ) );
    showMyFace.events( ).click( [&showMyFace]( ) { 
            setShowUserFace( showMyFace.checked( ) ); 
            } );

    // Add slider and label to it. The offset is 10 which is minimum blink per
    // minute user is allowed.
    nana::label dl( fm, "Set notification level: " );
    nana::slider thres( fm );
    thres.value( config_manager_.getValue<unsigned int>( KEY_BLINK_RATE_PER_MINUTE ) );
    thres.maximum( 12 );
    thres.move_step( true );
    thres.vernier( [](unsigned maximum, unsigned val ) {
            return std::to_string( 10 + val ) + " blinks per min";
            } );
    thres.events( ).click( [&thres] {
            int val = thres.value( ) + 10;
            setBlinkThreshold( val );
            });

    nana::picture canvas( fm, true );
    canvas.load( nana::paint::image( config_manager_.getIconpath() ));


    layout[ "SmallEye" ] << eye;
    layout[ "WearningGlass" ] << glass;

    // Add a textbox as label to slider.
    layout[ "Label" ] << dl;
    layout[ "Thres" ] << thres;

    layout[ "ShowEyes" ] << showMyFace;
    layout[ "Canvas" ] << canvas;
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
