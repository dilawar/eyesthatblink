/*
 * =====================================================================================
 *
 *       Filename:  blink_detector.cpp
 *
 *    Description:  Detect blink.
 *
 *        Version:  1.0
 *        Created:  Tuesday 18 April 2017 09:45:18  IST
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Dilawar Singh (), dilawars@ncbs.res.in
 *   Organization:  NCBS Bangalore
 *
 * =====================================================================================
 */

#include "blink_detector.hpp"
#include "helpers.h"
#include "stablizer.hpp"

#include <deque>
#include <iostream>
#include <algorithm>
#include <numeric>

#include <opencv2/video/video.hpp>
#include <opencv2/features2d/features2d.hpp>

#define STACK_SIZE  10

using namespace std;

deque<double> signal_( 2 * fps_, 0 );

deque<cv::Point> leftPupilLoc_( 10 );
deque<cv::Point> rightPupilLoc_( 10 );
deque<cv::Mat> frames_( STACK_SIZE );

// Previous and current matrices.
cv::Mat prev_;
cv::Mat curr_;


double pupil_weight( cv::Mat eye, cv::Point center, int radius )
{
    double sum = 0.0;
    for (int i = -radius; i < radius; i++) 
        for (int j = -radius; j < radius; j++) 
        {
            cv::Point p = center + cv::Point( i, j );
            sum += eye.at<uchar>( p );
        }

    // Return per pixal value.
    return 1.0 * sum / ( eye.rows * eye.cols );
}

/**
 * @brief Search for better location in neighbour-hood.
 *
 * @param 
 * @param 
 * @param diameter
 *
 * @return 
 */
void search_in_neighbourhood( 
        const cv::Mat& eye
        , cv::Point& loc
        , double diameter 
        , double* weight = 0
        )
{
    bool foundAnotherMinimum = true;
    double minW = 255.0;
    double r = 2.0;
    while( foundAnotherMinimum )
    {
        foundAnotherMinimum = false;
        for( double theta = 0.0; theta < 360; theta += 45 )
        {
            cv::Point newLoc;
            int xShift = ceil( r * cos( theta ));
            int yShift = ceil( r * sin( theta ));
            newLoc.x = loc.x + xShift;
            newLoc.y = loc.y + yShift;
            double w = pupil_weight( eye, newLoc, diameter /2 );
            if( w < minW )
            {
                minW = w;
                loc = newLoc;
                *weight = minW;
                foundAnotherMinimum = true;
            }

            // Don't go far from center.
            if( cv::norm( loc - newLoc ) >= diameter / 2.0 )
                break;
        }
    }
}

void find_pupil( const cv::Mat eye, const cv::Mat templ
        , double diameter, cv::Point& loc, double* weight = 0 
        )
{
    double r = diameter / 2.0;
    Mat result;
    cv::matchTemplate( eye, templ, result, cv::TM_SQDIFF );

    // Now retunrn the minimum location.
    cv::minMaxLoc( result, 0, 0, &loc, 0 );
    loc += cv::Point( templ.cols / 2, templ.rows / 2 );
    *weight = pupil_weight( eye, loc, diameter / 2 );
}

cv::Mat find_eyes( cv::Mat & face )
{
    int h = face.rows;
    int w = face.cols;

    int x0, y0;
    int eyeW, eyeH;

    // Eyes start from 10% from left and 30 % from top; width and heigh of
    // boudning rectangle is given below.
    x0 = w * 0.15;
    y0 = h * 0.32;
    eyeW = 0.65 * w - 1;
    eyeH = 0.15 * h - 1;
    auto rect = cv::Rect( x0, y0, eyeW, eyeH );
    return face( rect );
}

void crop_face( cv::Mat& face )
{
    int h = face.rows;
    int w = face.cols;
    int x0, y0;
    int eyeW, eyeH;
    // Eyes start from 10% from left and 30 % from top; width and heigh of
    // boudning rectangle is given below.
    x0 = w * 0.15;
    y0 = h * 0.25;
    eyeW = 0.7 * w - 1;
    eyeH = 0.4 * h - 1;
    auto rect = cv::Rect( x0, y0, eyeW, eyeH );
    face = face( rect );
}

void preprocess( cv::Mat& eyes )
{
    cv::cvtColor( eyes, eyes, cv::COLOR_BGR2GRAY );
    cv::equalizeHist( eyes, eyes );
}

bool is_given_std_away( double signal, double thres = 1.5, double atleast = 20 )
{
    double m, u;
    meanStd( signal_, &m, &u );

    // Blink detection. If signal goes above 1 std of mean, call it a blink.
    return( signal > m + thres * u && signal > atleast );

}

/**
 * @brief In this method, we simply count the dark pixals in the image. When the
 * count goes below a threshold level, we declare that there was a blink.
 *
 * @param eyes
 *
 * @return  True if there was a blink.
 *
 * NOTES: Simple mean does not work; no significant change in signal. Lets just
 * count the pixals below a threshold level.
 * 
 * Even simple tresholding did not work. To improve noise, lets rescale the
 * image to a fixed dimension first.
 *
 * Near the pupil, white area is usually not the brightest pixals either.
 */
