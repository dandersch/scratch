/* Idea: format a struct definition as an X-Macro table and use its fields to */
/* generate a type info array... TODO */

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

/* HOT RELOAD */
#include <sys/stat.h> // for checking if dll changed on disk
#include <dlfcn.h>    // for opening shared objects (requires linking with -ldl)
static void* dll_handle = NULL;
static const char* DLL_FILENAME = "./code.dll";

#include "type_info.h"
typedef struct state_t state_t;
typedef memory_layout_t (*memory_layout_info_fn)();
typedef void (*print_state_fn)(state_t*);

int main()
{
    void* memory = malloc(512);

    /* initial loading of dll */
    dll_handle = dlopen(DLL_FILENAME, RTLD_NOW);
    if (dll_handle == NULL) { printf("Opening DLL failed. Trying again...\n"); }
    memory_layout_info_fn memory_layout_info = (memory_layout_info_fn) dlsym(dll_handle, "memory_layout_info");
    print_state_fn print_state               = (print_state_fn) dlsym(dll_handle, "print_state");
    if (!memory_layout_info || !print_state) { printf("Finding functions failed\n"); return 0; }
    print_state((state_t*) memory);

    /* check if dll has changed on disk */
    struct stat attr;
    stat(DLL_FILENAME, &attr);
    unsigned int dll_id = attr.st_ino;
    unsigned int dll_last_mod = attr.st_mtime;
    while (1) if ((stat(DLL_FILENAME, &attr) == 0) && (dll_last_mod != attr.st_mtime))
    {
        printf("Attempting code hot reload...\n");

        memory_layout_t old_layout = memory_layout_info();

        if (dll_handle) {
            print_state          = NULL;
            memory_layout_info   = NULL;
            dll_id               = 0;

            if (dlclose(dll_handle) != 0) { printf("Failed to close DLL\n"); }
            dll_handle = NULL;
        }

        /* loading of new dll */
        usleep(50000);
        dll_handle = dlopen(DLL_FILENAME, RTLD_NOW);
        if (dll_handle == NULL) { printf("Opening DLL failed. Trying again...\n"); }
        memory_layout_info = (memory_layout_info_fn) dlsym(dll_handle, "memory_layout_info");
        print_state        = (print_state_fn) dlsym(dll_handle, "print_state");
        if (!memory_layout_info || !print_state) { printf("Finding functions failed\n"); return 0; }

        memory_layout_t new_layout = memory_layout_info();


        /* check manually if state prints out correctly */
        print_state((state_t*) memory);

        dll_id       = attr.st_ino;
        dll_last_mod = attr.st_mtime;
    }

    return 0;
}
