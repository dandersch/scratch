// idea originated from: https://www.chiark.greenend.org.uk/~sgtatham/coroutines.html, Implementation by Andrew Harter

/*
- [x] local state
- [ ] issue with local declartions being skipped by the case label
- [ ] default: catch for out of sync coroutines

the hot-reload / serialisation issue of out-of-sync line changes is probs a non-issue, don't pre-empt it. Focus on what's right in front of me.
*/

// dependency
#define SOKOL_TIME_IMPL
#include "sokol_time.h"

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


void func_to_yield()
{
        CoroutineBegin(coro);

        printf("1\n");
        //CoroutineYield(coro);
        CoroutineWait(coro, 4.0f);
        printf("3\n");

        //CoroutineEnd(coro);
        CoroutineReset(coro);
        //CoroutineIsFinished(coro);
}

int main()
{
    stm_setup();

    uint64_t ticks_new = stm_now();
    uint64_t ticks_old = stm_now();
    for (;;)
    {
        ticks_new = stm_now();
        if (stm_sec(stm_diff(ticks_new, ticks_old)) > 1)
        {
            printf("Tick\n");

            func_to_yield();
            printf("2\n");
            func_to_yield();

            ticks_old = stm_now();
        }
    };
}
