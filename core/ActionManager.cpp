/*
 * =============================================================================
 *
 *       Filename:  ActionManager.cpp
 *
 *    Description:  ActionManager class implementation.
 *
 *        Version:  1.0
 *        Created:  Wednesday 19 April 2017 03:48:42  IST
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Dilawar Singh (), dilawars@ncbs.res.in
 *   Organization:  NCBS Bangalore
 *
 * =============================================================================
 */

#include <iostream>
#include <sstream>
#include <future>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/numeric/conversion/cast.hpp>

#if USE_BOOST_PROCESS
#include <boost/process.hpp>
#endif

#if OS_IS_UNIX
#include "../actions/xbacklight.h"
#include <libnotify/notify.h>
#elif OS_IS_APPLE
#endif

#include "globals.h"
#include "plog/Log.h"

using namespace std;

extern unique_ptr<ConfigManager> pConfigManager_;

time_t getCurrentTime( )
{
    // get current time
    time_t nowtime;
    time(&nowtime);
    return nowtime;
}


/*
 *--------------------------------------------------------------------------------------
 *       Class:  ActionManager
 *      Method:  ActionManager
 * Description:  constructor
 *--------------------------------------------------------------------------------------
 */
ActionManager::ActionManager()
{
    n_blinks_ = 0;
    blink_rate_ = 0.0;

    last_blink_time_ = std::chrono::system_clock::now( );
    start_time_ = std::chrono::system_clock::now( );
    current_time_ = std::chrono::system_clock::now( );

    blinks_.push_back( make_pair( start_time_,current_time_));

    // Initialize data file.
    data_file_ = bfs::path( expand_user( DATA_FILEPATH ) );
    config_file_ = bfs::path( expand_user( CONFIG_FILEPATH ) );

    LOG_INFO << "|| Initializing Action Manager";

    initialize( );

}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis  Initialize everything.
 */
/* ----------------------------------------------------------------------------*/
void ActionManager::initialize( void )
{
    // initialize datafile here.

    if( ! bfs::exists( data_file_ ) )
        auto res = bfs::create_directories( data_file_.parent_path( ) );

    if( bfs::exists( config_file_ ) )
    {
        modification_times_[0] = bfs::last_write_time( config_file_ );
        modification_times_[1] = bfs::last_write_time( config_file_ );
    }
    else
    {
        modification_times_[0] = getCurrentTime( ); 
        modification_times_[1] = getCurrentTime( );
    }

    cout << "Blink threshold is " << pConfigManager_->getBlinkThreshold( ) << endl;
    brightness_ = 0.0;
    displays_ = find_display( );
    if( displays_.size( ) > 0 )
        for( auto d : displays_ )
            cout << "||| Display " << d << endl;
    else
        cout << "|| Err: No display found" << endl;


}  /* -----  end of method ActionManager::ActionManager  (constructor)  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  ActionManager
 *      Method:  ActionManager
 * Description:  copy constructor
 *--------------------------------------------------------------------------------------
 */
ActionManager::ActionManager ( const ActionManager &other )
{
} 

/*
 *--------------------------------------------------------------------------------------
 *       Class:  ActionManager
 *      Method:  ~ActionManager
 * Description:  destructor
 *--------------------------------------------------------------------------------------
 */ ActionManager::~ActionManager ()
{
} 

/*
 *--------------------------------------------------------------------------------------
 *       Class:  ActionManager
 *      Method:  operator =
 * Description:  assignment operator
 *--------------------------------------------------------------------------------------
 */
ActionManager&
ActionManager::operator = ( const ActionManager &other )
{
    if ( this != &other )
    {
    }
    return *this;
} 

/**
 * @brief Send message to user.
 *
 * @param msg
 */
void ActionManager::alert( const string& what )
{

#if OS_IS_UNIX 
    notify_init( "Eyes That Blink" );
    NotifyNotification * note = notify_notification_new( 
            "Alert", what.c_str(), "dialog-information" 
            );
    notify_notification_set_timeout( note, 1000 );
    notify_notification_show( note, NULL);

#elif OS_IS_APPLE 
#endif  // OS_IS_LINUX

    LOG_INFO << "Alerting " << what << endl;
    return;
}

/**
 * @brief Compute the average blink activity in given duration.
 *
 * @param interval_in_sec
 *
 * @return  average blink activity  = eye lid close time / total time.
 */
double ActionManager::blink_activity_in_interval(const double interval_in_sec )
{
    current_time_ = std::chrono::system_clock::now( );
    double ms = 1000.0 * interval_in_sec;
    double totalTime = min( 
            1.0 * diff_in_ms( current_time_, blinks_[0].first )
            , ms
            );

    double totalEyeLidClosedTime = 0.0;

    for( auto it = blinks_.rbegin(); it != blinks_.rend(); ++it )
    {
        totalEyeLidClosedTime += diff_in_ms( it->second, it->first );
        if( diff_in_ms( current_time_, it->first) > ms )
            break;
    }

    return totalEyeLidClosedTime / totalTime;
}

