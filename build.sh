#!/bin/bash

set -e

bear -- \
g++ -g -I/usr/include/SDL2 -lSDL2 isometric.cpp -o iso

./iso
