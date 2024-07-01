#pragma once

#include <SDL2/SDL.h>

/* HOT RELOAD */
#include <sys/stat.h>         // for checking if dll changed on disk
#include <SDL2/SDL_loadso.h>  // cross platform dll loading

// used by dll
#include <GL/glew.h>
