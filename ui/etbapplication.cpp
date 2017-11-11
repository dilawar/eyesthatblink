/***
 *       Filename:  etbapplication.cpp
 *
 *    Description:  App.
 *
 *        Version:  0.0.1
 *        Created:  2017-10-27
 *       Revision:  none
 *
 *         Author:  Dilawar Singh <dilawars@ncbs.res.in>
 *   Organization:  NCBS Bangalore
 *
 *        License:  GNU GPL2
 */

#include <iostream>
using namespace std;

#include "etbapplication.h"
#include "etbwindow.h"
#include "plog/Log.h"
#include "../core/ConfigManager.h"

namespace bfs = boost::filesystem;

extern ConfigManager config_manager_;

ETBApplication::ETBApplication()
#ifdef WITH_GTK3
    : Gtk::Application("org.dilawar.application")
#elif WITH_GTK2
#endif 
{
    try {
    // Load default values.
    show_user_face_ = config_manager_.getValue<bool>( 
            "global.show_user_face" );
    user_has_small_eyes_ = config_manager_.getValue<bool>( 
            "global.user_has_small_eyes" );
    user_wearning_glasses_ = config_manager_.getValue<bool>( 
            "global.user_wearning_glasses" ); 
    } catch( ... ) {
        LOG_INFO << "Failed to load configuration values";
        show_user_face_ = true;
        user_has_small_eyes_ = false;
        user_wearning_glasses_ = false;
    }

#ifdef WITH_GTK3
    Glib::set_application_name("Eyes That Blink");
#elif WITH_GTK2
    create_window( );
#endif
}

Glib::RefPtr<ETBApplication> ETBApplication::create()
{
    return Glib::RefPtr<ETBApplication>( new ETBApplication() );
}

void ETBApplication::toggleSmallEyeOption( )
{
    bool value = smallEye.get_active( );
    config_manager_.setValue<bool>( "global.user_has_small_eyes", value );
    reload_eye_cascade( );
    config_manager_.writeConfigFile( );
}


