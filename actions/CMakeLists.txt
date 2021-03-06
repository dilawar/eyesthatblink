cmake_minimum_required(VERSION 2.6)

# xbacklight 
find_package( PkgConfig REQUIRED )
set(LIBRARIES_TO_LINK "" )

pkg_check_modules( XCB xcb REQUIRED )
include_directories( ${XCB_INCLUDE_DIRS} )
LIST(APPEND LIBRARIES_TO_LINK ${XCB_LIBRARIES})

pkg_check_modules( XCB_RANDR xcb-randr REQUIRED )
include_directories( ${XCB_RANDR_INCLUDE_DIRS} )
LIST(APPEND LIBRARIES_TO_LINK ${XCB_RANDR_LIBRARIES})

pkg_check_modules( XCB_UTIL xcb-util REQUIRED )
include_directories( ${XCB_UTIL_INCLUDE_DIRS} )
LIST(APPEND LIBRARIES_TO_LINK ${XCB_UTIL_LIBRARIES})

add_library( xbacklight ./xbacklight.cc)
target_link_libraries( xbacklight ${LIBRARIES_TO_LINK} )

# test program
add_executable( test_xbacklight test_xbacklight.cpp )
target_link_libraries( test_xbacklight xbacklight )
add_dependencies( test_xbacklight xbacklight )
