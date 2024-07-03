# build pch
clang -c -fPIC -x c-header pch.h -o pch.pch

# build dll
clang --shared -include-pch "pch.pch" -fPIC gl_scope.c -o ./code.dll -lGL -lGLEW

# build exe
clang -I ./ -g $(sdl2-config --cflags) -include-pch "pch.pch" -fPIC -I/usr/include/GL -L$(pwd) -lSDL2 main.c -o main
