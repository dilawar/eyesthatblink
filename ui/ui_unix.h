/***
 *       Filename:  ui_unix.h
 *
 *    Description:  Unix related UI.
 *
 *        Version:  0.0.1
 *        Created:  2017-10-27
 *       Revision:  none
 *
 *         Author:  Dilawar Singh <dilawars@ncbs.res.in>
 *   Organization:  NCBS Bangalore
 *
 *        License:  GNU GPL2
 */

#ifndef UI_UNIX_H
#define UI_UNIX_H

#include "ui.h"
#include "../core/main_loop.h"
#include <opencv2/opencv.hpp>

void toggleShowUserFace( );
void loadSmallEyeCascade( bool value );
void loadEyeGlassesCascade( bool value );

int unix_ui( int argc, char* argv[] );

#endif /* end of include guard: UI_UNIX_H */
