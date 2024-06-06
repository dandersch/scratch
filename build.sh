#!/usr/bin/env sh


# build dll
clang --shared -fPIC dll.c -o ./code.dll

# build exe
clang -I ./ -g -fPIC -ldl main.c -o main # && ./main
