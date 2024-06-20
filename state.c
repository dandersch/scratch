/*
** A proof-of-concept of specifying a finite-state-machine design as an x-macro table.
** The table defines for every row what should happen to a state for a given transition.
** Every transition gets two columns: the target state and code to run (e.g. a function to call).
**
** The x-macro code generates enums and strings for states and transitions and a
** switch statement that implements the control flow.
**
** Current drawbacks:
** - Adding a transition requires code changes to the switch-statement generation code
** - Functions to be called have to be specified by the user
*/
#define GAME_STATES(STATE,TRANSITION)                                TRANSITION(NONE)        TRANSITION(PLAY)                       TRANSITION(BACK)\
STATE(MENU,                                       MENU,     game_state_menu_update(),           INGAME,     ,         EXIT, game_state_menu_exit() )\
STATE(INGAME,                                   INGAME,   game_state_ingame_update(),           INGAME,     ,         MENU,                        )\
STATE(CUTSCENE,                               CUTSCENE, game_state_cutscene_update(),           INGAME,     ,         MENU,                        )\
STATE(EXIT,                                       EXIT,            game_running = 0 ,             EXIT,     ,         EXIT,                        )

#define EXPAND_TO_NOTHING(...)
typedef enum game_transition_e
{
    #define TRANSITION_ENUM(name, ...) GAME_TRANSITION_TO_##name,
    GAME_STATES(EXPAND_TO_NOTHING,TRANSITION_ENUM)
    GAME_TRANSITION_COUNT
} game_transition_e;

const char* game_transition_strings[GAME_TRANSITION_COUNT] = {
    #define TRANSITION_ENUM_STRING(state, ...) #state,
    GAME_STATES(EXPAND_TO_NOTHING, TRANSITION_ENUM_STRING)
};

typedef enum game_state_e {
    #define STATE_ENUM(state, ...) GAME_STATE_##state,
    GAME_STATES(STATE_ENUM, EXPAND_TO_NOTHING)
    GAME_STATE_COUNT
} game_state_e;

const char* game_state_strings[GAME_STATE_COUNT] = {
    #define STATE_ENUM_STRING(state, ...) #state,
    GAME_STATES(STATE_ENUM_STRING, EXPAND_TO_NOTHING)
};

typedef struct game_state_t      { game_state_e type;      } game_state_t;
typedef struct game_transition_t { game_transition_e type; } game_transition_t;

#include <stdio.h>
#include <unistd.h>
void game_state_menu_update()     { printf("Menu...\n"); }
void game_state_ingame_update()   { printf("Ingame...\n"); }
void game_state_cutscene_update() { printf("Cutscene...\n"); }
void game_state_menu_exit()       { printf("EXIT FROM MENU\n"); }

int main()
{
    game_state_t game_state;
    game_state.type = GAME_STATE_MENU;
    game_transition_t game_transition;
    game_transition.type = GAME_TRANSITION_TO_NONE;

    int game_running = 1;
    while (game_running)
    {
        /* log */
        printf("STATE:      %s\n", game_state_strings[game_state.type]);
        printf("TRANSITION: %s\n", game_transition_strings[game_transition.type]);

        /* detect transition */
        game_transition.type = GAME_TRANSITION_TO_NONE;
        char input = getchar();
        getchar(); /* consume newline */
        switch (input)
        {
            case 'x': game_transition.type = GAME_TRANSITION_TO_BACK;  break;
            //case 'm': game_transition.type = GAME_TRANSITION_TO_MENU; break;
            case 'g': game_transition.type = GAME_TRANSITION_TO_PLAY; break;
        }

        /* the state machine */
        switch (game_state.type)
        {
            #define EXPAND_SWITCH(state, none, none_func, play, play_func, back, back_func, ...) case GAME_STATE_##state: { switch (game_transition.type) { \
              case (GAME_TRANSITION_TO_NONE) : { game_state.type = GAME_STATE_##none; none_func; } break;      \
              case (GAME_TRANSITION_TO_PLAY) : { game_state.type = GAME_STATE_##play; play_func; } break;      \
              case (GAME_TRANSITION_TO_BACK) : { game_state.type = GAME_STATE_##back; back_func; } break;      \
            } } break;

            GAME_STATES(EXPAND_SWITCH,EXPAND_TO_NOTHING)
        }
    }
};

