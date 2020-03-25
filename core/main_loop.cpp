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
#include <boost/filesystem.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/objdetect.hpp>


#include "main_loop.h"
#include "blink_detector.hpp"
#include "constants.h"
#include "globals.h"
#include "ActionManager.h"
#include "ConfigManager.h"

#include "plog/Log.h"

#include "../ui/ui.h"

namespace bfs = boost::filesystem;

extern unique_ptr<ConfigManager> pConfigManager_;
extern unique_ptr<ActionManager> pActionManager_;

extern cv::VideoCapture cap_;

auto t_ = std::chrono::system_clock::now( );

cv::String face_cascade_name = pConfigManager_->getCascadeFile( "haarcascade_frontalface_alt2.xml");
cv::String eye_cascade_name = pConfigManager_->getCascadeFile( "haarcascade_eye.xml" );

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
    auto t0 = std::chrono::system_clock::now( );

    std::vector<cv::Rect> faces;

    cv::Mat frame_gray;
    cvtColor( frame, frame_gray, cv::COLOR_BGRA2GRAY );

    cv::Mat face;

    face_cascade.detectMultiScale( frame_gray, faces, 1.2, 10);

    if (faces.size() > 0)
    {
        away_ = false;
        face = frame( faces[0] );
    }
    else
        away_ = true;

    auto t1 = std::chrono::system_clock::now( );
    LOG_DEBUG << "Time taken ms: " << diff_in_ms( t1, t0 );
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
    bool smallEyes = pConfigManager_->getValue<bool>( "global.user_has_small_eyes" );
    bool wearingGlasses = pConfigManager_->getValue<bool>( "global.user_wearing_glasses" );
    string cascadefile = pConfigManager_->getCascadeFile( "haarcascade_eye.xml" );

    if( smallEyes )
        cascadefile = pConfigManager_->getCascadeFile( "haarcascade_mcs_eyepair_small.xml" );

    if( wearingGlasses )
        cascadefile = pConfigManager_->getCascadeFile( "haarcascade_eye_tree_eyeglasses.xml" );

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
    auto t0 = std::chrono::system_clock::now( );

    vector< cv::Rect > eyes;
    eye_cascade.detectMultiScale( face, eyes, 1.1, 10  );
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

    //auto t1 = std::chrono::system_clock::now( );
    //cout << " EYES time taken " << diff_in_ms( t1,t0 ) << endl;

    // If eyes are smaller than 5 pixals each then reject.
    if( allR < 10 )
        return false;

    return true;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis  Process a sinal frame.
 *
 * @Returns   
 */
/* ----------------------------------------------------------------------------*/
bool process_frame(  )
{
    cap_.read(frame_);

    if( ! frame_.empty() )
    {
        // Resize the frame. Resizing the frame to half speeds up the whole
        // process.
        cv::Mat face;
        double rescaleFactor = 600.0 / frame_.cols;
        cv::resize( frame_, frame_, cv::Size(0,0), rescaleFactor, rescaleFactor );

#if 1
        // Detecting pupil directly is enough.
        face = find_face( frame_ );
#else
        face = frame_;
#endif

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
                pActionManager_->insert_state( t_, CLOSE );
            else
                pActionManager_->insert_state( t_, OPEN );

            // Show eyes only in debug mode.
            if( pConfigManager_->getValue<bool>( "global.show_user_face" ) )
            {
                string msg = std::to_string( pActionManager_->n_blinks_ );
                msg += "," + std::to_string( pActionManager_->running_avg_activity_ );
                msg += "," + std::to_string( pActionManager_->running_avg_activity_in_interval_ );

                // Show the face with rectangle drawn on them.
                cv::rectangle( face, eye_rects_[ 0 ], 255, 1 );
                cv::rectangle( face, eye_rects_[ 1 ], 255, 1 );
                cv::putText( face, msg, cv::Point(10,10)
                        , cv::FONT_HERSHEY_SIMPLEX, 0.3, 255
                        );

                // Show user face in UI.
                show_user_face( face );
            }
            else
            {
                show_icon( );
            }
        }
        else
            LOG_INFO << "No face found.";
    }
    else
    {
        pActionManager_->insert_state( t_, AWAY );
        LOG_INFO << "Empty frame";
    }

    return true;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis  Initialize camera.
 */
/* ----------------------------------------------------------------------------*/
void init_camera( )
{
    cap_ = cv::VideoCapture(0);
    if( ! cap_.isOpened( ) )
    {
        LOG_ERROR << "Failed to open default camera. Existing ... " << endl;
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
    pActionManager_->update_config_file( );
}
