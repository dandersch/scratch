# build exe
clang -Wno-unused-value -I cf -I /usr/include/SDL2 -D_REENTRANT main.c -L$(pwd) -lcute -Wl,-rpath=$(pwd) -o main
