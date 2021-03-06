/***
 *       Filename:  helpers.h
 *
 *    Description:  Helper functions.
 *
 *        Version:  0.0.1
 *        Created:  2017-11-11
 *       Revision:  none
 *
 *         Author:  Dilawar Singh <dilawars@ncbs.res.in>
 *   Organization:  NCBS Bangalore
 *
 *        License:  GNU GPL2
 */

#ifndef HELPERS_H
#define HELPERS_H

#include <deque>

#include "globals.h"

typedef std::chrono::time_point<std::chrono::system_clock> time_type_;

using namespace std;

bool rectInImage(cv::Rect rect, cv::Mat image);
bool inMat(cv::Point p,int rows,int cols);

cv::Mat matrixMagnitude(const cv::Mat &matX, const cv::Mat &matY);
double computeDynamicThreshold(const cv::Mat &mat, double stdDevFactor);

void _sleep( size_t milliseconds );

int diff_in_ms( time_type_ t1, time_type_ t2 );

#if 0
void print_time( time_type_ time, const string end = "\n" );
#endif

std::string expand_user(std::string path);

string spawn( const string& command );

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis  Find display using xrandr.
 *
 * @Returns   
 */
/* ----------------------------------------------------------------------------*/
vector<string> find_display( );

/**
 * @brief Compute the mean of given container.
 *
 * @tparam T
 * @param vec
 *
 * @return
 */
template<typename T = std::deque<double> >
double mean( const T vec )
{
    return accumulate( vec.begin(), vec.end(), 0.0 ) / vec.size( );
}

template< typename T = double >
void meanStd( const deque<T> vec, T* avg, T * stddev  = 0 )
{
    T m = mean( vec );
    *avg = m;

    if( stddev > 0 )
    {
        T sum = 0.0;
        for( T v : vec )
            sum += pow( v - m, 2 );
        *stddev = sqrt( sum / vec.size( ) );
    }
}

#endif
