#include "common.h"

#include <SDL2/SDL.h>

/* HOT RELOAD */
#include <sys/stat.h>         // for checking if dll changed on disk
#include <SDL2/SDL_loadso.h>  // cross platform dll loading

#define DLL_FILENAME "./code.dll"
#define DLL_TABLE(X)              \
    X(void, render,    state_t*)  \
    X(int,  on_load,   state_t*)  \
    X(int,  on_reload, state_t*)

typedef struct dll_t {
    #define DLL_FUNCTIONS(ret,func,...) ret (*func)(__VA_ARGS__);
    DLL_TABLE(DLL_FUNCTIONS)

    void* handle;
    unsigned int id;
    time_t last_mod;
    const char* file;
} dll_t;
static dll_t dll;

//#define MAKE_STATIC(ret,func,...) static ret (*func)(__VA_ARGS__) = NULL;
//DLL_TABLE(MAKE_STATIC)

static state_t state; // NOTE all program state lives in data section for this example

#ifdef _WIN32
  #include <windows.h>
  #define DLL_FILE_TMP DLL_FILENAME ".tmp"
  #define USE_TMP_DLL_IF_WIN32(sleeptime)   \
     Sleep(sleeptime); CopyFile(DLL_FILENAME, DLL_FILE_TMP, 0)
#else
  #define USE_TMP_DLL_IF_WIN32(sleeptime)
  #define DLL_FILE_TMP DLL_FILENAME // no tmp dll needed
#endif

void copy_file(const char* src_filename, const char* dest_filename); /* cross-platform CopyFile() equivalent from win32 api */
int platform_load_code() // maybe pass a dll_t
{
    dll.file = DLL_FILE_TMP;

    if (dll.handle) /* unload old dll */
    {
        #define SET_TO_NULL(ret, func, ...) dll.func = NULL;
        DLL_TABLE(SET_TO_NULL)

        dll.id  = 0;

        SDL_UnloadObject(dll.handle);
        dll.handle = NULL;

        /* NOTE: Linux could actually load the new dll directly without sleep */
        USE_TMP_DLL_IF_WIN32(500);
    }

    dll.handle = SDL_LoadObject(dll.file);
    if (dll.handle == NULL) { printf("Opening DLL failed. Trying again...\n"); }
    while (dll.handle == NULL) /* NOTE keep trying to load dll */
    {
        dll.handle = SDL_LoadObject(dll.file);
    }

    /* load all dll functions (and print out any not found) */
    #define LOAD_FUNCTION(ret, func, ...) \
        dll.func = (ret (*)(__VA_ARGS__)) SDL_LoadFunction(dll.handle, #func); \
        if (!dll.func) { printf("Error finding function: %s\n", #func); return 0; }
    DLL_TABLE(LOAD_FUNCTION)

    dll.on_reload(&state); // reload shader & "reinit" renderer

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

    #ifdef _WIN32
      CopyFile(DLL_FILENAME, DLL_FILE_TMP, 0);
      Sleep(300);
    #endif

    // initial loading of dll
    int code_loaded = platform_load_code();
    if (!code_loaded) { exit(-1); }
    struct stat attr;
    stat(DLL_FILENAME, &attr);
    dll.id = attr.st_ino;
    dll.last_mod = attr.st_mtime;

    dll.on_load(&state);

    // Main loop
    int running = 1;
    SDL_Event event;
    while (running)
    {
        /* check if dll has changed on disk */
        if ((stat(DLL_FILENAME, &attr) == 0) && (dll.last_mod != attr.st_mtime))
        {
            printf("Attempting code hot reload...\n");
            platform_load_code();
            dll.id       = attr.st_ino;
            dll.last_mod = attr.st_mtime;
        }

        // event handling
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
        }

        dll.render(&state);

        SDL_GL_SwapWindow(state.window);
    }

    return 0;
}