void ActionManager::insert_state( const time_type_& t, const status_t_ st )
{

    static time_type_ last_time_stamp = t;
    static bool blink_start = false;
    current_time_ = t;

    if( st == AWAY )
    {
        // Durating away time, we assume that user is blinking at normal rate.
        blink_start = false;
        prev_status_ = st;
        auto diffT = diff_in_ms( t, last_time_stamp );

        // Normally user keeps her eyes close for 7.5% of time.
        total_blink_activity += AVG_EYELID_CLOSE_TIME * diffT ;
        last_time_stamp = t;
        return;
    }

    // If it is not away, lets look for blink.
    if( st == CLOSE && (! blink_start ) )
    {
        blink_start_time_ = t;
        blink_start = true;
    }

    // Analyze after every blink.
    double runningTime = diff_in_ms(t,  start_time_ );
    if( blink_start && OPEN == st )
    {
        // Get value in microseconds.
        double duration = diff_in_ms( t, blink_start_time_ );

        // If blink duration is less than 150 ms; then it is face. mostly due to
        // noise in acquired frame.
        if( duration < 150.0 )
            return;

        blinks_.push_back( make_pair(blink_start_time_, t) );
        blink_start = false;
        n_blinks_ += 1;
        total_blink_activity += duration;
        // Now compute the time
        last_blink_time_ = t;

        // Store the avg activity whenever there is a blink.
        running_avg_activity_ = total_blink_activity / runningTime;
        running_avg_activity_in_interval_ = blink_activity_in_interval( 600 );

        // Write data line.
        write_data_line( );
    }

    // Analyze in every loop.
    running_avg_activity_ = total_blink_activity / runningTime;
    running_avg_activity_in_interval_ = blink_activity_in_interval( 600 );

    /*-----------------------------------------------------------------------------
     *  Trigger notification. If previous notification was less than 10 seconds
     *  ago, do not notify again.
     *-----------------------------------------------------------------------------*/
    static time_type_ last_nofified_on = start_time_;

    // NO notification for first 5 seconds no matter what.
    if( diff_in_ms( current_time_, start_time_ ) < 5000 )
        return;

    // Compare so  we can alert the user.
    if( running_avg_activity_in_interval_ < pConfigManager_->getBlinkThreshold( ) )
    {
        if( diff_in_ms( t, last_nofified_on ) > 10000 )
        {
            LOG_INFO << "Alerting user. Threshold " 
                     << pConfigManager_->getBlinkThreshold( );

            alert(  "You are not blinking enough" );
            last_nofified_on = t;
            linux_set_brightness( );
        }
    }
    else
    {
        linux_set_brightness( );
    }
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis  The last value in line is fraction of time for which eyes lids were
 * closed.
 *
 * TIMESTAMP, NUMBER OF BLINKS, AVERAGE EYELID CLOSED TIME, FRACTION OF TIME
 * EYELID WAS really closed.
 */
/* ----------------------------------------------------------------------------*/
void ActionManager::write_data_line( )
{
    auto t = boost::posix_time::microsec_clock::universal_time();
    string msg = boost::posix_time::to_iso_extended_string(t);
    msg += "," + std::to_string( n_blinks_ );
    msg += "," + std::to_string( running_avg_activity_ );
    msg += "," + std::to_string( running_avg_activity_in_interval_ );
    
    // Open file and write it.
    // Write only when there is a blink. 
    // Write only one line.
    data_file_h_.open( data_file_.c_str( ) ); //, std::ios_base::app );
    data_file_h_ << msg << endl;
    data_file_h_.close( );

    // Write data to stdout.
    cout << "DATA: " << msg << endl;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis  Poll the config file, if something has changed by other
 * application, reload it.
 */
/* ----------------------------------------------------------------------------*/
void ActionManager::update_config_file( )
{
    // No config file yet. Continue.
    if( ! bfs::exists( config_file_ ) )
        return;

    // get current time
    time_t nowtime = getCurrentTime( );

    modification_times_[0] = modification_times_[1];
    modification_times_[1] = bfs::last_write_time( config_file_ );

    if( std::difftime( modification_times_[1], modification_times_[0] ) > 0.0 )
    {
        cout << "Update config file. " << endl;
        // And reload the config file.
        pConfigManager_->readConfigFile( );

        // To be sure lets not rewrite it.
        modification_times_[0] = modification_times_[1];
    }
}

void ActionManager::linux_set_brightness( double frac )
{
    // Do make small changes.
    double brightness = 1.0, delta = 0.0;
    if( frac < 0.0 )
    {
        delta = max(0.0, 
            running_avg_activity_in_interval_ - pConfigManager_->getBlinkThreshold( ) 
            );
        brightness = 10 * delta + 0.5;
    }
    else
        brightness = frac;

    if( frac >= 1.0 && brightness_ >= 1.0 )
        return;

    int bg = boost::numeric_cast<int>( 100 * brightness );
    if( bg > 150 )
        return;

    assert( bg > 0 );
    stringstream ss;
    ss << bg;
    const char* perc = ss.str( ).c_str( );
    LOG_INFO << "XBACKLIGHT Setting brightness to " << perc;

    std::vector<const char*> args = { "xbacklight", "-set", perc };
    std::async( std::launch::async
            , [args](){ xbacklight_main( 3, args ); }
            );

    // Now set the brightness
    brightness_ = max( brightness, 1.0 );
}
