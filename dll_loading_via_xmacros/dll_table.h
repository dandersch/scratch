#pragma once
/* TODO put in implementation */
#include <sys/stat.h>
#include <dlfcn.h> // NOTE linux only

/* static function pointers */
#define FUNC_POINTERS(dll, ret, name, ...) static ret (*name)(__VA_ARGS__);
#define GRAB_TABLE(dll, path, table, ...) table(FUNC_POINTERS,_)
#define DLL_FUNC_PTRS RECODE_DLL_TABLE(GRAB_TABLE)

/* static dll handles + modification times */
#define DLL_HANDLE(dll, path, table, ...) static void* dll##_dll_handle; static time_t dll##_last_mod;
#define DLL_HANDLES RECODE_DLL_TABLE(DLL_HANDLE)

/* function declarations */
#define MAIN_FUNCTIONS(dll, path, table, ...)     \
                int  dll##_dll_changed_on_disk(); \
                int  dll##_dll_open();            \
                void dll##_dll_close();           \
                void dll##_dll_load_functions();
#define DLL_FUNCTION_DECL RECODE_DLL_TABLE(MAIN_FUNCTIONS)

/* dll_open implementation */
#define IMPLEMENT_DLL_OPEN(dll, path, table, ...) int dll##_dll_open() { dll##_dll_handle = dlopen(path, RTLD_NOW); struct stat attr; stat(path, &attr); dll##_last_mod = attr.st_mtime; if (!dll##_dll_handle) { return 0; } else { return 1; } };
#define DLL_OPEN_FUNCTION RECODE_DLL_TABLE(IMPLEMENT_DLL_OPEN)

/* dll_close implementation */ // TODO should also set function pointers to NULL
#define IMPLEMENT_DLL_CLOSE(dll, path, table, ...) void dll##_dll_close() { dlclose(dll##_dll_handle); };
#define DLL_CLOSE_FUNCTION RECODE_DLL_TABLE(IMPLEMENT_DLL_CLOSE)

/* dll_changed_on_disk implementation */
#define IMPLEMENT_DLL_CHANGED(dll, path, table, ...) int dll##_dll_changed_on_disk() { struct stat attr; int ret = ((stat(path, &attr) == 0) && (dll##_last_mod != attr.st_mtime)); return ret; }
#define DLL_CHANGED_FUNCTION RECODE_DLL_TABLE(IMPLEMENT_DLL_CHANGED)

/* dll_load_functions implementation */
// function pointer cast : (ret (*)(__VA_ARGS__))
#define LOAD_FUNCTIONS(dll, ret, name, ...) name =  dlsym(dll##_dll_handle, #name);
#define IMPLEMENT_DLL_LOAD_FUNCTIONS(dll, path, table, ...) void dll##_dll_load_functions() { table(LOAD_FUNCTIONS, dll) };
#define DLL_LOAD_FUNCTIONS RECODE_DLL_TABLE(IMPLEMENT_DLL_LOAD_FUNCTIONS)
