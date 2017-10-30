/***
 *       Filename:  etbwindow.cpp
 *
 *    Description:  ETB Window
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
#include "etbwindow.h"

ETBWindow::ETBWindow() : 
    pImage_( nullptr )
{
    set_title("Eyes That Blink");

#ifdef WITH_GTK3
    // We can use add_action because Gtk::ApplicationWindow derives from ActionMap.
    // This Action Map uses a "win." prefix for the actions.
    // Therefore, for instance, "win.new", is used in ETBApplication::on_startup()
    // to layout the menu.
    add_action("new",
               sigc::mem_fun(*this, &ETBWindow::on_action_something) );
    add_action("close",
               sigc::mem_fun(*this, &ETBWindow::on_action_close) );
    add_action("cut",
               sigc::mem_fun(*this, &ETBWindow::on_action_something) );
    add_action("copy",
               sigc::mem_fun(*this, &ETBWindow::on_action_something) );
    add_action("paste",
               sigc::mem_fun(*this, &ETBWindow::on_action_something) );
    add_action("send-note",
               sigc::mem_fun(*this, &ETBWindow::on_action_send_notification) );
    add_action("withdraw-note",
               sigc::mem_fun(*this, &ETBWindow::on_action_withdraw_notification) );
#endif
}

void ETBWindow::on_action_something()
{
    std::cout << G_STRFUNC << std::endl;
}

void ETBWindow::on_action_close()
{
    std::cout << G_STRFUNC << std::endl;

    hide();
}

void ETBWindow::on_action_send_notification()
{
    std::cout << G_STRFUNC << std::endl;

    auto note = Gio::Notification::create("Unimportant message!");
    note->set_body("Notification from " + Glib::get_application_name());
    note->add_button("Print", "app.print", Glib::ustring("Hello, world!"));
    note->add_button("Quit Application", "app.quit");
#if WITH_GTK3
    get_application()->send_notification("note", note);
#endif
}

void ETBWindow::on_action_withdraw_notification()
{
    std::cout << G_STRFUNC << std::endl;

#ifdef WITH_GTK3
    get_application()->withdraw_notification("note");
#endif
}
