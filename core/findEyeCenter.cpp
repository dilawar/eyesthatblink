#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/calib3d/calib3d.hpp"

//#include <mgl2/mgl.h>

#include <iostream>
#include <queue>
#include <stdio.h>

#include "constants.h"
#include "helpers.h"

using namespace std;
using namespace cv;


// Pre-declarations
cv::Mat floodKillEdges(cv::Mat &mat);

cv::Point unscalePoint(cv::Point p, cv::Rect origSize)
{
    float ratio = (((float)kFastEyeWidth)/origSize.width);
    int x = round(p.x / ratio);
    int y = round(p.y / ratio);
    return cv::Point(x,y);
}

void scaleToFastSize(const cv::Mat &src,cv::Mat &dst)
{
    cv::resize(src, dst, cv::Size(kFastEyeWidth,(((float)kFastEyeWidth)/src.cols) * src.rows));
}

cv::Mat computeMatXGradient(const cv::Mat &mat)
{
    cv::Mat out(mat.rows,mat.cols,CV_64F);

    for (int y = 0; y < mat.rows; ++y)
    {
        const uchar *Mr = mat.ptr<uchar>(y);
        double *Or = out.ptr<double>(y);

        Or[0] = Mr[1] - Mr[0];
        for (int x = 1; x < mat.cols - 1; ++x)
        {
            Or[x] = (Mr[x+1] - Mr[x-1])/2.0;
        }
        Or[mat.cols-1] = Mr[mat.cols-1] - Mr[mat.cols-2];
    }

    return out;
}

#pragma mark Main Algorithm

void testPossibleCentersFormula(int x, int y, const cv::Mat &weight,double gx, double gy, cv::Mat &out)
{
    // for all possible centers
    for (int cy = 0; cy < out.rows; ++cy)
    {
        double *Or = out.ptr<double>(cy);
        const unsigned char *Wr = weight.ptr<unsigned char>(cy);
        for (int cx = 0; cx < out.cols; ++cx)
        {
            if (x == cx && y == cy)
            {
                continue;
            }
            // create a vector from the possible center to the gradient origin
            double dx = x - cx;
            double dy = y - cy;
            // normalize d
            double magnitude = sqrt((dx * dx) + (dy * dy));
            dx = dx / magnitude;
            dy = dy / magnitude;
            double dotProduct = dx*gx + dy*gy;
            dotProduct = std::max(0.0,dotProduct);
            // square and multiply by the weight
            if (kEnableWeight)
            {
                Or[cx] += dotProduct * dotProduct * (Wr[cx]/kWeightDivisor);
            }
            else
            {
                Or[cx] += dotProduct * dotProduct;
            }
        }
    }
}

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

cv::Point find_pupil( cv::Mat leftE, cv::Mat rightE )
{
    Mat both;

    Mat img_object = leftE;
    Mat img_scene = rightE;

    //-- Step 1: Detect the keypoints using SURF Detector
    int minHessian = 400;
    SurfFeatureDetector detector( minHessian );

    std::vector<KeyPoint> keypoints_object, keypoints_scene;

    detector.detect( img_object, keypoints_object );
    detector.detect( img_scene, keypoints_scene );

    //-- Step 2: Calculate descriptors (feature vectors)
    SurfDescriptorExtractor extractor;

    Mat descriptors_object, descriptors_scene;

    extractor.compute( img_object, keypoints_object, descriptors_object );
    extractor.compute( img_scene, keypoints_scene, descriptors_scene );

    //-- Step 3: Matching descriptor vectors using FLANN matcher
    FlannBasedMatcher matcher;
    std::vector< DMatch > matches;
    matcher.match( descriptors_object, descriptors_scene, matches );

    double max_dist = 0; double min_dist = 100;

    //-- Quick calculation of max and min distances between keypoints
    for( int i = 0; i < descriptors_object.rows; i++ )
    { double dist = matches[i].distance;
        if( dist < min_dist ) min_dist = dist;
        if( dist > max_dist ) max_dist = dist;
    }

    printf("-- Max dist : %f \n", max_dist );
    printf("-- Min dist : %f \n", min_dist );

    //-- Draw only "good" matches (i.e. whose distance is less than 3*min_dist )
    std::vector< DMatch > good_matches;

    for( int i = 0; i < descriptors_object.rows; i++ )
    { if( matches[i].distance < 3*min_dist )
        { good_matches.push_back( matches[i]); }
    }

    Mat img_matches;
    drawMatches( img_object, keypoints_object, img_scene, keypoints_scene,
            good_matches, img_matches, Scalar::all(-1), Scalar::all(-1),
            vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS );

    //-- Localize the object
    std::vector<Point2f> obj;
    std::vector<Point2f> scene;

    for( int i = 0; i < good_matches.size(); i++ )
    {
        //-- Get the keypoints from the good matches
        obj.push_back( keypoints_object[ good_matches[i].queryIdx ].pt );
        scene.push_back( keypoints_scene[ good_matches[i].trainIdx ].pt );
    }

    Mat H = findHomography( obj, scene, CV_RANSAC );

    //-- Get the corners from the image_1 ( the object to be "detected" )
    std::vector<Point2f> obj_corners(4);
    obj_corners[0] = cvPoint(0,0); obj_corners[1] = cvPoint( img_object.cols, 0 );
    obj_corners[2] = cvPoint( img_object.cols, img_object.rows ); obj_corners[3] = cvPoint( 0, img_object.rows );
    std::vector<Point2f> scene_corners(4);

    perspectiveTransform( obj_corners, scene_corners, H);

    //-- Draw lines between the corners (the mapped object in the scene - image_2 )
    line( img_matches, scene_corners[0] + Point2f( img_object.cols, 0), scene_corners[1] + Point2f( img_object.cols, 0), Scalar(0, 255, 0), 4 );
    line( img_matches, scene_corners[1] + Point2f( img_object.cols, 0), scene_corners[2] + Point2f( img_object.cols, 0), Scalar( 0, 255, 0), 4 );
    line( img_matches, scene_corners[2] + Point2f( img_object.cols, 0), scene_corners[3] + Point2f( img_object.cols, 0), Scalar( 0, 255, 0), 4 );
    line( img_matches, scene_corners[3] + Point2f( img_object.cols, 0), scene_corners[0] + Point2f( img_object.cols, 0), Scalar( 0, 255, 0), 4 );

    //-- Show detected matches
    imshow( "Good Matches & Object detection", img_matches );

    waitKey(0);
    return 0;
    cv::imshow( "Overlap image", both );
}


