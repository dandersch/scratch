#ifndef COMMON_H_
#define COMMON_H_

#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>

#define SCREEN_WIDTH  300
#define SCREEN_HEIGHT 300

// TODO unused
#define KEYMAP(X) \
    X(KEY_SPACE, SDLK_SPACE) \
    X(KEY_UP,    SDLK_UP) \
    X(KEY_DOWN,  SDLK_DOWN) \
    X(KEY_LEFT,  SDLK_LEFT) \
    X(KEY_RIGHT, SDLK_RIGHT)

enum
{
    #define ENUMIFY(key) key,
    KEY_COUNT
};


/* define export declaration for .dll & .so files */
#if defined(_MSC_VER)
    #define EXPORT __declspec(dllexport)
#else
    #define EXPORT __attribute__((visibility("default")))
#endif

#endif // COMMON_H_
