#!/usr/bin/env bash
mkdir -p _debian 
(
    rm -f CMakeCache.txt
    cd _debian
    cmake -DCMAKE_INSTALL_PREFIX=/usr ..
    make -j3
    cpack 
)
echo "All done"
