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
#include "../core/main_loop.h"
#include "../core/ConfigManager.h"

namespace bfs = boost::filesystem;

extern unique_ptr<ConfigManager> pConfigManager_;

ETBApplication::ETBApplication() : Gtk::Application("org.dilawar.application")
{
    try {
        // Load default values.
        show_user_face_ = pConfigManager_->getValue<bool>( 
                "global.show_user_face" );
        user_has_small_eyes_ = pConfigManager_->getValue<bool>( 
                "global.user_has_small_eyes" );
        user_wearning_glasses_ = pConfigManager_->getValue<bool>( 
                "global.user_wearning_glasses"); 
    } catch( std::exception& e ) {
        LOG_INFO << "Failed to load configuration values" <<  e.what() << endl;
        show_user_face_ = true;
        user_has_small_eyes_ = false;
        user_wearning_glasses_ = false;
    }
    Glib::set_application_name("Eyes That Blink");
}

ETBApplication* ETBApplication::create()
{
    return new ETBApplication();
}

void ETBApplication::setSmallEyeOption( )
{
    bool value = smallEye.get_active( );
    pConfigManager_->setValue<bool>( "global.user_has_small_eyes", value );
    reload_eye_cascade( );
    pConfigManager_->writeConfigFile( );
}


void ETBApplication::setEyeGlassOption( )
{
    bool value = glasses.get_active( );
    LOG_INFO << "Setting 'user wearing glasses?' to " << value;
    pConfigManager_->setValue<bool>( "global.user_wearing_glasses", value );
    reload_eye_cascade( );
    pConfigManager_->writeConfigFile( );
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis  When pressed show user face.
 *
 * @Param menuitem
 */
/* ----------------------------------------------------------------------------*/
void ETBApplication::setShowUserFace( )
{
    LOG_INFO << "Toggling show user face ";
    bool val = ! show_user_face_;
    pConfigManager_->setValue<bool>( "global.show_user_face", val );
    show_user_face_ = val;
}

void ETBApplication::on_startup()
{
    //Call the base class's implementation:
    Gtk::Application::on_startup();

    add_action("quit",
               sigc::mem_fun(*this, &ETBApplication::on_action_quit));

    auto app_menu = Gio::Menu::create();
    app_menu->append("_Quit", "app.quit");

    set_app_menu(app_menu);
}

void ETBApplication::setBlinkThresholdValue( )
{
    double value = threshold.get_value( );
    LOG_INFO << "Setting threshold value to " << value;
    pConfigManager_->setBlinkThreshold( value );
    pConfigManager_->writeConfigFile( );
}


/* --------------------------------------------------------------------------*/
/**
 * @Synopsis  Create window.
 */
/* ----------------------------------------------------------------------------*/
void ETBApplication::create_window()
{
    window.set_default_size(300, 200);
    window.signal_hide().connect(
            sigc::bind( 
                sigc::mem_fun(*this, &ETBApplication::on_window_hide)
                , &window
                )
            );
    // Table.
    table_.resize( 8, 1 );

    // Small eyes button.
    smallEye.set_label( "Small eyes" );
    if( pConfigManager_->getValue<bool>( "global.user_has_small_eyes" ) == true )
        smallEye.set_active( );

    showUserFace.set_label( "Show my face" );
    showUserFace.set_active( pConfigManager_->getValue<bool>( "global.show_user_face" ) );
    showUserFace.signal_toggled( ).connect( 
            sigc::mem_fun( *this, &ETBApplication::setShowUserFace )
            );
    showUserFace.show( );
    table_.attach( showUserFace, 0, 1, 0, 1 );

    smallEye.signal_toggled( ).connect( 
            sigc::mem_fun( *this, &ETBApplication::setSmallEyeOption )
            );
    smallEye.show( );
    table_.attach( smallEye, 0, 1, 1, 2 );

    // Glasses.
    glasses.set_label( "Wearing glasses? " );
    glasses.set_active( 
            pConfigManager_->getValue<bool>( "global.user_wearing_glasses" )
            );

    glasses.signal_toggled( ).connect( 
            sigc::mem_fun( *this, &ETBApplication::setEyeGlassOption )
            );
    glasses.show( );
    table_.attach( glasses, 0, 1, 2, 3);

    label.set_label( "Blinks per min" );
    label.show( );
    thresBox.pack_start( label ); 

    threshold.set_range( 10, 20);
    threshold.set_value( pConfigManager_->getBlinkPerMinuteThreshold( ) );

    threshold.signal_value_changed( ).connect( 
                sigc::mem_fun( *this, &ETBApplication::setBlinkThresholdValue )
            );
    thresBox.pack_start( threshold );
    threshold.show( );
    table_.attach( thresBox, 0, 1, 3, 4);
    thresBox.show( );

    table_.attach( image_, 0, 1, 4, 7);
    image_.show( );

    window.add( table_ );
    table_.show( );

    // Make sure that the application runs for as long this window is 
    add_window(window);
    window.show( );
}

void ETBApplication::on_window_hide(Gtk::Window* window)
{
    LOG_DEBUG << "Hiding window";
    close_camera( );
    quit();
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

    quit(); // Not really necessary, when Gtk::Widget::hide() is called.

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
    cv::Mat face;
    if( gray.channels( ) == 1 )
        cv::cvtColor( gray, face, cv::COLOR_GRAY2BGR);
    else
        face = gray;

    double width = face.cols;
    double fixedWidth = 150;
    cv::resize( face, face, cv::Size( fixedWidth, face.rows * fixedWidth / width ) );

    auto pixbuf = Gdk::Pixbuf::create_from_data(face.data
            , Gdk::COLORSPACE_RGB, false
            , 8, face.cols, face.rows, (int)face.step);
    image_.set(pixbuf);
    return true;
}

bool ETBApplication::show_icon( )
{
    auto iconPath = pConfigManager_->getIconpath();
    if(bfs::exists(iconPath))
    {
        auto pixbuf = Gdk::Pixbuf::create_from_file(iconPath.string(), 150, 150 );
        image_.set( pixbuf );
        icon_is_set_ = true;
    }
    return true;
}

Gtk::Window& ETBApplication::getWindow()
{
    return window;
}
