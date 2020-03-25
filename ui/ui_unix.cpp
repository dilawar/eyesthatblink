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

#include "../actions/linux.h"
#include "../core/ConfigManager.h"
#include "../core/main_loop.h"
#include "../external/plog/include/plog/Log.h"

#include <boost/filesystem.hpp>
#include "etbapplication.h"

#include <chrono>
#include <ctime>
#include <iostream>
#include <memory>
#include <thread>

using namespace std;

extern bool show_user_face_;
extern bool finished_;

extern double time_to_process_one_frame_;

unique_ptr<ETBApplication> pApp_;

extern unique_ptr<ConfigManager> pConfigManager_;


bool fetch_and_process(int arg)
{
    // If callback_started_ is still true that means previous call is not
    // complete yet. Don't do anything till previous call returns.
    static bool wait = false;
    if (wait) {
        return true;
    }

    wait = true;
    //auto t0 = std::chrono::system_clock::now();
    process_frame();
    //auto t1 = std::chrono::system_clock::now();
    //time_to_process_one_frame_ = diff_in_ms(t1, t0);

    // std::this_thread::sleep_for(std::chrono::milliseconds(
    //     max(10, 100 - (int)time_to_process_one_frame_)));
    wait = false;

    return true;
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
int unix_ui()
{
    // Add a callback function.
    sigc::slot<bool> slot = sigc::bind(sigc::ptr_fun(fetch_and_process), 0);
    Glib::signal_timeout().connect(slot, 100);

    pApp_.reset(ETBApplication::create());
    pApp_->run(pApp_->getWindow());
    return 1;
}

// Show user face in main window.
bool show_user_face(const cv::Mat& gray)
{
    pApp_->show_user_face(gray);
    return true;
}

bool show_icon()
{
    pApp_->show_icon();
    return true;
}
