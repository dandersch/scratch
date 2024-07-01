/*
 * Idea: Format a struct definition as an X-Macro table and use its fields to
 * generate a type info array. Then use this type info to handle the addition of
 * struct members inbetween DLL hot-reloads by fixing up the memory layout given
 * by the old and new dll.
 */

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

/* HOT RELOAD */
#include <sys/stat.h> // for checking if dll changed on disk
#include <dlfcn.h>    // for opening shared objects (requires linking with -ldl)
static void* dll_handle = NULL;
static const char* DLL_FILENAME = "./code.dll";

#include "type_info.h"
typedef struct state_t state_t;
typedef memory_layout_t (*memory_layout_info_fn)();
typedef void (*print_state_fn)(state_t*);
#define MEMORY_SIZE 512

/* ASSUMPTIONS */
// (1) the new layout still fits into memory without reallocating (no bounds check)
// (2) only handle additions of members (no deletions/renames/reordering)
// (3) if member names are the same, they are the same completely
void change_state_to_fit_new_memory_layout(void* state, memory_layout_t old, memory_layout_t new)
{
    void* new_memory = malloc(MEMORY_SIZE);

    // states:
    //   members identical -> copy over
    //   members differ -> advance until members identical again
    int old_layout_index = 0;
    int new_layout_index = 0;
    int finished = 0;

    do
    {
        printf("Old index: %i vs New index: %i\n", old_layout_index, new_layout_index);
        printf("Old member: %s vs New member: %s\n", old.type_info[old_layout_index].name, new.type_info[new_layout_index].name);
        if (!strcmp(old.type_info[old_layout_index].name,
                    new.type_info[new_layout_index].name)) // assumption (3)
        {
            printf("Members are the same\n");

            void* new_address = ((void*) new_memory) + new.type_info[new_layout_index].offset;
            void* old_address = ((void*) state) + old.type_info[old_layout_index].offset;
            memcpy(new_address,  old_address, old.type_info[old_layout_index].size);

            // advance
            old_layout_index++;
            new_layout_index++;
        } else {
            // assumption (2)
            printf("Member was added\n");
            new_layout_index++;
        }

        if (new_layout_index == new.member_count) { finished = 1; }
    } while(!finished);

    memcpy(state, new_memory, MEMORY_SIZE);
    free(new_memory);
}

#include <assert.h>
int main()
{
    void* memory = malloc(MEMORY_SIZE);

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
    while (1)
    {
        if ((stat(DLL_FILENAME, &attr) == 0) && (dll_last_mod != attr.st_mtime))
        {
            printf("Attempting code hot reload...\n");

            /* copy old layout: all data that resides in dll memory needs to be deep copied */
            memory_layout_t old_layout  = memory_layout_info();
            meta_struct_t* old_type_info = malloc(sizeof(meta_struct_t) * old_layout.member_count);
            memcpy(old_type_info, old_layout.type_info, sizeof(meta_struct_t) * old_layout.member_count);
            assert(!memcmp(old_type_info,old_layout.type_info, sizeof(meta_struct_t) * old_layout.member_count));
            /* strings have to be deep copied as well... */
            for (int i = 0; i < old_layout.member_count; i++)
            {
                size_t type_string_size = strlen(old_layout.type_info[i].type) + 1;
                size_t name_string_size = strlen(old_layout.type_info[i].name) + 1;
                old_type_info[i].type   = malloc(type_string_size);
                old_type_info[i].name   = malloc(name_string_size);
                strncpy(old_type_info[i].type, old_layout.type_info[i].type, type_string_size);
                strncpy(old_type_info[i].name, old_layout.type_info[i].name, name_string_size);
            }
            old_layout.type_info = old_type_info;

            if (dll_handle) {
                print_state          = NULL;
                memory_layout_info   = NULL;
                dll_id               = 0;

                if (dlclose(dll_handle) != 0) { printf("Failed to close DLL\n"); }
                dll_handle = NULL;
            }

            /* loading of new dll */
            {
                usleep(50000);
                dll_handle = dlopen(DLL_FILENAME, RTLD_NOW);
                if (dll_handle == NULL) { printf("Opening DLL failed.\n"); }
                memory_layout_info = (memory_layout_info_fn) dlsym(dll_handle, "memory_layout_info");
                print_state        = (print_state_fn)        dlsym(dll_handle, "print_state");
                if (!memory_layout_info || !print_state) { printf("Finding functions failed\n"); return 1; }
            }

            memory_layout_t new_layout = memory_layout_info();

            if (new_layout.member_count > old_layout.member_count) // only handle additions
            {
                change_state_to_fit_new_memory_layout(memory, old_layout, new_layout);
            }

            /* check manually if state prints out correctly */
            print_state((state_t*) memory);

            dll_id       = attr.st_ino;
            dll_last_mod = attr.st_mtime;

            free(old_type_info);
        }
    }

    return 0;
}
