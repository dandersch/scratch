#include "common.h"

#include <SDL2/SDL.h>

/* HOT RELOAD */
#include <sys/stat.h>         // for checking if dll changed on disk
#include <SDL2/SDL_loadso.h>  // cross platform dll loading
static void* dll_handle = NULL;
static unsigned int dll_id;
static time_t dll_last_mod;
#define DLL_FILENAME "./code.dll"
#define DLL_TABLE(X)                            \
    X(void, hello)                              \
    X(void, render, state_t*)                   \
    X(void, create_shader_program, state_t*)    \
    X(int,  init_renderer, state_t*)
#define MAKE_STATIC(ret,func,...) static ret (*func)(__VA_ARGS__) = NULL;
DLL_TABLE(MAKE_STATIC)

static state_t state; // NOTE all program state lives in data section for this example

void copy_file(const char* src_filename, const char* dest_filename); /* cross-platform CopyFile() equivalent from win32 api */
int platform_load_code()
{
    const char* dll_file = DLL_FILENAME ".tmp";

    // unload old dll
    if (dll_handle)
    {
        #define SET_TO_NULL(ret, func, ...) func = NULL;
        DLL_TABLE(SET_TO_NULL)

        dll_id                = 0;

        SDL_UnloadObject(dll_handle);
        dll_handle = NULL;

        /* NOTE: Linux could actually load the new dll directly without sleep */
        SDL_Delay(500);
        copy_file(DLL_FILENAME, DLL_FILENAME ".tmp");
    }

    dll_handle = SDL_LoadObject(dll_file);
    if (dll_handle == NULL) { printf("Opening DLL failed. Trying again...\n"); }
    while (dll_handle == NULL) /* NOTE keep trying to load dll */
    {
        dll_handle = SDL_LoadObject(dll_file);
    }

    /* load all dll functions (and print out any not found) */
    #define LOAD_FUNCTION(ret, func, ...) \
        func = (ret (*)(__VA_ARGS__)) SDL_LoadFunction(dll_handle, #func); \
        if (!func) { printf("Error finding function: %s\n", #func); return 0; }
    DLL_TABLE(LOAD_FUNCTION)

    hello(); // test calling dll function
    // reload shader & "reinit" renderer
    init_renderer(&state);
    create_shader_program(&state);

    return 1;
}

int main(int argc, char* args[])
{
    /* sdl initialization boilerplate */
    if (SDL_Init(SDL_INIT_VIDEO) < 0) { fprintf(stderr, "Failed to initialize SDL: %s\n", SDL_GetError()); return -1; }
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    state.window = SDL_CreateWindow("OpenGL Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    if (!state.window) { fprintf(stderr, "Failed to create SDL window: %s\n", SDL_GetError()); return -1; }
    state.context = SDL_GL_CreateContext(state.window); // opengl context
    if (!state.context) { fprintf(stderr, "Failed to create OpenGL context: %s\n", SDL_GetError()); return -1; }

    // Initialize GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) { fprintf(stderr, "Failed to initialize GLEW\n"); return -1; }

    copy_file(DLL_FILENAME, DLL_FILENAME ".tmp");

    // initial loading of dll
    int code_loaded = platform_load_code();
    if (!code_loaded) { exit(-1); }
    struct stat attr;
    stat(DLL_FILENAME, &attr);
    dll_id = attr.st_ino;
    dll_last_mod = attr.st_mtime;

    // call dll functions
    init_renderer(&state);
    create_shader_program(&state);

    // Main loop
    int running = 1;
    SDL_Event event;
    while (running)
    {
        /* check if dll has changed on disk */
        if ((stat(DLL_FILENAME, &attr) == 0) && (dll_last_mod != attr.st_mtime))
        {
            printf("Attempting code hot reload...\n");
            platform_load_code();
            dll_id       = attr.st_ino;
            dll_last_mod = attr.st_mtime;
        }

        // event handling
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
        }

        render(&state);

        SDL_GL_SwapWindow(state.window);
    }

    return 0;
}

void copy_file(const char* src_filename, const char* dest_filename) {
    SDL_RWops* src = SDL_RWFromFile(src_filename, "rb");
    if (!src) { fprintf(stderr, "Error opening dll to read: %s\n", SDL_GetError()); return; }

    SDL_RWops* dest = SDL_RWFromFile(dest_filename, "wb");
    if (!dest) { fprintf(stderr, "Error opening tmp dll to write: %s\n", SDL_GetError()); SDL_RWclose(src); return;}

    /* buffered read/write */
    #define BUFFER_SIZE 4096
    char buffer[BUFFER_SIZE];
    size_t bytes_read, bytes_written;
    while ((bytes_read = SDL_RWread(src, buffer, 1, BUFFER_SIZE)) > 0) {
        bytes_written = SDL_RWwrite(dest, buffer, 1, bytes_read);
        if (bytes_written != bytes_read) {
            fprintf(stderr, "Error writing to tmp dll: %s\n", SDL_GetError());
            break;
        }
    }

    /* cleanup */
    SDL_RWclose(src);
    SDL_RWclose(dest);
}
