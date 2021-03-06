cmake_minimum_required(VERSION 2.8)
cmake_policy(SET CMP0048 NEW)
project( EyesThatBlink VERSION 1.0.0)
add_definitions( --std=c++11 )

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
# Enable/disable debug.
set(WITH_DEBUG ON)
if(WITH_DEBUG)
    set( CMAKE_BUILD_TYPE Debug )
else()
    set( CMAKE_BUILD_TYPE Release )
endif()

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/external/plog/include )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/external/pstreams-1.0.1/)

set(USE_BOOST_PROCESS OFF)
if(USE_BOOST_PROCESS)
    add_definitions( -DUSE_BOOST_PROCESS )
    include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/external/cpp-subprocess/)
endif( )

find_package(OpenCV REQUIRED)
if(OpenCV_FOUND)
    set(WITH_LOCAL_OPENCV OFF)
else( )
set( WITH_LOCAL_OPENCV ON )
endif( )

include_directories( ${OpenCV_INCLUDE_DIRS} )
message( STATUS "OpenCV libraries ${OpenCV_LIBRARIES}" )

# Check for pkg-config modules. We need to link with their libs.
find_package( PkgConfig REQUIRED )
set( OTHER_LIBS "" )
if( WITH_LOCAL_OPENCV )
    if( UNIX AND NOT APPLE )
        pkg_check_modules( GTK3 gtk+-3.0 )
        list(APPEND OTHER_LIBS ${GTK3_LIBRARIES})
        pkg_check_modules( GLIB2 glib-2.0 )
        list(APPEND OTHER_LIBS ${GLIB2_LIBRARIES})
        pkg_check_modules( LIBPNG libpng )
        list(APPEND OTHER_LIBS ${LIBPNG_LIBRARIES})
        pkg_check_modules( V4L2 libv4l2 )
        list(APPEND OTHER_LIBS ${V4L2_LIBRARIES})
        pkg_check_modules( V4L1 libv4l1 )
        list(APPEND OTHER_LIBS ${V4L1_LIBRARIES})
        pkg_check_modules( LIBDC1394 libdc1394-2 )
        list(APPEND OTHER_LIBS ${LIBDC1394_LIBRARIES})
        pkg_check_modules( LIBWEBP libwebp )
        list(APPEND OTHER_LIBS ${LIBWEBP_LIBRARIES})
    else(APPLE)
        pkg_check_modules( LIBPNG libpng )
    endif( )

    # Other libs.
    find_package( ZLIB )
    # libdl 
    list(APPEND OTHER_LIBS dl )
endif( )

if(UNIX)
    pkg_check_modules( LIBNOTIFY libnotify REQUIRED )
    list(APPEND OTHER_LIBS ${LIBNOTIFY_LIBRARIES})

    if(NOT WITH_LOCAL_OPENCV)
        pkg_check_modules( GTKMM gtkmm-2.4 REQUIRED )
        add_definitions( -DWITH_GTK2 )
    else( )
        pkg_check_modules( GTKMM gtkmm-3.0 REQUIRED )
        add_definitions( -DWITH_GTK3 )
    endif( )
    include_directories( ${GTKMM_INCLUDE_DIRS} )
endif( )

# pthreads
find_package( Threads )
list(APPEND OTHER_LIBS ${CMAKE_THREAD_LIBS_INIT} )

find_package( Boost REQUIRED COMPONENTS iostreams system filesystem date_time
    regex)
include_directories( ${Boost_INCLUDE_DIRS} )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/external/boost-process/include )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/external/boost-asio/include )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/external/boost-winapi/include )

if(WIN32)
    add_definitions( -DOS_IS_WINDOWS )
elseif(UNIX)
    add_definitions( -DOS_IS_UNIX )
    if(APPLE)
        add_definitions( -DOS_IS_APPLE )
    endif(APPLE)
    set( APP_DATA_DIR "\$HOME/.config/eyesthatblink/" )
    set( CONFIG_FILE_PATH "${APP_DATA_DIR}/${CMAKE_PROJECT_NAME}.conf" )
    set( DATA_FILE_PATH "${APP_DATA_DIR}/${CMAKE_PROJECT_NAME}.data" )
endif( )

set( LOG_FILE_PATH "\$HOME/.cache/eyesthatblink.log" ) # log file.
set( AVERAGE_BLINK_DURATION 200 )   # in ms.

