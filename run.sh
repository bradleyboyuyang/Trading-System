#!/bin/bash

# make and enter build directory
mkdir -p build
cd build || exit

# build project
cmake ..

# if successfully compiled with multiple threads
if make -j8; then
    # start server in background and print its output to the console
    ./server &
    server_pid=$!
    sleep 3
    
    # start all clients in background
    ./price &
    ./trade &
    ./market &
    ./inquiry &
    
    # trap SIGINT and terminate all processes
    trap 'kill $server_pid; pkill -P $$; exit' SIGINT

    # you can press any key to terminate all processes
    read -p "Press any key to terminate all processes." -n 1 -r
    echo

    # wait all processes to exit
    wait

    # kill all processes
    kill $server_pid
    pkill -P $$

    cd ..

else
    echo "Build failed"
fi
