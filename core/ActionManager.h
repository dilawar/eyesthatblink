/*
 * =====================================================================================
 *
 *       Filename:  ActionManager.hpp
 *
 *    Description:  ActionManager class.
 *
 *        Version:  1.0
 *        Created:  Wednesday 19 April 2017 03:47:29  IST
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Dilawar Singh (), dilawars@ncbs.res.in
 *   Organization:  NCBS Bangalore
 *
 * =====================================================================================
 */

#ifndef  ActionManager_INC
#define  ActionManager_INC

#include <chrono>
#include <array>
#include <vector>
#include <utility>
#include <fstream>
#include <boost/filesystem.hpp>

#include "../actions/linux.h"

namespace bfs = boost::filesystem;

using namespace std;

#include "globals.h"
#include "helpers.h"

/*
 * =====================================================================================
 *        Class:  ActionManager
 *  Description:  
 * =====================================================================================
 */
class ActionManager
{
    public:

        ActionManager ();                             /* constructor      */
        ActionManager ( const ActionManager &other );   /* copy constructor */
        ~ActionManager ();                            /* destructor       */

        ActionManager& operator = ( const ActionManager &other ); /* assignment operator */

        void insert_state( const time_type_&  stamp, const status_t_ status );

        // Compute blink activity for given duration.
        double blink_activity_in_interval( const double interval_in_sec );

        double average_blink_rate_per_minute( const double interval_in_sec );

        void alert( const string& what );

        void write_data_line( );

        void update_config_file( );

    private:

        status_t_ prev_status_;
        status_t_ curr_status_;

        double blink_rate_;

        // Time when app was launched.
        time_type_ start_time_;
        time_type_ current_time_;

        // Blink start time and end time.
        time_type_ blink_start_time_;

        // unsigned blink activity.
        unsigned total_blink_activity;

        time_type_ last_blink_time_;

        // Save start timestamp and end timestmap of blinks.
        vector< pair<time_type_, time_type_> > blinks_;

        // Note down last two write time.
        std::array<time_t, 2> modification_times_;

    public:
        
        unsigned long n_blinks_;

        double running_avg_activity_;
        double running_avg_activity_in_interval_;


        bfs::path data_file_;
        bfs::path config_file_;

        ofstream data_file_h_;
        ofstream config_file_h_;

        string display_;

}; /* -----  end of class ActionManager  ----- */


#endif   /* ----- #ifndef ActionManager_INC  ----- */
