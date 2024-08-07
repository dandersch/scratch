#include "common.h"
#include "pch.h"

/* forward declarations */
typedef struct state_t state_t;

/* HOT RELOAD */
#define DLL_FILENAME "./code.dll"
#define DLL_TABLE(X)               \
    X(int,  on_load,   state_t**)  \
    X(void, render,    state_t*)   \
    X(int,  on_reload, state_t*)   \
    X(void, update, state_t*, float, float, int) \
    X(void, mouse_click, state_t*, int, int) // TODO generalize

typedef struct dll_t {
    #define DLL_FUNCTIONS(ret,func,...) ret (*func)(__VA_ARGS__);
    DLL_TABLE(DLL_FUNCTIONS)

    void* handle;
    time_t last_mod;
    const char* file;
} dll_t;
static dll_t dll;

#ifdef _WIN32
  #include <windows.h>
  #define DLL_FILE_TMP DLL_FILENAME ".tmp"
  #define USE_TMP_DLL_IF_WIN32(sleeptime)   \
     Sleep(sleeptime); CopyFile(DLL_FILENAME, DLL_FILE_TMP, 0)
#else
  #define USE_TMP_DLL_IF_WIN32(sleeptime)
  #define DLL_FILE_TMP DLL_FILENAME // no tmp dll needed
#endif

//static state_t state; // NOTE all program state lives in data section for this example
static state_t* state; // allocated by dll
static SDL_GLContext context;
static SDL_Window* window;

int platform_load_code(dll_t* dll)
{
    dll->file = DLL_FILE_TMP;

    if (dll->handle) /* unload old dll */
    {
        #define SET_TO_NULL(ret, func, ...) dll->func = NULL;
        DLL_TABLE(SET_TO_NULL)

        SDL_UnloadObject(dll->handle);
        dll->handle = NULL;

        USE_TMP_DLL_IF_WIN32(500); /* NOTE: Linux can load new dll directly */
    }

    dll->handle = SDL_LoadObject(dll->file);
    if (dll->handle == NULL) { printf("Opening DLL failed. Trying again...\n"); }
    while (dll->handle == NULL) /* NOTE keep trying to load dll */
    {
        dll->handle = SDL_LoadObject(dll->file);
    }

    /* load all dll functions (and print out any not found) */
    #define LOAD_FUNCTION(ret, func, ...) \
        dll->func = (ret (*)(__VA_ARGS__)) SDL_LoadFunction(dll->handle, #func); \
        if (!dll->func) { printf("Error finding function: %s\n", #func); return 0; }
    DLL_TABLE(LOAD_FUNCTION)

    return 1;
}

int main(int argc, char* args[])
{
    /* init */
    {
        /* sdl initialization boilerplate */
        if (SDL_Init(SDL_INIT_VIDEO) < 0) { fprintf(stderr, "Failed to initialize SDL: %s\n", SDL_GetError()); return -1; }
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG); // only for 4.3+
        window = SDL_CreateWindow("OpenGL Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
        if (!window) { fprintf(stderr, "Failed to create SDL window: %s\n", SDL_GetError()); return -1; }
        context = SDL_GL_CreateContext(window); // opengl context
        if (!context) { fprintf(stderr, "Failed to create OpenGL context: %s\n", SDL_GetError()); return -1; }

    }

    #ifdef _WIN32
      CopyFile(DLL_FILENAME, DLL_FILE_TMP, 0);
      Sleep(300);
    #endif

    // initial loading of dll
    int code_loaded = platform_load_code(&dll);
    if (!code_loaded) { fprintf(stderr, "Couldn't load dll\n"); exit(-1); }
    struct stat attr;
    stat(DLL_FILENAME, &attr);
    dll.last_mod = attr.st_mtime;

    dll.on_load(&state);

    int running = 1;
    SDL_Event event;
    int mouse_x, mouse_y;
    float pos_x = 0, pos_y = 0;
    while (running)
    {
        /* check if dll has changed on disk */
        if ((stat(DLL_FILENAME, &attr) == 0) && (dll.last_mod != attr.st_mtime))
        {
            printf("Attempting code hot reload...\n");
            platform_load_code(&dll);
            dll.on_reload(state);
            dll.last_mod = attr.st_mtime;
        }

        /* event handling */
        int mouse_wheel = 0;
        while (SDL_PollEvent(&event)) {
            switch (event.type)
            {
                case SDL_QUIT: { running = 0; } break;
                case SDL_MOUSEMOTION:
                {
                    mouse_x = event.motion.x;
                    mouse_y = event.motion.y;
                } break;
                case SDL_MOUSEBUTTONDOWN: {
                    dll.mouse_click(state, mouse_x, mouse_y);
                } break;

                case SDL_MOUSEWHEEL:
                {
                    mouse_wheel = event.wheel.y;
                } break;

                case SDL_KEYDOWN:
                {
                    SDL_Keycode keycode = event.key.keysym.sym;
                    int is_down         = (event.key.state == SDL_PRESSED);

                    if(event.key.repeat == 0)
                    {
                        if (is_down)
                        {
                            if((keycode >= SDLK_F1) && (keycode <= SDLK_F12)) {}
                            if (keycode == SDLK_SPACE) { dll.mouse_click(state, mouse_x, mouse_y); }

                            if (keycode == SDLK_UP)    { pos_y += 10; }
                            if (keycode == SDLK_DOWN)  { pos_y -= 10; }
                            if (keycode == SDLK_LEFT)  { pos_x -= 10; }
                            if (keycode == SDLK_RIGHT) { pos_x += 10; }
                        }
                    }


                } break;
            }
        }
        dll.update(state,pos_x,pos_y,mouse_wheel);
        dll.render(state);

        SDL_GL_SwapWindow(window);
    }

    return 0;
}
