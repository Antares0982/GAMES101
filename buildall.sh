#!/bin/bash
for dirname in `find . -maxdepth 1 -type d`
do
    if [[ "./.git" == $dirname || "." == $dirname ]];then
        continue
    fi

    cd $dirname
    if [ ! -f "CMakeLists.txt" ]; then
        cd ..
        continue
    fi

    if [ ! -d "build" ]; then
        mkdir build
    fi
    
    cd build && cmake .. && make -j8
    cd ../..
done