set(ICON_FILE_48 ${CMAKE_CURRENT_SOURCE_DIR}/data/EyesThatBlink.png)
set(DESKTOP_FILE ${CMAKE_CURRENT_SOURCE_DIR}/data/EyesThatBlink.desktop)
set(CASCADE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/cascades)

get_filename_component( ICON_FILENAME ${ICON_FILE_48} NAME )

set( ETB_BINARY_PATH ${CMAKE_INSTALL_PREFIX}/bin )
set( UI_XML_PATH ${CMAKE_INSTALL_PREFIX}/share/eyesthatblink/ui.xml )
message( STATUS "ETB path ${ETB_BINARY_PATH}/" )

set(APP_DATA_DIR  ${CMAKE_INSTALL_PREFIX}/share/eyesthatblink )
set(ICONFILE_INSTALL_DIR ${APP_DATA_DIR} )
set(CASCADE_INSTALL_DIR ${APP_DATA_DIR}/cascades )
set(ICONFILE_INSTALL_PATH ${ICONFILE_INSTALL_DIR}/${ICON_FILENAME})

# Write configuration.
configure_file( 
    ${CMAKE_CURRENT_SOURCE_DIR}/config.h.in 
    ${CMAKE_CURRENT_SOURCE_DIR}/config.h
    )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/plog/include )
add_subdirectory( ${CMAKE_CURRENT_SOURCE_DIR}/core )
add_subdirectory( ${CMAKE_CURRENT_SOURCE_DIR}/ui )
add_subdirectory( ${CMAKE_CURRENT_SOURCE_DIR}/actions )


# Add target and dependencies.
add_executable( eyesthatblink 
    WIN32 MACOSX_BUNDLE main.cpp  ${ICON_FILE_48} )

if(WITH_LOCAL_OPENCV)
    # Build opencv before eyesthatblink only if required.
    add_dependencies( eyesthatblink opencv_local )
endif()

target_link_libraries( eyesthatblink core ui xbacklight)

if(GTKMM_FOUND)
    target_link_libraries( eyesthatblink ${GTKMM_LIBRARIES} )
endif()
target_link_libraries( eyesthatblink ${OpenCV_LIBRARIES} )
target_link_libraries( eyesthatblink ${Boost_LIBRARIES} )
target_link_libraries( eyesthatblink ${ZLIB_LIBRARIES} )


if(UNIX AND NOT APPLE )
    target_link_libraries( eyesthatblink core ${OTHER_LIBS} )
elseif(APPLE)
    target_link_libraries( eyesthatblink core "-framework CoreMedia" )
    target_link_libraries( eyesthatblink "-framework AVFoundation" )
    target_link_libraries( eyesthatblink "-framework Foundation" )
    target_link_libraries( eyesthatblink "-framework CoreGraphics" )
    target_link_libraries( eyesthatblink "-framework CoreVideo" )
    target_link_libraries( eyesthatblink ${LIBPNG_LIBRARIES} )
endif( )

## Write configuration file.
## Write the binary
#configure_file( 
#    ${CMAKE_CURRENT_SOURCE_DIR}/eyesthatblink.in 
#    ${CMAKE_BINARY_DIR}/eyesthatblink
#    )


add_custom_target( run 
    DEPENDS eyesthatblink 
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/eyesthatblink
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    VERBATIM
    )

# Install 
file(GLOB CASCADE_XML_FILES "${CASCADE_DIR}/*.xml")
install( TARGETS eyesthatblink DESTINATION bin )
install( PROGRAMS ${CMAKE_BINARY_DIR}/eyesthatblink DESTINATION bin )

install( FILES ${ICON_FILE_48} DESTINATION share/icons/hicolor/48x48/apps )
install( FILES ${ICON_FILE_48} DESTINATION ${ICONFILE_INSTALL_DIR} )
install( FILES ${CMAKE_CURRENT_SOURCE_DIR}/ui/ui.xml DESTINATION ${APP_DATA_DIR} )

install( FILES ${DESKTOP_FILE} DESTINATION share/applications )
install( FILES ${CASCADE_XML_FILES} DESTINATION ${CASCADE_INSTALL_DIR} )

# uninstall target.
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
    IMMEDIATE @ONLY
    )

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake
    )

# CPACK.
set( CPACK_GENERATOR "DEB;RPM" )
set( CPACK_DEBIAN_PACKAGE_MAINTAINER "Dilawar Singh <dilawars@ncbs.res.in>" )
set( CPACK_DEBIAN_PACKAGE_DEPENDS "python-gi" )
include( CPack )