bool count_dark_pixals( const cv::Mat eyes, double threshold = 20 )
{

    // We count the dark pixal below a threshold value. There is no direct
    // function call to do this so we use an alternative way. 
    double min, max;
    cv::minMaxLoc( eyes, &min, &max );

    cv::Mat invEyes = ( max - eyes );
    threshold = ( max - threshold );
    cv::threshold( invEyes, invEyes, threshold, max, cv::THRESH_BINARY );

    double signal = cv::sum( eyes )[0];

    signal_.pop_front( );
    signal_.push_back( signal ); 

    return is_given_std_away( signal, 1.5 );
}

cv::Point find_avg_location( const deque<cv::Point> vec )
{

    auto sum = accumulate( vec.begin(), vec.end(), cv::Point(0,0) );
    return cv::Point( sum.x / vec.size(), sum.y / vec.size( ) );
}

cv::Mat pupil_template( double r )
{
    // Template
    int h = 2*r, w = 3*r;
    Mat templ( h, w, CV_8UC1, 255 );
    cv::circle( templ, cv::Point(w/2, h/2), r, 0, -1 );
    cv::circle( templ, cv::Point(w/2, h/2), 2, 255, -1 );
    return templ;
}

/**
 * @brief 
 *
 * @param eyes
 * @param threshold
 *
 * @return 
 * NOTES:
 *  SimpleBlobDetector did not work. It only works with geometric shapes.
 */
bool pupil_detection( const cv::Mat eyes, double threshold = 10 )
{
    double r_ = eyes.rows * 0.25;
    int r = ceil( r_ );
    cv::Mat eyeDebug( eyes );

    vector<cv::Point2f> corners;
    cv::goodFeaturesToTrack( eyes, corners, 2, 0.1, 20 );

    // Sort these points so first one is always the left one.
    sort( corners.begin(), corners.end(),
            [](const cv::Point a, const cv::Point b) { return a.x > b.x; }
            );

    if( corners.size( ) < 2 )
        return false;

    // Put values in deque for computing running average.
    leftPupilLoc_.pop_front( );
    rightPupilLoc_.pop_front( );
    leftPupilLoc_.push_back( corners[0] );
    rightPupilLoc_.push_back( corners[1] );

    auto leftP = find_avg_location( leftPupilLoc_ );
    auto rightP = find_avg_location( rightPupilLoc_ );

    double signalL, signalR;
#if 0
    search_in_neighbourhood( eyes, leftP, 2 * r, &signalL );
    search_in_neighbourhood( eyes, rightP, 2 * r, &signalR );
#else
    // Template
    Mat templ = pupil_template( r );
    find_pupil( eyes.colRange(0, eyes.cols/2), templ, 2*r, leftP, &signalL );
    find_pupil( eyes.colRange(eyes.cols/2, eyes.cols), templ, 2*r, rightP, &signalR );
    rightP += cv::Point( eyes.cols/2, 0 );
#endif

    double signal = signalR + signalL;
    signal_.pop_front( );
    signal_.push_back( signal );

    try 
    {
        bool debug = true;
        if( debug )
        {
            // Constrict left eyes and right eye rectangle.
            cv::Rect leftPupil( max(0, leftP.x - r), max(0,leftP.y -r) , 2*r, 2*r );
            cv::Rect rightPupil( max(0, rightP.x - r), max(0, rightP.y -r) , 2*r, 2*r );
            cv::rectangle( eyeDebug, leftPupil, 255, 1 );
            cv::rectangle( eyeDebug, rightPupil, 255, 1 );
            cv::rectangle( eyeDebug, rightPupil, 255, 1 );
            cv::putText( eyeDebug , std::to_string( (int)signal )
                       , cv::Point( 10, 10 ), cv::FONT_HERSHEY_SIMPLEX
                       , 0.3, 255
                    );

        }

    }
    catch (exception & e )
    {
        cout << "x";
    }

    return is_given_std_away( signal, 1.5, false );
}

/**
 * @brief Return if a blink is found in face.
 *
 * @param face
 * @param method
 *
 * @return 
 */
bool detect_blink( cv::Mat & face, const method_t_ method )
{
    
    /*-----------------------------------------------------------------------------
     *  STEP1 : Locate eyes. Extract two subimages for left eye and right eye.
     *-----------------------------------------------------------------------------*/
    auto eyes = find_eyes( face );

    preprocess( eyes );
    frames_.push_back( eyes );
    frames_.pop_front( );
    total_frames_ += 1;

    if( total_frames_ <= STACK_SIZE )
        return false;

    bool isBlink = false;
    /*-----------------------------------------------------------------------------
     *  Method specific algorithm to detect the blink.
     *-----------------------------------------------------------------------------*/
    if( COUNT_DARK_PIXAL == method )
        isBlink = count_dark_pixals( eyes );
    else if ( PUPIL_DETECTION == method )
        isBlink = pupil_detection( eyes );

    return false;
}

