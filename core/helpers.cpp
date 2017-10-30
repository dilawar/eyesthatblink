#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>
#include <queue>
#include <stdio.h>
#include <thread>
#include <chrono>
#include <boost/algorithm/string.hpp>

#include <iomanip>
#include <ctime>

#include "constants.h"
#include "globals.h"
#include "helpers.h"

#if USE_BOOST_PROCESS
#include <boost/process.hpp>
#else
#include "exec-stream.h"
#endif

bool rectInImage(cv::Rect rect, cv::Mat image)
{
    return rect.x > 0 && rect.y > 0 && rect.x+rect.width < image.cols &&
           rect.y+rect.height < image.rows;
}

bool inMat(cv::Point p,int rows,int cols)
{
    return p.x >= 0 && p.x < cols && p.y >= 0 && p.y < rows;
}

cv::Mat matrixMagnitude(const cv::Mat &matX, const cv::Mat &matY)
{
    cv::Mat mags(matX.rows,matX.cols,CV_64F);
    for (int y = 0; y < matX.rows; ++y)
    {
        const double *Xr = matX.ptr<double>(y), *Yr = matY.ptr<double>(y);
        double *Mr = mags.ptr<double>(y);
        for (int x = 0; x < matX.cols; ++x)
        {
            double gX = Xr[x], gY = Yr[x];
            double magnitude = sqrt((gX * gX) + (gY * gY));
            Mr[x] = magnitude;
        }
    }
    return mags;
}

double computeDynamicThreshold(const cv::Mat &mat, double stdDevFactor)
{
    cv::Scalar stdMagnGrad, meanMagnGrad;
    cv::meanStdDev(mat, meanMagnGrad, stdMagnGrad);
    double stdDev = stdMagnGrad[0] / sqrt(mat.rows*mat.cols);
    return stdDevFactor * stdDev + meanMagnGrad[0];
}

/**
 * @brief Sleep for given milliseconds.
 *
 * @param milliseconds
 */
void sleep( size_t x )
{
    std::this_thread::sleep_for(std::chrono::milliseconds(x));
}

/**
 * @brief Compute difference of two time stamps.
 *
 * @param t1 First time.
 * @param t2 Second time.
 *
 * @return  Difference in milli-seconds.
 */
int diff_in_ms( time_type_ t1, time_type_ t2 )
{
    return abs(
               std::chrono::duration_cast< std::chrono::milliseconds >( t2 - t1 ).count( )
           );
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis  Expand user in given path.
 *
 * @Param path
 *
 * @Returns   
 */
/* ----------------------------------------------------------------------------*/
std::string expand_user(std::string path)
{
    string home = getenv("HOME");

    if (! path.empty() && path[0] == '~')
    {
        assert(path.size() == 1 or path[1] == '/');  // or other error handling

        if ( home.size( ) > 0 || (home == getenv("USERPROFILE")) )
            path.replace(0, 1, home);
        else
        {
            char const *hdrive = getenv("HOMEDRIVE"),
                        *hpath = getenv("HOMEPATH");
            path.replace(0, 1, std::string(hdrive) + hpath);
        }
    }

    // If there is $HOME.
    if( path.find( "$HOME" ) != string::npos )
        boost::replace_all( path, "$HOME", home );

    return path;
}

/* --------------------------------------------------------------------------*/
/**
 * @Synopsis  Swapn a child process.
 *
 * @Param command
 *
 * @Returns   
 */
/* ----------------------------------------------------------------------------*/
int spawn( const string& command )
{
    vector<string> cmdVec;
    boost::algorithm::split( cmdVec, command, boost::is_any_of(" ") );

#if USE_BOOST_PROCESS
    namespace bp = boost::process;

#ifdef OS_IS_UNIX
#ifdef OS_IS_APPLE
    bp::spawn( command );
#else
    bp::spawn( command );
#endif
#endif

#else // NOT USING BOOST SUBPROCESS

    exec_stream_t es;
    vector<string> argVec( cmdVec.begin()+1, cmdVec.end() );
    string args = boost::algorithm::join( argVec, " " );

#ifdef OS_IS_UNIX
    es.start( cmdVec[0], args );
#endif

#endif
    LOG_INFO << "Executing " << command;
    return 0;
}


#if 0
// WARN: Require gcc-5.0
void print_time( time_type_ time, const std::string end )
{
    std::time_t now = std::chrono::system_clock::to_time_t( time );
    std::cout << std::put_time( std::localtime(&now), "%F %T" ) << end;
}
#endif
