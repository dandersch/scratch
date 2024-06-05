// idea originated from: https://www.chiark.greenend.org.uk/~sgtatham/coroutines.html, Implementation by Andrew Harter

/*
- [x] local state
- [ ] issue with local declartions being skipped by the case label
- [ ] default: catch for out of sync coroutines

the hot-reload / serialisation issue of out-of-sync line changes is probs a non-issue, don't pre-empt it. Focus on what's right in front of me.
*/

// dependency
#define SOKOL_TIME_IMPL
#include "dep/sokol_time.h"

typedef struct Coroutine Coroutine;
struct Coroutine
{
    int line;
    unsigned long start_time;
    void* data;
};

// a coroutine always needs to be started with this
#define CoroutineBegin(coro) switch (coro->line) {case 0: coro->line = 0;

#define CoroutineYield(coro) do { coro->line = __LINE__; return; case __LINE__:;} while(0)
#define CoroutineYieldUntil(coro, condition) while (!(condition)) { CoroutineYield(coro); }
#define CoroutineWait(coro, duration) do {if (coro->start_time == 0.0f) { coro->start_time = stm_now(); } CoroutineYieldUntil(coro, stm_sec(stm_now()) > stm_sec(coro->start_time) + (double)duration); coro->start_time = 0; } while (0)

// end a coroutine that was started by CoroutineBegin with one of these,
// depending on whether you want the coroutine to run again
#define CoroutineEnd(coro) do { coro->line = __LINE__; } while (0); }
#define CoroutineReset(coro) do { coro->line = 0; } while (0); }

// TODO don't know what this does
#define CoroutineIsFinished(coro) (coro->line == -1)

static struct Coroutine g_coro;
static struct Coroutine* coro = &g_coro;

#include <stdio.h>

struct CoroutineState
{
    float test;
    char array[400];
    void* thing;
};


#include <stdlib.h>
#include <assert.h>
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
static arena_t* frame_arena;
static arena_t* frame_arena_prev;

void func_to_yield()
{
    /* JUGGLE */
    // COROUTINE_JUGGLE_MEMORY(coro, coro_state_t, coro_state)
    struct CoroutineState* cs = (struct CoroutineState*) coro->data;
    if (!cs) {
        // inital allocation
        cs = (struct CoroutineState*) arena_push(frame_arena, sizeof(struct CoroutineState));
        coro->data = cs;
    } else {
        // juggle from previous to current frame arena
        cs = arena_push(frame_arena, sizeof(struct CoroutineState));
        cs = memcpy(cs, coro->data,sizeof(struct CoroutineState));
        coro->data = cs;
    }

    /*
    ** NOTE: possible drawbacks of performing juggle in the coroutine:
    ** - this only works when the coroutine is guaranteed to be called every frame
     */

    cs->test = -1.0f;
    printf("0: Float value should be -1.0f: %f\n", cs->test);

    CoroutineBegin(coro);
    cs->test = 1.0f;

    printf("1: Float value should be 1.0f: %f\n", cs->test);

    CoroutineWait(coro, 0.5f);
    cs->test = 3.0f;
    printf("2: Float value should be 3.0f: %f\n", cs->test);

    CoroutineReset(coro);

    cs->test = 5.0f;
    printf("3: Float value should be 5.0f %f\n", cs->test);
}

int main()
{
    frame_arena      = arena_alloc(512);
    frame_arena_prev = arena_alloc(512);

    printf("arena buffer: %p\n", frame_arena->data);
    printf("arena prev buffer: %p\n", frame_arena_prev->data);

    stm_setup();

    uint64_t ticks_new = stm_now();
    uint64_t ticks_old = stm_now();
    for (;;)
    {
        ticks_new = stm_now();
        if (stm_sec(stm_diff(ticks_new, ticks_old)) > 1.5)
        {
            func_to_yield();

            printf("arena cap: %zu, size: %zu, buffer: %p\n", frame_arena->capacity, frame_arena->size, frame_arena->data);

            /* swap */
            arena_t* temp    = frame_arena;
            frame_arena      = frame_arena_prev;
            frame_arena_prev = temp;

            arena_clear(frame_arena);
            ticks_old = stm_now();
        }
    };
}
