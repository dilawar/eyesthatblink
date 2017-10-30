/*
 * =====================================================================================
 *
 *       Filename:  blink_detector.hpp
 *
 *    Description:  Detect blinks.
 *
 *        Version:  1.0
 *        Created:  Tuesday 18 April 2017 09:40:52  IST
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Dilawar Singh (), dilawars@ncbs.res.in
 *   Organization:  NCBS Bangalore
 *
 * =====================================================================================
 */


#ifndef  blink_detector_INC
#define  blink_detector_INC

#include "opencv2/imgproc/imgproc.hpp"
#include "globals.h"

#include <utility>

using namespace std;

enum method_t_ { 
    COUNT_DARK_PIXAL            // Works badly
    , OPTICAL_FLOW              // Doesn't work at all
    , PUPIL_DETECTION   
    };

cv::Mat find_eyes( cv::Mat & face );

void crop_face( cv::Mat& face );

void preprocess( cv::Mat& eyes );

double pupil_weight( cv::Mat eye, cv::Point center, int radius );

bool detect_blink( cv::Mat & img, const method_t_ method );

#endif   /* ----- #ifndef blink_detector_INC  ----- */
