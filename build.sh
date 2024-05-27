#!/bin/bash

watched_files="hot_reload.c|shader.frag|shader.vert"

# build dll
clang --shared ${CmnCompilerFlags} -fPIC hot_reload.c -o ./code.dll -lGL -lGLEW

# build exe
clang -I ./ -g -fPIC -I/usr/include/GL -L$(pwd) -lSDL2 -lGL -lGLEW -ldl main.c -o main

# rebuild dll if source file changed
inotifywait --recursive --include ${watched_files} --monitor --event modify --event create ./ |
   while read file_path file_event file_name; do
        clang --shared ${CmnCompilerFlags} -fPIC hot_reload.c -o ./code.dll -lGL -lGLEW
   done
