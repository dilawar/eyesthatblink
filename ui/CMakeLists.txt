cmake_minimum_required(VERSION 2.8)
if( UNIX )
    add_library( ui ui_unix.cpp etbapplication.cpp )
    if(WITH_LOCAL_OPENCV)
        add_dependencies( ui opencv_local )
    endif()
endif( )
