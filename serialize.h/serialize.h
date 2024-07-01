#include <stdio.h>
#include <stdlib.h>

/* struct definitions */

#define FILL_FIELDS(type,ptr,name,array, ...) type ptr name array;
#define STRUCT(name) typedef struct name { name(FILL_FIELDS) } name

#define entity_t(X, ...) \
  X( char,    ,        foo,       [2],        ,    _array,           0  )  \
  X( int,     ,   bz_count,          ,        ,          ,           0  )  \
  X( float,  *,         bz,          ,    _ptr,          ,  t.bz_count  )
  /* type   ptr       name      array   is_ptr    is_array        count  */
STRUCT(entity_t);

#define state_t(X, ...) \
  X( char,      ,        foo,          ,        ,          ,           0  )  \
  X( entity_t,  ,        ent,          ,        ,          ,           0  )
STRUCT(state_t);

#define primitive_types(X, ...)  \
   X(char,    )     \
   X(float,   )     \
   X(int,     )

static void serialize_char(char c, int count)       { printf("%c ", c); }
static void serialize_char_array(char c[], int count) { for (int i =0; i < count; i++) { printf("%c ", c[i]); } }
static void serialize_float(float f, int count)     { printf("%f ", f); }
static void serialize_int(int i, int count)         { printf("%i ", i); }

static void serialize_char_ptr(char* c, int count)    { printf("%p ", c); }
static void serialize_float_ptr(float* c, int count)  { for(int i = 0; i < count; i++) { printf("%f ", c[i]); } }

#define SERIALIZE(type, ptr, name, array, is_ptr, is_array, count) serialize_##type##is_ptr##is_array(t.name, count);
#define GENERATE_SERIALIZE_FUNCTION(member) \
    void serialize_##member(member t, int count) \
    {                                 \
         member(SERIALIZE)            \
    }

GENERATE_SERIALIZE_FUNCTION(entity_t)
GENERATE_SERIALIZE_FUNCTION(state_t)

int main()
{
   state_t test;
   test.foo = 'c';
   test.ent.bz_count = 3;
   test.ent.bz = (float*) malloc(sizeof(test.ent) * test.ent.bz_count);
   test.ent.bz[0] = 1.2f;
   test.ent.bz[1] = 2.2f;
   test.ent.bz[2] = 3.2f;
   serialize_state_t(test, 1);
}
