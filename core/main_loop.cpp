/***
 *       Filename:  main_loop.cpp
 *
 *    Description:  main loop which process the frame.
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

#include <iostream>

#include "main_loop.h"
#include "blink_detector.hpp"
#include "constants.h"
#include "globals.h"
#include "ActionManager.h"
#include "ConfigManager.h"
#include <boost/filesystem.hpp>

#if OPENCV_VERSION_MAJOR==3
#include <opencv2/highgui.hpp>
#include <opencv2/objdetect.hpp>
#else
#include <opencv2/highgui/highgui.hpp>
#endif

#include "../ui/ui.h"

namespace bfs = boost::filesystem;

extern ConfigManager config_manager_;

ActionManager am_;

extern cv::VideoCapture cap_;

auto t_ = std::chrono::system_clock::now( );

cv::String face_cascade_name = config_manager_.getCascadeFile( "haarcascade_frontalface_default.xml");
//cv::String face_cascade_name = config_manager_.getCascadeFile( "haarcascade_frontalface_alt2.xml");
cv::String eye_cascade_name = config_manager_.getCascadeFile( "haarcascade_eye.xml" );

cv::CascadeClassifier face_cascade( face_cascade_name );
cv::CascadeClassifier eye_cascade( eye_cascade_name );

cv::Mat frame_;

void signalHandler( int signum )
{
    LOG_INFO << "Caught signal number " << signum;
}

/**
 * @function 
 */
cv::Mat find_face( cv::Mat frame, int method = 1, bool show = false )
{
    std::vector<cv::Rect> faces;

    cv::Mat frame_gray;
    cvtColor( frame, frame_gray, CV_BGRA2GRAY );

    cv::Mat face;

    face_cascade.detectMultiScale( 
            frame_gray, faces, 1.1, 2
            , 0|CV_HAAR_SCALE_IMAGE|CV_HAAR_FIND_BIGGEST_OBJECT
            , cv::Size(30,30)
            );

    if (faces.size() > 0)
    {
        away_ = false;
        face = frame( faces[0] );
    }
    else
        away_ = true;

    return face;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis  Change the cascade file.
 *
 * @Param cascadefile
 */
/* ----------------------------------------------------------------------------*/
void reload_eye_cascade(  )
{
    bool smallEyes = config_manager_.getValue<bool>( "global.user_has_small_eyes" );
    bool wearingGlasses = config_manager_.getValue<bool>( "global.user_wearing_glasses" );
    string cascadefile = config_manager_.getCascadeFile( "haarcascade_eye.xml" );

    if( smallEyes )
        cascadefile = config_manager_.getCascadeFile( "haarcascade_mcs_eyepair_small.xml" );

    if( wearingGlasses )
        cascadefile = config_manager_.getCascadeFile( "haarcascade_eye_tree_eyeglasses.xml" );

    LOG_DEBUG << "Changing cascade file to " << cascadefile;
    bool res = eye_cascade.load( cascadefile );
    if( ! res )
    {
        LOG_ERROR << "Failed to load " << cascadefile;
        throw runtime_error( "failed to load cascade" );
    }
}

/**
 * @brief Generate template for pupil.
 *
 * @param face
 */
bool locate_pupil( const cv::Mat face )
{
    vector< cv::Rect > eyes;
    eye_cascade.detectMultiScale( face, eyes, 1.05, 10  );
    if( eyes.size() < 2 )
        return false;

    // Now sort them so left eye is always on the left.
    sort( eyes.begin(), eyes.end( ), 
            []( const cv::Rect & r1, const cv::Rect & r2 )
                { return r1.x > r2.x; }
        );

    // Else got both eyes.
    double w, h;
    double allR = 0.0;
    for (size_t i = 0; i < 2; i++) 
    {
        w = eyes[ i ].width; 
        h = eyes[ i ].height;
        double r = h / 4.0;
        allR += r;
        cv::Point pupilL = eyes[i].tl( ) + cv::Point( h/2, w/2);
        cv::Point offset( r, r );
        cv::Rect eyeLoc( pupilL - offset, pupilL + offset );
        // Update the eye location.
        eye_rects_[ i ] =  eyeLoc;
    }

    // If eyes are smaller than 5 pixals each then reject.
    if( allR < 10 )
        return false;

    return true;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis  Process a singal frame. Find face and update its value.
 *
 * @Returns True if face with pupils were located, false otherwise.
 */
/* ----------------------------------------------------------------------------*/
bool process_frame( cv::Mat& face )
{
    cap_.read(frame_);

    if( ! frame_.empty() )
    {
        // Resize the frame. Resizing the frame to half speeds up the whole
        // process.
        double rescaleFactor = 480.0 / frame_.cols;
        cv::resize( frame_, frame_, cv::Size(0,0), rescaleFactor, rescaleFactor );
        face = find_face( frame_ );

        if( ! face.empty( ) )
        {
            crop_face( face );
            preprocess( face );

            // Here is a blink. Save the timestamp.
            t_ = std::chrono::system_clock::now( );
            total_frames_ += 1;

            /*--------------------------------------------------------------
             *  Generate tempalte for pupil. If it has not been generated.
             *------------------------------------------------------------*/
            bool islocated = locate_pupil( face );
            if( ! islocated )
                am_.insert_state( t_, CLOSE );
            else
                am_.insert_state( t_, OPEN );

            // Show eyes only in debug mode.
            if( config_manager_.getValue<bool>( "global.show_user_face" ) )
            {
                string msg = std::to_string( am_.n_blinks_ );
                msg += "," + std::to_string( am_.running_avg_activity_ );
                msg += "," + std::to_string( am_.running_avg_activity_in_interval_ );

                // Show the face with rectangle drawn on them.
                cv::rectangle( face, eye_rects_[ 0 ], 255, 1 );
                cv::rectangle( face, eye_rects_[ 1 ], 255, 1 );
                cv::putText( face, msg, cv::Point(10,10)
                        , cv::FONT_HERSHEY_SIMPLEX, 0.3, 255
                        );

                return true;
            }
            else
             return false;
        }
        else
            LOG_INFO << "No face found.";
    }
    else
        am_.insert_state( t_, AWAY );

    return false;
}

void init_camera( )
{
    cap_ = cv::VideoCapture( 0 );

    if( ! cap_.isOpened( ) )
    {
        cerr << "Failed to open default camera. Existing ... " << endl;
        exit( -1 );
    }


}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis  Close camera.
 *
 */
/* ----------------------------------------------------------------------------*/
void close_camera( )
{
    cap_.release( );
}

void update_config_file( )
{
    am_.update_config_file( );
}
