#include <stdio.h>

/* USAGE CODE: define x-macros before including dll_table.h */
#define RECODE_DLL_TABLE(DLL) \
    DLL(cute,    "./cute.dll",    CUTE_FUNCTION_TABLE)    \
    DLL(tex,     "./tex.dll",     TEXER_FUNCTION_TABLE)   \
    DLL(compute, "./compute.dll", COMPUTE_FUNCTION_TABLE)

#define CUTE_FUNCTION_TABLE(FUNC, DLL) \
    FUNC(DLL, void, dialogue, Routine*, int)

#define TEXER_FUNCTION_TABLE(FUNC, DLL) \
    FUNC(DLL, int, generate_textures, state_t*, float) \
    FUNC(DLL, int, alloc_texture, state_t*)

#define COMPUTE_FUNCTION_TABLE(FUNC, DLL) \
    FUNC(DLL, int, on_load, state_t*) \
    FUNC(DLL, void, update, state_t*, char, double, double) \
    FUNC(DLL, void, draw,   state_t*)


/* TODO forward declarations */
typedef struct Routine Routine;
typedef struct state_t state_t;

#define DLL_TABLE_IMPLEMENTATION
#include "../dll_table.h"

#include <unistd.h>
int main() {
    tex_dll_open();
    tex_dll_load_functions();

    if (generate_textures) { printf("found\n"); }
    else          { printf("not found\n"); }

    if (alloc_texture) { printf("found\n"); }
    else          { printf("not found\n"); }

    cute_dll_open();
    cute_dll_load_functions();

    if (dialogue) { printf("found\n"); }
    else          { printf("not found\n"); }

    sleep(1);
    if (compute_dll_open()) printf("success\n");
    sleep(1);
    compute_dll_load_functions();

    if (on_load) { printf("found\n"); }
    else          { printf("not found\n"); }
}
