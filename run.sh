#!/bin/bash

# make and enter build directory
mkdir -p build
cd build

# build project
cmake ..

# compile with multiple threads
make -j8 

# if build correctly
if [ $? -eq 0 ]; then
    sleep 3
    # start server in frontground
    ./server 
    server_pid=$!
    
    # start all clients in background
    ./price &
    ./trade &
    ./market &
    ./inquiry &
    
    # wait all processes to exit
    wait

    # you can press any key to terminate all processes
    read -p "Press any key to terminate all processes." -n 1 -r
    echo

    # kill all processes
    kill $server_pid
    pkill -P $$

    cd ..

else
    echo "Build failed"
fi
