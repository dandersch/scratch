#ifndef COMMON_H_
#define COMMON_H_

#include <GL/glew.h>
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

// contains all state of the program
typedef struct state_t
{
    GLuint VAO, VBO, shader_program;
    SDL_GLContext context;
    SDL_Window* window;
} state_t;

#define SCREEN_WIDTH  300
#define SCREEN_HEIGHT 300

#endif // COMMON_H_
