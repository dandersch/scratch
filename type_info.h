#pragma once

/* Idea: format a struct definition as an X-Macro table and use its fields to */
/* generate a type info array and a printing function for the struct. */

// current limitations:
// - doesn't handle arrays and array sizes in a good way
// - can't handle nested structures
// - can't distinguish between primitive types and structs
//   see https://en.wikipedia.org/wiki/C_data_types for possible primitive datatypes
// - print function doesn't print values in a readable way (only bytes)

#define OFFSET_OF(type, member) __builtin_offsetof(type, member)
#define EXPORT // TODO

/* struct definitions using X-macros */
#define FILL_FIELDS(type,name,array, ...) type name array;
#define STRUCT_NAME_START(name) typedef struct name {
#define STRUCT_NAME_END(name)   } name
#define STRUCT(name) STRUCT_NAME_START(name) name(FILL_FIELDS) STRUCT_NAME_END(name)

/* array containing serializing data using X-macros */
typedef struct meta_struct_t {
    size_t offset;
    size_t size;
    const char*  name;
    const char*  type;
} meta_struct_t;
#define create_print_function(struct_name)                                                   \
  void struct_name##_print_info( struct_name foo) {                                          \
    int member_count = (sizeof(struct_name##_type_info)/sizeof(struct_name##_type_info[0])); \
    printf("member count of %s: %i \n", #struct_name, member_count);                         \
    for (int i = 0; i < member_count; i++) {                                                 \
        printf("   member %s:\n",        struct_name##_type_info[i].name);                     \
        printf("      offset: %zu  \n",  struct_name##_type_info[i].offset );                  \
        printf("      size:   %zu  \n",  struct_name##_type_info[i].size );                    \
        printf("      type:   %s   \n",  struct_name##_type_info[i].type );                    \
        printf("      value:  %c   \n",  *(((char*) &foo) + struct_name##_type_info[i].offset)); \
        /*printf("      value:  %.*x \n",  struct_name##_type_info[i].size,            */        \
        /*    (unsigned)*(unsigned char*) ((char*) &foo) + struct_name##_type_info[i].offset);*/ \
    }                                                                                          \
  }
#define META_MEMBER(b,c,d,...) { OFFSET_OF(__VA_ARGS__, c) , sizeof(b), #c, #b#d },  // NOTE workaround
#define META(name) meta_struct_t name##_type_info[] =  { name(META_MEMBER, name) }; create_print_function(name)

typedef struct memory_layout_t { meta_struct_t* type_info; int member_count; } memory_layout_t;

#include <stdio.h>
void print_memory_layout(meta_struct_t* members, int member_count) {
  printf("member count: %i \n", member_count);
  for (int i = 0; i < member_count; i++) {
      printf("   member %s:\n",        members[i].name);
      printf("      offset: %zu  \n",  members[i].offset );
      printf("      size:   %zu  \n",  members[i].size );
      printf("      type:   %s   \n",  members[i].type );
  }
}
