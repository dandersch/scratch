#include <cute.h>
//using namespace Cute;

#include <cute_routine.h>

int main(int argc, char* argv[])
{
    // Create a window with a resolution of 640 x 480.
    int options = CF_APP_OPTIONS_WINDOW_POS_CENTERED | CF_APP_OPTIONS_DEFAULT_GFX_CONTEXT;
    CF_Result result = cf_make_app("Fancy Window Title", 0, 0, 640, 480, options, argv[0]);
    if (cf_is_error(result)) { return -1; }

    #define SPRITE_COUNT 48
    CF_Sprite sprite[SPRITE_COUNT];
    for (int i = 0; i < SPRITE_COUNT; i++) {
        char path[20];
        sprintf(path, "res/Slice %i.png", i+1);
        //printf("%s\n", path);
        sprite[i] = cf_make_easy_sprite_from_png(path,  &result);

        sprite[i].scale       = cf_add_v2(sprite[i].scale, (CF_V2){2,2});
    }
    if (cf_is_error(result)) { printf("failed to make sprite\n"); return -1; }

    while (cf_app_is_running()) {
        cf_app_update(NULL);

        // All your game logic and updates go here...
        cf_app_draw_onto_screen(1);

        CF_V2 move = {0};
        for (int i = 0; i < SPRITE_COUNT; i++) {
            switch (i%4) {
                case 0 : move = (CF_V2){1,0};  break;
                case 1 : move = (CF_V2){0,1};  break;
                case 2 : move = (CF_V2){0,-1}; break;
                case 3 : move = (CF_V2){-1,0}; break;
            }
            sprite[i].transform.p = cf_add_v2(sprite[i].transform.p, move);
            cf_draw_sprite(&sprite[i]);
        }
    }

    cf_destroy_app();

    return 0;
}