void ETBApplication::toggleEyeGlassOption( )
{
    bool value = glasses.get_active( );
    LOG_INFO << "Setting 'user wearing glasses?' to " << value;
    config_manager_.setValue<bool>( "global.user_wearing_glasses", value );
    reload_eye_cascade( );
    config_manager_.writeConfigFile( );
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis  When pressed show user face.
 *
 * @Param menuitem
 */
/* ----------------------------------------------------------------------------*/
void ETBApplication::toggleShowUserFace( )
{
    LOG_INFO << "Toggling show user face ";
    bool val = ! show_user_face_;
    config_manager_.setValue<bool>( "global.show_user_face", val );
    show_user_face_ = val;
}

void ETBApplication::on_startup()
{
#ifdef WITH_GTK3
    //Call the base class's implementation:
    Gtk::Application::on_startup();

    add_action("quit",
               sigc::mem_fun(*this, &ETBApplication::on_action_quit) );

    auto app_menu = Gio::Menu::create();
    app_menu->append("_Quit", "app.quit");

    set_app_menu(app_menu);
#elif WITH_GTK2
    LOG_DEBUG << "Using GTK2. Doing nothing here";
#endif

}

void ETBApplication::setBlinkThresholdValue( )
{
    double value = threshold.get_value( );
    LOG_INFO << "Setting threshold value to " << value;
    config_manager_.setBlinkThreshold( value );
    config_manager_.writeConfigFile( );
}


/* --------------------------------------------------------------------------*/
/**
 * @Synopsis  Create window.
 */
/* ----------------------------------------------------------------------------*/
void ETBApplication::create_window()
{
#if WITH_GTK3
    window.set_default_size(300, 200);
    window.signal_hide().connect(
            sigc::bind( 
                sigc::mem_fun(*this, &ETBApplication::on_window_hide)
                , &window
                )
            );
#elif WITH_GTK2
    set_default_size(300, 200);
    signal_hide().connect(
            sigc::bind( 
                sigc::mem_fun(*this, &ETBApplication::on_window_hide)
                , this
                )
            );
#endif
    // Table.
    table.resize( 8, 1 );

    // Small eyes button.
    smallEye.set_label( "Small eyes" );
    if( config_manager_.getValue<bool>( "global.user_has_small_eyes" ) == true )
        smallEye.set_active( );

    showUserFace.set_label( "Show my face" );
    showUserFace.set_active( config_manager_.getValue<bool>( "global.show_user_face" ) );
    showUserFace.signal_toggled( ).connect( 
            sigc::mem_fun( *this, &ETBApplication::toggleShowUserFace )
            );
    showUserFace.show( );
    table.attach( showUserFace, 0, 1, 0, 1 );

    smallEye.signal_toggled( ).connect( 
            sigc::mem_fun( *this, &ETBApplication::toggleSmallEyeOption )
            );
    smallEye.show( );
    table.attach( smallEye, 0, 1, 1, 2 );

    // Glasses.
    glasses.set_label( "Wearing glasses? " );
    glasses.set_active( 
            config_manager_.getValue<bool>( "global.user_wearing_glasses" )
            );

    glasses.signal_toggled( ).connect( 
            sigc::mem_fun( *this, &ETBApplication::toggleEyeGlassOption )
            );
    glasses.show( );
    table.attach( glasses, 0, 1, 2, 3);

    label.set_label( "Blinks per min" );
    label.show( );
    thresBox.pack_start( label ); 

    threshold.set_range( 10, 20);
    threshold.set_value( config_manager_.getBlinkPerMinuteThreshold( ) );

    threshold.signal_value_changed( ).connect( 
                sigc::mem_fun( *this, &ETBApplication::setBlinkThresholdValue )
            );
    thresBox.pack_start( threshold );
    threshold.show( );
    table.attach( thresBox, 0, 1, 3, 4);
    thresBox.show( );

    table.attach( image, 0, 1, 4, 7);
    image.show( );

#if WITH_GTK3
    window.add( table );
#elif WITH_GTK2
    add(table);
#endif
    table.show( );

#ifdef WITH_GTK3
    // Make sure that the application runs for as long this window is 
    add_window(window);
    window.show( );
#endif
    show( );
}

void ETBApplication::on_window_hide(Gtk::Window* window)
{
    LOG_DEBUG << "Hiding window";
    delete window;
    close_camera( );

#ifdef WITH_GTK3
    quit( );
#elif WITH_GTK2
    Gtk::Main::quit( );
#endif
}

void ETBApplication::on_activate()
{
    //std::cout << "debug1: " << G_STRFUNC << std::endl;
    // The application has been started, so let's show a window.
    // A real application might want to reuse this "empty" window in on_open(),
    // when asked to open a file, if no changes have been made yet.
    create_window();
}


void ETBApplication::on_action_quit()
{
    
    LOG_DEBUG << "Closing application";

    close_camera( );

#ifdef WITH_GTK3
    quit(); // Not really necessary, when Gtk::Widget::hide() is called.

    // Gio::Application::quit() will make Gio::Application::run() return,
    // but it's a crude way of ending the program. The window is not removed
    // from the application. Neither the window's nor the application's
    // destructors will be called, because there will be remaining reference
    // counts in both of them. If we want the destructors to be called, we
    // must remove the window from the application. One way of doing this
    // is to hide the window.
    auto windows = get_windows();
    if (windows.size() > 0)
        windows[0]->hide(); // In this simple case, we know there is only one window.
#elif WITH_GTK2
    Gtk::Main::quit( );
#endif
}

void ETBApplication::on_action_print(const Glib::VariantBase& parameter)
{
    std::cout << G_STRFUNC << " Parameter=" << parameter.print() << std::endl;
}


/* --------------------------------------------------------------------------*/
/**
 * @Synopsis  Show user face.
 *
 * @Param gray
 *
 * @Returns   
 */
/* ----------------------------------------------------------------------------*/
bool ETBApplication::show_user_face( const cv::Mat& gray )
{

    if( ! show_user_face_ )
        show_icon( );

    cv::Mat face;
    if( gray.channels( ) == 1 )
        cv::cvtColor( gray, face, cv::COLOR_GRAY2BGR);
    else
        face = gray;

    return false;

    double width = face.cols;
    double fixedWidth = 150;
    cv::resize( face, face, cv::Size( fixedWidth, face.rows * fixedWidth / width ) );

    auto pixbuf = Gdk::Pixbuf::create_from_data( face.data
            , Gdk::COLORSPACE_RGB, false
            , 8, face.cols, face.rows, face.step 
            );
    image.set( pixbuf );
    icon_is_set_ = false;
    return true;
}

bool ETBApplication::show_icon( )
{
    return false;
    if( bfs::exists( ICONFILE_PATH ) )
    {
        auto pixbuf = Gdk::Pixbuf::create_from_file( ICONFILE_PATH, 150, 150 );
        image.set( pixbuf );
        icon_is_set_ = true;
    }
    return true;
}
