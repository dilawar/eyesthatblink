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
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/boost-winapi/include)

set(RUNTIME_CONFIG_DIR "\$HOME/.config/eyesthatblink")
set(RUNTIME_DATA_DIR "\$HOME/.cache/eyesthatblink")

if(WIN32)
    add_definitions(-DOS_IS_WINDOWS)
elseif(UNIX)
    add_definitions(-DOS_IS_UNIX)
    if(APPLE)
        add_definitions(-DOS_IS_APPLE)
    endif(APPLE)
    set(DATA_FILEPATH "\$HOME/.config/eyesthatblink/${CMAKE_PROJECT_NAME}.data")
endif()

set(LOG_FILEPATH "\$HOME/.cache/eyesthatblink.log")
set(CONFIG_FILEPATH "\$HOME/.config/eyesthatblink/config.xml")

set(AVERAGE_BLINK_DURATION 200)

set(ICON_FILENAME "EyesThatBlink.png")
set(DESKTOP_FILENAME "EyesThatBlink.desktop")
set(APP_DATADIR ${CMAKE_CURRENT_SOURCE_DIR}/data)

set(ICON_FILE_48 ${APP_DATADIR}/${ICON_FILENAME})
set(DESKTOP_FILE ${APP_DATADIR}/${DESKTOP_FILENAME})
set(CASCADE_DIR ${APP_DATADIR}/cascades)

set(ETB_BINARY_PATH ${CMAKE_INSTALL_PREFIX}/bin)
set(UI_XML_PATH ${CMAKE_INSTALL_PREFIX}/share/eyesthatblink/ui.xml )
message(STATUS "ETB path ${ETB_BINARY_PATH}/")

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
