echo >/dev/null # >nul & GOTO WINDOWS & rem ^
echo 'Processing for Linux'

#clang -c -fPIC -x c-header pch.h -o pch.pch # build pch

# build dll
clang --shared -include-pch "pch.pch" -fPIC hot_reload.c -o ./code.dll -lGL -lGLEW

# build exe
clang -I ./ -g $(sdl2-config --cflags) -include-pch "pch.pch" -fPIC -I/usr/include/GL -L$(pwd) -lSDL2 main.c -o main



exit 0
:WINDOWS
@ECHO off
echo 'Processing for Windows'

REM build exe
cl.exe /Zi /nologo /I ./ /I ./dep main.c /link ./dep/SDL2main.lib ./dep/SDL2.lib shell32.lib opengl32.lib ./dep/glew32.lib /DEBUG:FULL /PDB:main.pdb /SUBSYSTEM:CONSOLE /OUT:main.exe

REM build dll
clang-cl.exe /Zi /nologo /I ./ /I ./dep hot_reload.c /LD /link opengl32.lib ./dep/glew32.lib /NOIMPLIB /NOEXP /DEBUG:FULL /PDB:code.pdb /OUT:code.dll
