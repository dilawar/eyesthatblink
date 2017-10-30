include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/.. )

set(CORE_SRCS 
    globals.cpp 
    helpers.cpp 
    blink_detector.cpp 
    #stablizer.cpp 
    ActionManager.cpp 
    ConfigManager.cpp
    main_loop.cpp 
    )

add_library( core STATIC ${CORE_SRCS} )

if(WITH_LOCAL_OPENCV)
    add_dependencies( core opencv_local )
endif( )
