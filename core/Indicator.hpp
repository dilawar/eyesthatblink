/***
 *       Filename:  Indicator.h
 *
 *    Description:  Indicator
 *
 *        Version:  0.0.1
 *        Created:  2017-10-03
 *       Revision:  none
 *
 *         Author:  Dilawar Singh <dilawars@ncbs.res.in>
 *   Organization:  NCBS Bangalore
 *
 *        License:  GNU GPL2
 */

#ifndef INDICATOR_H
#define INDICATOR_H

#ifdef USE_APPINDICATOR
#include <gtk/gtk.h>
#include <libappindicator/app-indicator.h>
#endif

string iconPath = "./data/icon.png";

void init_indicator(int main, char **argc)
{
    cout << "Constructing applet" << endl;

    GtkWidget *indicator_menu;
    GtkWidget *menuItem;

    AppIndicator *indicator;
    GError *error = NULL;

    gtk_init(0, NULL);

    indicator = app_indicator_new("eyesthatblink", iconPath.c_str(),
                                  APP_INDICATOR_CATEGORY_APPLICATION_STATUS);

    app_indicator_set_status(indicator, APP_INDICATOR_STATUS_ACTIVE);
    app_indicator_set_icon(indicator, iconPath.c_str());

    indicator_menu = gtk_menu_new();
    menuItem = gtk_menu_item_new_with_label("menu1");

    // style
    GtkStyle *style;
    style = gtk_rc_get_style(gtk_image_menu_item_new_with_mnemonic("a"));

    gtk_menu_shell_insert(GTK_MENU_SHELL(indicator_menu), menuItem, 0);
    gtk_widget_show(menuItem);

    app_indicator_set_menu(indicator, GTK_MENU(indicator_menu));

    gtk_widget_show_all(indicator_menu);
}

#endif /* end of include guard: INDICATOR_H */