/**
 * @brief Find the center of eye.
 *
 * @param eye
 *
 * @return 
 */
cv::Point find_eye_center( cv::Mat eye )
{
    Mat img;
    eye.copyTo( img );
    
    //GaussianBlur( eye, eye, Size(5, 5), 2, 2 );

    // Divide by 4 is a good value. Divide by 5 migh miss some pixals.
    int pupilDiameter = eye.rows / 3.0;

    Point midPoint, pupilLoc;
    midPoint.x = eye.cols/2.0;
    midPoint.y = eye.rows/2.0;

    pupilLoc.x = midPoint.x;
    pupilLoc.y = midPoint.y;

    // Now we move in 8 directions: for following combinations of xshift and
    // yshift. We find if there is any poisition where we get the minimum.
    double minW = 100;
    cv::Point minLoc( pupilLoc );
    double r = 2.0;

#if 0
    bool foundAnotherMinimum = true;
    while( foundAnotherMinimum )
    {
        foundAnotherMinimum = false;
        for( double theta = 0.0; theta < 360; theta += 45 )
        {
            Point newLoc;
            int xShift = ceil( r * cos( theta ));
            int yShift = ceil( r * sin( theta ));
            newLoc.x = pupilLoc.x + xShift;
            newLoc.y = pupilLoc.y + yShift;
            double w = pupil_weight( eye, newLoc, pupilDiameter /2 );
            if( w < minW )
            {
                minW = w;
                pupilLoc = newLoc;
                foundAnotherMinimum = true;
            }

            // Don't go far from center.
            if( cv::norm( pupilLoc - midPoint ) >= 10 )
                break;
        }
    }
#endif

    
#if 0
    /*-----------------------------------------------------------------------------
     *  The problem with the following methos is the presence of eyebrows which
     *  are of the same color as of pupil. Any simple thresholding would not
     *  work. On aninal without eyebrows this method gives good result. Need to
     *  find method which is very robust in finding/locating pupil when eye
     *  location is known.
     *-----------------------------------------------------------------------------*/

    cout << "New pupil location: " << pupilLoc <<  " with cost " << minW << endl;

    double min, max;
    minMaxLoc( eye, &min, &max );
    cv::circle( eye, pupilLoc, pupilDiameter / 2.0, 255, 1 );

    eye = max - eye;

    auto avg = cv::mean( eye );
    double mean = avg[ 0 ];
    double std = cv::norm( eye -mean ) / pow(eye.rows * eye.cols, 0.5);
    cv::threshold( eye, eye, max - 2.0 * std, max, cv::THRESH_BINARY_INV );

    // Now threshold it.
    Mat eyeDebug;
    hconcat( eye, img, eyeDebug );
    cv::imshow( "Eyes", eyeDebug );
#endif

    return pupilLoc;
}

/**
 * @brief Search for the center of eyes.
 *
 * @param face
 * @param eye
 * @param debugWindow
 *
 * @return 
 */
