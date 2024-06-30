/* USAGE CODE: define x-macros before including dll_table.h */
#define RECODE_DLL_TABLE(DLL) \
    DLL(sound, "code.dll",  SOUND_FUNCTION_TABLE) \
    DLL(grafx, "grafx.dll", GRAFX_FUNCTION_TABLE)
    
#define GRAFX_FUNCTION_TABLE(FUNC, DLL) \
    FUNC(DLL, void, init_graphics, int) \
    FUNC(DLL, void, draw, void)

#define SOUND_FUNCTION_TABLE(FUNC, DLL) \
    FUNC(DLL, void, init_sound, int) \
    FUNC(DLL, void, play_sound, void)
// #include "dll_table.h"

/* static function pointers */
#define FUNC_POINTERS(dll, ret, name, ...) static ret (*name)(__VA_ARGS__);
#define GRAB_TABLE(dll, path, table, ...) table(FUNC_POINTERS,_)
RECODE_DLL_TABLE(GRAB_TABLE)

/* static dll handles */

/* function declarations */
#define MAIN_FUNCTIONS(dll, path, table, ...)     \
                void dll##_dll_changed_on_disk(); \
                void dll##_dll_load();            \
                void dll##_dll_load_functions();  \
RECODE_DLL_TABLE(MAIN_FUNCTIONS)

#ifdef DLL_TABLE_IMPLEMENTATION

/* dll_load_functions implementation */
#define LOAD_FUNCTIONS(dll, ret, name, ...) name = (__VA_ARGS__) SDL_LoadFunction(dll, #name);
#define IMPLEMENT_DLL_LOAD_FUNCTIONS(dll, path, table, ...) void dll##_dll_load_functions() { table(LOAD_FUNCTIONS, dll) };
RECODE_DLL_TABLE(IMPLEMENT_DLL_LOAD_FUNCTIONS) 


#endif

#undef LOAD_FUNCTIONS
