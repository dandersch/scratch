/*
** A proof-of-concept of specifying a finite-state-machine design as an x-macro table.
** The table defines for every row what should happen to a state when given a transition.
** Every transition is represented in the columns (except for the first column, which contains all states).
** Every (state,transition) entry contains a TARGET macro containing what state to transition to for what transition and
** what code to run (e.g. a function to call).
**
** The x-macro code generates enums and strings for states and transitions and a
** switch statement that implements the control flow.
**
** Current drawbacks:
** - Functions to be called have to be specified by the user
*/
#define GAME_STATES(STATE,TRANSITION,TARGET) \
                TRANSITION(NONE)                                         TRANSITION(PLAY)                          TRANSITION(BACK)                            TRANSITION(PAUSE) \
STATE(MENU,     TARGET(NONE,  MENU,     game_state_menu_update())        TARGET(PLAY, INGAME,                 )    TARGET(BACK, EXIT, game_state_menu_exit())  TARGET(PAUSE, MENU,                            )  )\
STATE(INGAME,   TARGET(NONE,  INGAME,   game_state_ingame_update())      TARGET(PLAY, INGAME,                 )    TARGET(BACK, MENU,                       )  TARGET(PAUSE, INGAME,   printf("pause game\n") )  )\
STATE(CUTSCENE, TARGET(NONE,  CUTSCENE, game_state_cutscene_update())    TARGET(PLAY, INGAME,                 )    TARGET(BACK, MENU,                       )  TARGET(PAUSE, CUTSCENE, printf("pausing\n")    )  )\
STATE(EXIT,     TARGET(NONE,  EXIT,     game_running = 0; )              TARGET(PLAY, EXIT, game_running = 0;)    TARGET(BACK, EXIT, game_running = 0;      )  TARGET(PAUSE, EXIT,  game_running = 0;         )  )

#define EXPAND_TO_0(...) 0
#define EXPAND_TO_NOTHING(...)
enum {
    #define TRANSITION_ENUM(name, ...) GAME_TRANSITION_TO_##name,
    GAME_STATES(EXPAND_TO_NOTHING,TRANSITION_ENUM, EXPAND_TO_0)
    GAME_TRANSITION_COUNT
};
const char* game_transition_strings[GAME_TRANSITION_COUNT] = {
  #define TRANSITION_ENUM_STRING(state, ...) #state,
  GAME_STATES(EXPAND_TO_NOTHING, TRANSITION_ENUM_STRING, EXPAND_TO_0)
};
enum {
    #define STATE_ENUM(state, ...) GAME_STATE_##state,
    GAME_STATES(STATE_ENUM, EXPAND_TO_NOTHING, EXPAND_TO_0)
    GAME_STATE_COUNT
};
const char* game_state_strings[GAME_STATE_COUNT] = {
    #define STATE_ENUM_STRING(state, ...) #state,
    GAME_STATES(STATE_ENUM_STRING, EXPAND_TO_NOTHING, EXPAND_TO_0)
};

#include <stdio.h>
#include <unistd.h>
void game_state_menu_update()     { printf("Menu...\n"); }
void game_state_ingame_update()   { printf("Ingame...\n"); }
void game_state_cutscene_update() { printf("Cutscene...\n"); }
void game_state_menu_exit()       { printf("EXIT FROM MENU\n"); }
int main()
{
    int game_state      = GAME_STATE_MENU;
    int game_transition = GAME_TRANSITION_TO_NONE;
    int game_running = 1;
    while (game_running)
    {
        /* log */
        printf("STATE:      %s\n", game_state_strings[game_state]);
        printf("TRANSITION: %s\n", game_transition_strings[game_transition]);

        if (game_state == GAME_STATE_EXIT) { break; } // avoid one frame of delay

        /* detect transition */
        game_transition = GAME_TRANSITION_TO_NONE;
        char input = getchar(); getchar(); /* consume newline */
        switch (input)
        {
            case 'x': game_transition = GAME_TRANSITION_TO_BACK;  break;
            case 'p': game_transition = GAME_TRANSITION_TO_PAUSE; break;
            case 'g': game_transition = GAME_TRANSITION_TO_PLAY;  break;
        }

        /* the state machine */
        switch (game_state)
        {
            #define TARGET_CASE(transition, target_state, code) case (GAME_TRANSITION_TO_##transition) : { game_state = GAME_STATE_##target_state; code;  } break;
            #define EXPAND_SWITCH(state, targets) case GAME_STATE_##state: { switch (game_transition) { targets } } break;
            GAME_STATES(EXPAND_SWITCH,EXPAND_TO_NOTHING,TARGET_CASE)
            default: { /* UNREACHABLE */ } break;
        }
    }
};

