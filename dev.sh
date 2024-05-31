#!/bin/bash

terminate_program()
{
    echo "Terminating program running in background"
    if [[ -n "$bg_pid" ]]; then
        kill "$bg_pid"
    fi
}

trap terminate_program EXIT # call on exit

watched_files="hot_reload.c|shader.glsl"

./build.sh

./main &
bg_pid=$! # capture pid of program

# rebuild dll if source file changed
inotifywait --recursive --include ${watched_files} --monitor --event modify ./ |
   while read file_path file_event file_name; do
        echo "$file_path $file_event $file_name"
        ./build.sh
   done
