#ifndef COMMON_H_
#define COMMON_H_

#include <GL/glew.h>
#include <stdio.h>
#include <stdlib.h>
//#include <SDL2/SDL.h>

typedef void *SDL_GLContext;
struct SDL_Window;
typedef struct SDL_Window SDL_Window ;

// contains all state of the program
typedef struct state_t
{
    GLuint VAO, VBO, shader_program;

} state_t;

#define SCREEN_WIDTH  300
#define SCREEN_HEIGHT 300

/* define export declarations for .dll & .so files */
#if defined(_MSC_VER)
    #define EXPORT __declspec(dllexport)
#else
    #define EXPORT __attribute__((visibility("default")))
#endif

#endif // COMMON_H_
