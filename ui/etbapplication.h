/***
 *       Filename:  etbapplication.h
 *
 *    Description:  GUI based on linux.
 *
 *        Version:  0.0.1
 *        Created:  2017-10-29
 *       Revision:  none
 *
 *         Author:  Dilawar Singh <dilawars@ncbs.res.in>
 *   Organization:  NCBS Bangalore
 *
 *        License:  GNU GPL2
 */

#ifndef GTKMM_EXAMPLEAPPLICATION_H
#define GTKMM_EXAMPLEAPPLICATION_H

#include "opencv2/opencv.hpp"
#include "ui_unix.h"
#include <gtkmm.h>

#ifdef WITH_GTK3
class ETBApplication: public Gtk::Application
#elif WITH_GTK2
class ETBApplication: public Gtk::Window 
#endif
{

#ifdef WITH_GTK3
protected:
    ETBApplication();
#elif WITH_GTK2
public:
    ETBApplication( );
#endif

public:
    static Glib::RefPtr<ETBApplication> create();
    bool show_user_face( const cv::Mat& gray );

protected:

#ifdef WITH_GTK3
    void on_activate() override;
    void on_startup() override;
#else
    void on_startup( );
    void on_activate( );
#endif

private:

    void create_window();

    void on_window_hide(Gtk::Window* window);

    void on_action_quit();
    void on_action_print(const Glib::VariantBase& parameter);

    void toggleShowUserFace( );
    void toggleEyeGlassOption( );
    void toggleSmallEyeOption( );

    void setBlinkThresholdValue( );

private:

#ifdef WITH_GTK3
    Gtk::Window window;
#endif
    Gtk::CheckButton showUserFace;
    Gtk::Image image;
    Gtk::VBox vbox;

    Gtk::Table table;

    Gtk::CheckButton smallEye;
    Gtk::CheckButton glasses;
    Gtk::HBox thresBox;
    Gtk::Button closeButton;
    Gtk::HScale threshold;
    Gtk::Label label;

    bool show_user_face_;
    bool user_has_small_eyes_;
    bool user_wearning_glasses_;


    double user_blink_thres_;

};

#endif /* GTKMM_EXAMPLEAPPLICATION_H */
