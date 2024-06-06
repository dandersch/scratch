#pragma once
#include <stdio.h>
#include <string.h>

#if !defined(_MSC_VER)
    #define OFFSET_OF(type, member) __builtin_offsetof(type, member)
#else
    #define OFFSET_OF(s,m) ((size_t)&(((s*)0)->m))
#endif
#define EXPORT // TODO

// current limitations:
// - doesn't handle arrays and array sizes in a good way
// - can't handle nested structures
// - doesnt follow pointers
// - can't distinguish between primitive types and structs
//   see https://en.wikipedia.org/wiki/C_data_types for possible primitive datatypes

#define TYPES_AND_FORMATTERS(X)\
  X(float,  "%f")\
  X(double, "%f")\
  X(int,    "%i")\
  X(char,   "%c" )

#define STRING_COMPARE(x_type, format, ...) else if (!strcmp(#x_type, type)) { printf("    value:  " format "\n", *((x_type*) ptr_to_value)); }
void print_value_based_on_type(char* type, void* ptr_to_value)
{
    if (0)  {}
    TYPES_AND_FORMATTERS(STRING_COMPARE)
    else { printf("Unknown type"); }
}

/* struct definitions using X-macros */
#define FILL_FIELDS(type,ptr,name,array, ...) type ptr name array;
#define STRUCT_NAME_START(name) typedef struct name {
#define STRUCT_NAME_END(name)   } name
#define STRUCT(name) STRUCT_NAME_START(name) name(FILL_FIELDS) STRUCT_NAME_END(name)

/* array containing serializing data using X-macros */
typedef struct meta_struct_t {
    size_t offset;
    size_t size;
    char*  name;
    char*  type;
} meta_struct_t;
#define create_print_function(struct_name)                                                   \
  void struct_name##_print_info( struct_name foo) {                                          \
    int member_count = (sizeof(struct_name##_type_info)/sizeof(struct_name##_type_info[0])); \
    printf("member count of %s: %i \n", #struct_name, member_count);                         \
    for (int i = 0; i < member_count; i++) {                                                 \
        printf("  member %s:\n",        struct_name##_type_info[i].name);                     \
        printf("    offset: %zu  \n",  struct_name##_type_info[i].offset );                  \
        printf("    size:   %zu  \n",  struct_name##_type_info[i].size );                    \
        printf("    type:   %s   \n",  struct_name##_type_info[i].type );                    \
        print_value_based_on_type(struct_name##_type_info[i].type, (void*) (((char*) &foo) + struct_name##_type_info[i].offset)); \
    }                                                                                          \
  }
#define META_MEMBER(a,b,c,d,...) { OFFSET_OF(__VA_ARGS__, c) , sizeof(a), #c, #a#b#d },  // NOTE workaround
#define META(name) meta_struct_t name##_type_info[] =  { name(META_MEMBER, name) }; create_print_function(name)

typedef struct memory_layout_t { meta_struct_t* type_info; int member_count; } memory_layout_t;

void print_memory_layout(meta_struct_t* members, int member_count) {
  printf("member count: %i \n", member_count);
  for (int i = 0; i < member_count; i++) {
      printf("   member %s:\n",        members[i].name);
      printf("      offset: %zu  \n",  members[i].offset );
      printf("      size:   %zu  \n",  members[i].size );
      printf("      type:   %s   \n",  members[i].type );
  }
}

#define entity_t(X, ...)           \
  X(float, , x,   , __VA_ARGS__) \
  X(float, , y,   , __VA_ARGS__) \
  X(float, , z,   , __VA_ARGS__)