cv::Point findEyeCenter(cv::Mat face, cv::Rect eye, std::string debugWindow="")
{
    cv::Mat eyeROIUnscaled = face(eye);
    cv::Mat eyeROI;
    scaleToFastSize(eyeROIUnscaled, eyeROI);

    // draw eye region
    rectangle(face,eye,1234);
    //-- Find the gradient
    cv::Mat gradientX = computeMatXGradient(eyeROI);
    cv::Mat gradientY = computeMatXGradient(eyeROI.t()).t();

    //-- Normalize and threshold the gradient
    // compute all the magnitudes
    cv::Mat mags = matrixMagnitude(gradientX, gradientY);
    //compute the threshold
    double gradientThresh = computeDynamicThreshold(mags, kGradientThreshold);

    //double gradientThresh = kGradientThreshold;
    //double gradientThresh = 0;
    //normalize
    for (int y = 0; y < eyeROI.rows; ++y)
    {
        double *Xr = gradientX.ptr<double>(y), *Yr = gradientY.ptr<double>(y);
        const double *Mr = mags.ptr<double>(y);
        for (int x = 0; x < eyeROI.cols; ++x)
        {
            double gX = Xr[x], gY = Yr[x];
            double magnitude = Mr[x];
            if (magnitude > gradientThresh)
            {
                Xr[x] = gX/magnitude;
                Yr[x] = gY/magnitude;
            }
            else
            {
                Xr[x] = 0.0;
                Yr[x] = 0.0;
            }
        }
    }

    //-- Create a blurred and inverted image for weighting
    cv::Mat weight;
    GaussianBlur( eyeROI, weight, cv::Size( kWeightBlurSize, kWeightBlurSize ), 0, 0 );
    for (int y = 0; y < weight.rows; ++y)
    {
        unsigned char *row = weight.ptr<unsigned char>(y);
        for (int x = 0; x < weight.cols; ++x)
        {
            row[x] = (255 - row[x]);
        }
    }

    //-- Run the algorithm!
    cv::Mat outSum = cv::Mat::zeros(eyeROI.rows,eyeROI.cols,CV_64F);
    // for each possible gradient location
    // Note: these loops are reversed from the way the paper does them
    // it evaluates every possible center for each gradient location instead of
    // every possible gradient location for every center.
    for (int y = 0; y < weight.rows; ++y)
    {
        const double *Xr = gradientX.ptr<double>(y), *Yr = gradientY.ptr<double>(y);
        for (int x = 0; x < weight.cols; ++x)
        {
            double gX = Xr[x], gY = Yr[x];
            if (gX == 0.0 && gY == 0.0)
                continue;

            testPossibleCentersFormula(x, y, weight, gX, gY, outSum);
        }
    }
    // scale all the values down, basically averaging them
    double numGradients = (weight.rows*weight.cols);
    cv::Mat out;
    outSum.convertTo(out, CV_32F,1.0/numGradients);
    //-- Find the maximum point
    cv::Point maxP;
    double maxVal;
    cv::minMaxLoc(out, NULL,&maxVal,NULL,&maxP);

    //-- Flood fill the edges
    if(kEnablePostProcess)
    {
        cv::Mat floodClone;
        //double floodThresh = computeDynamicThreshold(out, 1.5);
        double floodThresh = maxVal * kPostProcessThreshold;
        cv::threshold(out, floodClone, floodThresh, 0.0f, cv::THRESH_TOZERO);
        cv::Mat mask = floodKillEdges(floodClone);
        cv::minMaxLoc(out, NULL,&maxVal,NULL,&maxP,mask);
    }
 
    cv::Point pupil = unscalePoint(maxP,eye);

    return pupil;
}

#pragma mark Postprocessing

bool floodShouldPushPoint(const cv::Point &np, const cv::Mat &mat)
{
    return inMat(np, mat.rows, mat.cols);
}

// returns a mask
cv::Mat floodKillEdges(cv::Mat &mat)
{
    rectangle(mat,cv::Rect(0,0,mat.cols,mat.rows),255);

    cv::Mat mask(mat.rows, mat.cols, CV_8U, 255);
    std::queue<cv::Point> toDo;
    toDo.push(cv::Point(0,0));
    while (!toDo.empty())
    {
        cv::Point p = toDo.front();
        toDo.pop();
        if (mat.at<float>(p) == 0.0f)
        {
            continue;
        }
        // add in every direction
        cv::Point np(p.x + 1, p.y); // right
        if (floodShouldPushPoint(np, mat)) toDo.push(np);
        np.x = p.x - 1;
        np.y = p.y; // left
        if (floodShouldPushPoint(np, mat)) toDo.push(np);
        np.x = p.x;
        np.y = p.y + 1; // down
        if (floodShouldPushPoint(np, mat)) toDo.push(np);
        np.x = p.x;
        np.y = p.y - 1; // up
        if (floodShouldPushPoint(np, mat)) toDo.push(np);
        // kill it
        mat.at<float>(p) = 0.0f;
        mask.at<uchar>(p) = 0;
    }
    return mask;
}
