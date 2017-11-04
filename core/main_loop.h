/***
 *       Filename:  main_loop.h
 *
 *    Description:  description
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

#ifndef MAIN_LOOP_H
#define MAIN_LOOP_H

#include "globals.h"

/** Global variables */

void init_camera( );

void close_camera( );

bool process_frame(  );

bool locate_pupil( const cv::Mat face );

void reload_eye_cascade( );

cv::Mat find_face( cv::Mat frame, int method, bool show );

void update_config_file( );

bool show_user_face( const cv::Mat& face );

bool show_icon( );

#endif /* end of include guard: MAIN_LOOP_H */
