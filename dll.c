#include <stdio.h>
#include "type_info.h"

#include "meta_gen.h"

/* ------------------- usage code ---------------------------------------------------------------- */
// Idea to improve usage code : Have a small preprocessor that outputs above
// code for structs that are inside marker macros
#define INTROSPECT(struct_def)
INTROSPECT(typedef struct stuff_t {
    int a;
    float b[4];
    double x;
    char* c;
} stuff_t;)

STRUCT(entity_t);
META(entity_t);

// #define state_t(X, ...)           \
//   X(char,     , foo,    , __VA_ARGS__) \
//   X(float,    , x,      , __VA_ARGS__) \
//   X(float,    , y,      , __VA_ARGS__) \
//   X(float,    , z,      , __VA_ARGS__) \
//   X(int,      , i,      , __VA_ARGS__) \
//   X(entity_t, , e,      , __VA_ARGS__) \
//   X(double,   , w,      , __VA_ARGS__) \
//   X(char,     , baz,    , __VA_ARGS__) \
//   X(char,     , bar,    , __VA_ARGS__)
// STRUCT(state_t);
// META(state_t);

INTROSPECT(typedef struct state_t {
  char      foo;
  float     x;
  float     y;
  float     z;
  int       i;
  double    w;
  char      baz;
  char      bar;
};)

INTROSPECT(struct abc_t {
    int* a;
    float b[4];
    double x;
    char* c[2];
};)

EXPORT memory_layout_t memory_layout_info()
{
  return (memory_layout_t) {state_t_type_info, sizeof(state_t_type_info)/sizeof(state_t_type_info[0])};
}

EXPORT void print_state(state_t* state)
{
    entity_t ent;
    state_t_print_info(*state);
    state->foo = '@';
    state->z   = 4.f;
    state->i   = 1337;
    state->baz = 'Z';
    state->bar = 'x';
}

void test(stuff_t t)
{
}
