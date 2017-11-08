if(UNIX AND NOT APPLE)
    set(SRCS linux.cpp)
endif( )
add_library( action ${SRCS} )
