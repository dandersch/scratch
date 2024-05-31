#!/bin/sh

# build dll
clang --shared ${CmnCompilerFlags} -fPIC hot_reload.c -o ./code.dll -lGL -lGLEW

# build exe
clang -I ./ -g -fPIC -I/usr/include/GL -L$(pwd) -lSDL2 -lGL -lGLEW -ldl main.c -o main
