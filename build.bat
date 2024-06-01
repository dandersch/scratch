#!/bin/bash

# build exe
cl.exe /I ./ /I ./dep main.c /link ./dep/SDL2main.lib ./dep/SDL2.lib shell32.lib opengl32.lib ./dep/glew32.lib /SUBSYSTEM:CONSOLE /OUT:main.exe

# build dll
clang-cl.exe /I ./ /I ./dep hot_reload.c /LD /link opengl32.lib ./dep/glew32.lib /NOIMPLIB /NOEXP /OUT:code.dll

# TODO find out how to not generate this file
rm main.obj
