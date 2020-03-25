cmake_minimum_required(VERSION 3.1)
project(EyesThatBlink VERSION 1.0.1)

cmake_policy(SET CMP0048 NEW)

add_definitions(--std=c++14)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/external/plog/include )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/external/pstreams-1.0.1/)

find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})
message(STATUS "OpenCV libraries ${OpenCV_LIBRARIES}")

# Check for pkg-config modules. We need to link with their libs.
find_package(PkgConfig REQUIRED)
if(UNIX)
    pkg_check_modules(LIBNOTIFY libnotify REQUIRED)
    list(APPEND OTHER_LIBS ${LIBNOTIFY_LIBRARIES})
    pkg_check_modules(GTKMM gtkmm-3.0 REQUIRED)
    include_directories(${GTKMM_INCLUDE_DIRS})
endif()

find_package(Threads)
find_package(Boost
    COMPONENTS program_options regex date_time iostreams filesystem system
    REQUIRED)

list(APPEND OTHER_LIBS ${CMAKE_THREAD_LIBS_INIT} )

include_directories(${Boost_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/boost-process/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/boost-asio/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/boost-winapi/include)

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
add_executable(eyesthatblink main.cpp  ${ICON_FILE_48})
target_link_libraries(eyesthatblink core ui xbacklight)

if(GTKMM_FOUND)
    target_link_libraries( eyesthatblink ${GTKMM_LIBRARIES} )
endif()

target_link_libraries(eyesthatblink ${OpenCV_LIBRARIES})
target_link_libraries(eyesthatblink ${ZLIB_LIBRARIES})
target_link_libraries(eyesthatblink 
    Boost::date_time
    Boost::filesystem
    Boost::iostreams
    Boost::program_options
    Threads::Threads)

if(UNIX AND NOT APPLE )
    target_link_libraries(eyesthatblink core ${OTHER_LIBS} )
elseif(APPLE)
    target_link_libraries(eyesthatblink core "-framework CoreMedia")
    target_link_libraries(eyesthatblink "-framework AVFoundation")
    target_link_libraries(eyesthatblink "-framework Foundation")
    target_link_libraries(eyesthatblink "-framework CoreGraphics")
    target_link_libraries(eyesthatblink "-framework CoreVideo")
    target_link_libraries(eyesthatblink ${LIBPNG_LIBRARIES})
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
