#define stuff_t(X, ...) \
  X(int,  , a,  , __VA_ARGS__) \
  X(float,  , b, [4], __VA_ARGS__) \
  X(double,  , x,  , __VA_ARGS__) \
  X(char, *, c,  , __VA_ARGS__) 
STRUCT(stuff_t);
META(stuff_t);

#define state_t(X, ...) \
  X(char,  , foo,  , __VA_ARGS__) \
  X(float,  , x,  , __VA_ARGS__) \
  X(float,  , y,  , __VA_ARGS__) \
  X(float,  , z,  , __VA_ARGS__) \
  X(int,  , i,  , __VA_ARGS__) \
  X(double,  , w,  , __VA_ARGS__) \
  X(char,  , baz,  , __VA_ARGS__) \
  X(char,  , bar,  , __VA_ARGS__) 
STRUCT(state_t);
META(state_t);

#define abc_t(X, ...) \
  X(int, *, a,  , __VA_ARGS__) \
  X(float,  , b, [4], __VA_ARGS__) \
  X(double,  , x,  , __VA_ARGS__) \
  X(char, *, c, [2], __VA_ARGS__) 
STRUCT(abc_t);
META(abc_t);

