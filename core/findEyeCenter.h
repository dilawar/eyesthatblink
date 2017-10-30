#ifndef EYE_CENTER_H
#define EYE_CENTER_H

#include "opencv2/imgproc/imgproc.hpp"

cv::Point findEyeCenter(cv::Mat face, cv::Rect eye, std::string debugWindow);

cv::Point find_eye_center( cv::Mat eye );

cv::Point find_pupil( cv::Mat leftE, cv::Mat rightE );

#endif
