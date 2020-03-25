/*
 * =====================================================================================
 * 
 *
 *       Filename:  globals.c
 *
 *    Description:  Global variables.
 *
 *        Version:  1.0
 *        Created:  Sunday 16 April 2017 04:05:35  IST
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Dilawar Singh (), dilawars@ncbs.res.in
 *   Organization:  NCBS Bangalore
 *
 * =====================================================================================
 */

#include "globals.h"
#include "opencv2/opencv.hpp"

bool away_ = false;
double fps_ = 20.0;

cv::Mat pupil_template_;

std::array<cv::Rect, 2> eye_rects_;

double pupil_radius_ = 0.0;

size_t total_frames_ = 0;

// Time to process one frame (in ms ).
double time_to_process_one_frame_ = 100.0;

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis  When set to yes, show user face.
 */
/* ----------------------------------------------------------------------------*/
bool show_user_face_ = false;
bool finished_ = false;

// Does user have small eyes.
bool small_eyes_ = false;
bool wearing_glasses_ = false;

ConfigManager config_manager_;

ActionManager am_;

cv::VideoCapture cap_;

boost::program_options::variables_map vm_;
