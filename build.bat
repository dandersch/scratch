@echo off

REM dependencies:
REM   SDL2.dll >= 2.0.18.
REM   SDL2 header files (SDL2.h, ...)
REM   SDL2.lib
REM   SDL2main.lib

cl.exe /DWIN32 /EHsc /std:c++17 isometric.cpp /I ./dep/include /link dep/SDL2main.lib dep/SDL2.lib shell32.lib /SUBSYSTEM:CONSOLE
