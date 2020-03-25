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

class ETBApplication: public Gtk::Application
{

protected:
    ETBApplication();

public:
    static ETBApplication* create();
    bool show_user_face( const cv::Mat& gray );

    bool show_icon( );

    void create_window();

    void on_window_hide(Gtk::Window* window);

    void on_action_quit();
    void on_action_print(const Glib::VariantBase& parameter);

    void setShowUserFace();
    void setEyeGlassOption();
    void setSmallEyeOption();

    void setBlinkThresholdValue();

    Gtk::Window& getWindow();

protected:

    void on_activate() override;
    void on_startup() override;


private:
    Gtk::Window window;
    Gtk::CheckButton showUserFace;
    Gtk::Image image_;
    Gtk::VBox vbox;

    Gtk::Table table_;

    Gtk::CheckButton smallEye;
    Gtk::CheckButton glasses;
    Gtk::HBox thresBox;
    Gtk::Button closeButton;
    Gtk::HScale threshold;
    Gtk::Label label;

    bool show_user_face_;
    bool user_has_small_eyes_;
    bool user_wearning_glasses_;

    bool icon_is_set_;

    double user_blink_thres_;

};

#endif /* GTKMM_EXAMPLEAPPLICATION_H */
