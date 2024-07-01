#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#define SOKOL_TIME_IMPL
#include "dep/sokol_time.h"

/* - COROUTINE --------------------------------------------------------------------------------------------------------------------------------------------------- */
typedef struct
{
    float elapsed;                     // timer
    int flag;                          // ?
    int index;                         // code-line & case-label
    #define COROUTINE_MAX_DEPTH 8      // how deep into nested coroutines we go
    int line[COROUTINE_MAX_DEPTH];     // keeps track of the sequence point of nested coroutines
    void* data;
} coroutine_t;
#define COROUTINE_CASE_OFFSET (1024 * 1024)

static inline void coroutine_init(coroutine_t* co) {
    co->elapsed = 0; co->flag = 0; co->index = 0;
    for (int i = 0; i < COROUTINE_MAX_DEPTH; ++i) co->line[i] = 0;
}
#define COROUTINE_START(co)          do { co->flag = 0; switch (co->line[co->index]) { default: /* default is 0, and will run just once. */
#define COROUTINE_CASE(co, name)     case __LINE__: name: co->line[co->index] = __LINE__;
#define COROUTINE_WAIT(co, time, dt) do { case __LINE__: co->line[co->index] = __LINE__; co->elapsed += dt; do { if (co->elapsed < time) { co->flag = 1; goto __co_end; } else { co->elapsed = 0; } } while (0); } while (0)
#define COROUTINE_EXIT(co)           do { co->flag = 1; goto __co_end; } while (0)
#define COROUTINE_YIELD(co)          do { co->line[co->index] = __LINE__; COROUTINE_EXIT(co); case __LINE__:; } while (0)
/* COROUTINE_CALL(co, call_some_other_func(co, my_params)); */
#define COROUTINE_CALL(co, ...)      co->flag = 0; case __LINE__: COROUTINE_ASSERT(co->index < COROUTINE_MAX_DEPTH); co->line[co->index++] = __LINE__; __VA_ARGS__; co->index--; do { if (co->flag) { goto __co_end; } else { case __LINE__ + COROUTINE_CASE_OFFSET: co->line[co->index] = __LINE__ + COROUTINE_CASE_OFFSET; } } while (0)
#define COROUTINE_END(co)            } co->line[co->index] = 0; __co_end:; } while (0)

#define COROUTINE_JUGGLE_STATE(coroutine, coroutine_state_t, state)                  \
coroutine_state_t* state = (coroutine_state_t*) coroutine->data;                     \
int our_marker = __COUNTER__;                   /* TODO only in debug */             \
if (!state) {                                                                        \
    state = (coroutine_state_t*) arena_push(frame_arena, sizeof(coroutine_state_t)); \
    coroutine->data = state;                                                         \
    coroutine_markers[our_marker].marker = 1; /* TODO only in debug */               \
} else {                                                                             \
    state           = arena_push(frame_arena, sizeof(coroutine_state_t));            \
    state           = memcpy(state, coroutine->data, sizeof(coroutine_state_t));     \
    coroutine->data = state;                                                         \
    coroutine_markers[our_marker].marker = 1; /* TODO only in debug */               \
}

#define COROUTINE_PUSH_STATE(arena, coroutine, coroutine_state_t, state)             \
coroutine_state_t* state = (coroutine_state_t*) coroutine->data;                     \
if (!state) {                                                                        \
    state = (coroutine_state_t*) arena_push(arena, sizeof(coroutine_state_t));       \
    coroutine->data = state;                                                         \
}
// TODO a free-list would work better for deallocating
#define COROUTINE_POP_STATE(arena, coroutine, coroutine_state_t, state)

/* proof-of-concept of detecting stale coroutines */
typedef struct coroutine_marker_t
{
    int marker;
    int watched;
} coroutine_marker_t;
#define COROUTINE_MARKER_MAX 128
static int coroutine_marker_count; // TODO unused, using __COUNTER__ for now
static coroutine_marker_t coroutine_markers[COROUTINE_MARKER_MAX];

/* ------ USAGE ----------------------------------------------------------------------------------------------------------------------------------------------- */
static coroutine_t g_coro;
static coroutine_t* coro = &g_coro;

/* simple memory arena api */
typedef struct arena_t {
    size_t size;
    size_t capacity;
    char data[];
} arena_t;
arena_t* arena_alloc(size_t capacity) {
    arena_t* arena = (arena_t*) malloc(sizeof(arena_t) + capacity);
    memset(arena,0,sizeof(arena_t) + capacity);
    arena->capacity = capacity;
    return arena;
}
void arena_clear(arena_t* arena) {
    memset(arena->data,0,arena->capacity); arena->size = 0;
}
void* arena_push(arena_t* arena, size_t size) {
    void* buf      = NULL;
    size_t push_to = arena->size + size;
    if (push_to <= arena->capacity)
    {
        buf = arena->data + arena->size;
        arena->size = push_to;
    }
    assert(buf);
    return buf;
}
static arena_t* game_arena;
static arena_t* frame_arena;
static arena_t* frame_arena_prev;

void func_to_yield(float dt /*NOTE unused*/)
{
    typedef struct coro_state_t
    {
        float test;
        char array[400];
        void* thing;
    } coro_state_t;

    /*
    ** NOTE: possible drawbacks of performing juggle in the coroutine:
    ** - this only works when the coroutine is guaranteed to be called every frame
    ** - idea: have a global "marker array" of ints that are all 0 by default.
    **   The juggle macro sets a marker to 1 every time it's called. That marker
    **   corresponds to the coroutine by using the __COUNTER__ macro,
    **   (only have this in debug builds)
     */
    COROUTINE_JUGGLE_STATE(coro, coro_state_t, state);
    //COROUTINE_PUSH_STATE(game_arena, coro, coro_state_t, state);

    COROUTINE_START(coro);

    COROUTINE_YIELD(coro);
    state->test = 5.0f;

    assert(state->test == 5.0f);

    COROUTINE_WAIT(coro, 1.f, 1.f);

    assert(state->test == 5.0f);

    COROUTINE_EXIT(coro);
    COROUTINE_END(coro);
}

int main()
{
    game_arena       = arena_alloc(1024);
    frame_arena      = arena_alloc(512);
    frame_arena_prev = arena_alloc(512);

    stm_setup();

    uint64_t ticks_new = stm_now();
    uint64_t ticks_old = stm_now();
    uint64_t frame_counter = 0;
    for (;;)
    {
        ticks_new = stm_now();
        if (stm_sec(stm_diff(ticks_new, ticks_old)) > 1.5)
        {
            ++frame_counter;

            /* reset all coroutine markers */
            for (int i = 0; i < COROUTINE_MARKER_MAX; i++) {
                coroutine_markers[i].marker = 0;
            }

            printf("Frame counter: %lu\n", frame_counter);
            if (frame_counter != 3) // provoke a stale coroutine
                func_to_yield(0);

            /* Check coroutine_markers for stale coroutines */
            {
                for (int i = 0; i < COROUTINE_MARKER_MAX; i++)
                {
                    if (coroutine_markers[i].marker == 1) {
                        coroutine_markers[i].watched = 1; // watch a marked coroutine
                    }
                    if (coroutine_markers[i].watched == 1) {
                        assert(coroutine_markers[i].marker == 1); // stale coroutine
                    }
                }
            }

            /* swap */
            arena_t* temp    = frame_arena;
            frame_arena      = frame_arena_prev;
            frame_arena_prev = temp;

            arena_clear(frame_arena);
            ticks_old = stm_now();
        }
    };
}
