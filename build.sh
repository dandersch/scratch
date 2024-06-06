#!/usr/bin/env sh

./pp.c $(echo *.c *.h | sed 's/pp.c//' | sed 's/meta_gen.h//') > meta_gen.h

# build dll
clang --shared -fPIC dll.c -o ./code.dll

# build exe
clang -I ./ -g -fPIC -ldl main.c -o main # && ./main
