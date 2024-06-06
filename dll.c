#include <stdio.h>
#include "type_info.h"

/* ------------------- usage code ---------------------------------------------------------------- */
#define state_t(X, ...)           \
  X(char, foo ,    , __VA_ARGS__) \
  X(float, x,      , __VA_ARGS__) \
  X(float, y,      , __VA_ARGS__) \
  X(float, z,      , __VA_ARGS__) \
  X(int,   i,      , __VA_ARGS__) \
  X(double, w,     , __VA_ARGS__) \
  X(char, baz,     , __VA_ARGS__) \
  X(char, bar ,    , __VA_ARGS__)
STRUCT(state_t);
META(state_t);

#define INTROSPECT(struct_def)

INTROSPECT(struct stuff_t {
  // ...
};)

EXPORT memory_layout_t memory_layout_info()
{
  return (memory_layout_t) {state_t_type_info, sizeof(state_t_type_info)/sizeof(state_t_type_info[0])};
}

EXPORT void print_state(state_t* state)
{
    state_t_print_info(*state);
    state->foo = '@';
    state->z   = 4.f;
    state->i   = 1337;
    state->baz = 'Z';
    state->bar = 'x';
}
