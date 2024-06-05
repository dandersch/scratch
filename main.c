#include "common.h"

#include <SDL2/SDL.h>

/* HOT RELOAD */
#include <sys/stat.h> // for checking if dll changed on disk
//#include <dlfcn.h>    // for opening shared objects (requires linking with -ldl)
#include <SDL2/SDL_loadso.h>  // cross platform dll loading
static void* dll_handle = NULL;

#if defined(_WIN32)
  #include <windows.h>
  static const char* DLL_TMP_FILE = "code.tmp.dll";
#endif
static const char* DLL_FILENAME = "./code.dll";

static unsigned int dll_id;
static time_t dll_last_mod;
static void (*hello)(state_t*) = NULL;
static void (*render)(state_t*) = NULL;
static void (*create_shader_program)(state_t*) = NULL;
static void (*init_renderer)(state_t*) = NULL;

static state_t state; // NOTE all program state lives in data section for this example

int platform_load_code()
{
    #if defined(_WIN32)
      const char* dll_file = DLL_TMP_FILE;
    #else
      const char* dll_file = DLL_FILENAME;
    #endif

    // unload old dll
    if (dll_handle)
    {
        hello                 = NULL;
        render                = NULL;
        create_shader_program = NULL;
        init_renderer         = NULL;
        dll_id                = 0;

        //if (dlclose(dll_handle) != 0) { printf("Failed to close DLL\n"); }
        SDL_UnloadObject(dll_handle);
        dll_handle = NULL;


        #ifdef _WIN32
          Sleep(500);
          CopyFile(DLL_FILENAME, DLL_TMP_FILE, 0);
        #endif
    }

    //dll_handle = dlopen(dll_file, RTLD_NOW);
    dll_handle = SDL_LoadObject(dll_file);
    if (dll_handle == NULL) { printf("Opening DLL failed. Trying again...\n"); }
    // NOTE try opening until it works, otherwise we need to sleep() for a moment to avoid a crash
    while (dll_handle == NULL)
    {
        dll_handle = SDL_LoadObject(dll_file);
        /* dll_handle = dlopen(dll_file, RTLD_NOW); */
    }

    //render = (void (*)(state_t*)) dlsym(dll_handle, "render");
    render = (void (*)(state_t*)) SDL_LoadFunction(dll_handle, "render");
    if (!render) { printf("Finding render function failed\n"); return 0; }

    //create_shader_program = (void (*)(state_t*)) dlsym(dll_handle, "create_shader_program");
    //init_renderer         = (void (*)(state_t*)) dlsym(dll_handle, "init_renderer");
    //hello                 = (void (*)(state_t*)) dlsym(dll_handle, "hello");
    create_shader_program = (void (*)(state_t*)) SDL_LoadFunction(dll_handle, "create_shader_program");
    init_renderer         = (void (*)(state_t*)) SDL_LoadFunction(dll_handle, "init_renderer");
    hello                 = (void (*)(state_t*)) SDL_LoadFunction(dll_handle, "hello");

    hello(&state); // test calling dll function

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

    #ifdef _WIN32
      CopyFile(DLL_FILENAME, DLL_TMP_FILE, 0);
    #endif

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
