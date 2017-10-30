/*
 * =====================================================================================
 *
 *       Filename:  globals.h
 *
 *    Description:  Global variables.
 *
 *        Version:  1.0
 *        Created:  Sunday 16 April 2017 04:05:08  IST
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Dilawar Singh (), dilawars@ncbs.res.in
 *   Organization:  NCBS Bangalore
 *
 * =====================================================================================
 */

#ifndef  globals_INC
#define  globals_INC

#define AVG_EYELID_CLOSE_TIME       0.075
#define LOW_THRESHOLD               0.100
#define VERY_LOW_THRESHOLD          0.020

#include "opencv2/opencv.hpp"

#include <chrono>
#include <array>

#include "ConfigManager.h"

enum status_t_ { OPEN, CLOSE, AWAY, BLINK };

typedef std::chrono::time_point<std::chrono::system_clock> time_type_;

extern bool away_;
extern double fps_;
extern size_t total_frames_;

extern cv::Mat pupil_template_;

// Image stabilization module.
extern std::array<cv::Rect, 2> eye_rects_;
extern double pupil_radius_;

extern size_t total_frames_;
extern double fps_;

// Compute average time to process one frame. We should not call 'callback'
// function sooner than this value.
extern double time_to_process_one_frame_;

// Store rectangle of left and right eye.
extern std::array<cv::Rect, 2> eye_rects_;

extern bool show_user_face_;
extern bool finished_;

// Small eyes.
extern bool small_eyes_;

extern ConfigManager config_manager_;

extern cv::VideoCapture cap_;

// Logging to file.

#endif   /* ----- #ifndef globals_INC  ----- */
