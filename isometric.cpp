#include <stdio.h>

#include <SDL.h>
#include <SDL_events.h>
#include <SDL_render.h>

struct point { int x;    int y;};
struct line  { point p1; point p2; };

static float shear_x = 1;
static float shear_y = 0;

point apply_transform(point p) // apply a shear transform
{
    point sheared_p = {0,0};
    float transform[9] = {      1, shear_x, 0,
                          shear_y,       1, 0,
                                0,       0, 1};
    sheared_p.x = p.x * transform[0] + p.y * transform[1] + 0 * transform[2];
    sheared_p.y = p.x * transform[3] + p.y * transform[4] + 0 * transform[5];
    return sheared_p;
};

point apply_inverse(point p) // remove shear
{
    point sheared_p = {0,0}; // TODO rename
    float inverse[9] = {(1.f / (1.f - (shear_x * shear_y))), shear_x/(shear_x*shear_y - 1.f), 0,
                        shear_y/(shear_x*shear_y - 1.f),     1.f/(1.f - shear_x*shear_y),     0,
                                0,       0, 1};
    sheared_p.x = p.x * inverse[0] + p.y * inverse[1] + 0 * inverse[2];
    sheared_p.y = p.x * inverse[3] + p.y * inverse[4] + 0 * inverse[5];
    return sheared_p;
};

#define SCREEN_WIDTH  720
#define SCREEN_HEIGHT 576

int main(int argc, char** argv)
{
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Window*   window;
    SDL_Renderer* renderer;
    SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &window, &renderer);
    //SDL_version vers;
    //SDL_GetVersion(&vers);
    //printf("%u.%u.%u\n", vers.major, vers.minor, vers.patch); // 2.0.22

    // load in a texture
    SDL_Surface* bitmap  = SDL_LoadBMP("stone.bmp");
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, bitmap);

    // set up points to draw lines. 720/12 = 60, i.e. every 60 pixels two new
    // points: one point up (0), one point down (576)
    line vert_lines[(SCREEN_WIDTH/60)  + 1]; // TODO hardcoded
    line hori_lines[(SCREEN_HEIGHT/48) + 1]; // TODO hardcoded
    for(int i = 0; i < 12; i++)              // TODO hardcoded
    {
        point p1 = {i * 60, 0};
        point p2 = {i * 60, SCREEN_HEIGHT};
        vert_lines[i] = {p1, p2};

        p1 = {0,   i * 48};
        p2 = {SCREEN_WIDTH, i * 48};
        hori_lines[i] = {p1, p2};
    }
    // -1px to be visible on screen
    vert_lines[12] = {{SCREEN_WIDTH-1,               0}, {SCREEN_WIDTH-1,   SCREEN_HEIGHT}};
    hori_lines[12] = {{             0, SCREEN_HEIGHT-1}, {  SCREEN_WIDTH, SCREEN_HEIGHT-1}};

    point mouse_pos = {0,0};

    bool is_running = true;
    while (is_running)
    {
        { /* event loop */
            SDL_Event event;
            while (SDL_PollEvent(&event))
            {
                if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE)
                {
                    is_running = false;
                }

                if (event.type == SDL_KEYDOWN)
                {
                    if (event.key.keysym.sym == SDLK_x) { shear_x -= 0.1f; printf("shear_x: %f\n", shear_x); }
                    if (event.key.keysym.sym == SDLK_c) { shear_x += 0.1f; printf("shear_x: %f\n", shear_x); }
                    if (event.key.keysym.sym == SDLK_s) { shear_y -= 0.1f; printf("shear_x: %f\n", shear_y); }
                    if (event.key.keysym.sym == SDLK_d) { shear_y += 0.1f; printf("shear_x: %f\n", shear_y); }
                }

                if (event.type == SDL_MOUSEMOTION)
                {
                    mouse_pos.x = event.motion.x;
                    mouse_pos.y = event.motion.y;
                }
            }
        } /* end event */

        // TODO probably better to not use pointers
        line* highlight_vert_line = NULL;
        line* highlight_hori_line = NULL;
        { /* update loop */

            // find out between which lines the mouse is
            point grid_mouse = apply_inverse(mouse_pos);
            int vert_idx = (grid_mouse.x - (grid_mouse.x % 60)) / 60;
            int hori_idx = (grid_mouse.y - (grid_mouse.y % 48)) / 48;
            if (vert_idx >= 0 && vert_idx < 12) { highlight_vert_line = &vert_lines[vert_idx]; }
            if (hori_idx >= 0 && hori_idx < 12) { highlight_hori_line = &hori_lines[hori_idx]; }

            // do a lerp for some nice looking animation
            #if 1
            float time_to_lerp       = 5000.f; // TODO use a timestep in the future
            static float timer       = 0;
            static bool  lerp_to_max = true;
            timer += 2.f;
            float interpolant = timer/time_to_lerp;
            float min = -0.5f; float max =  0.5f;
            if (lerp_to_max) {
                               shear_x = min + interpolant * (max - min);
                               shear_y = min + interpolant * (max - min);
                             }
            else             {
                               shear_x = max + interpolant * (min - max);
                               shear_y = max + interpolant * (min - max);
                             }

            if (shear_x >= max) { lerp_to_max = false; timer = 0; }
            if (shear_x <= min) { lerp_to_max = true;  timer = 0; }
            #endif
        } /* end update */

        { /* render loop */
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
            SDL_RenderClear(renderer);


            SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
            for(int i = 0; i < 13; i++)
            {
                if (highlight_vert_line == &vert_lines[i] ||
                    highlight_vert_line == &vert_lines[i-1]) // TODO why does this not go OOB...?
                {
                    SDL_SetRenderDrawColor(renderer, 0, 255, 0, SDL_ALPHA_OPAQUE);
                }
                else { SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE); }

                // apply a shear transform to the lines before rendering
                SDL_RenderDrawLine(renderer, apply_transform(vert_lines[i].p1).x,
                                             apply_transform(vert_lines[i].p1).y,
                                             apply_transform(vert_lines[i].p2).x,
                                             apply_transform(vert_lines[i].p2).y);
                if (highlight_hori_line == &hori_lines[i] ||
                    highlight_hori_line == &hori_lines[i-1])
                {
                    SDL_SetRenderDrawColor(renderer, 0, 255, 0, SDL_ALPHA_OPAQUE);
                }
                else { SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE); }

                SDL_RenderDrawLine(renderer, apply_transform(hori_lines[i].p1).x,
                                             apply_transform(hori_lines[i].p1).y,
                                             apply_transform(hori_lines[i].p2).x,
                                             apply_transform(hori_lines[i].p2).y);
            }

            if (highlight_vert_line && highlight_hori_line)
            {
                // TODO maybe just use SDL_FPoint everywhere instead of our own
                // TODO I believe we are off by one px somewhere...
                point top_L_point = apply_transform({highlight_vert_line->p1.x     , highlight_hori_line->p1.y     });
                point btm_L_point = apply_transform({highlight_vert_line->p1.x     , highlight_hori_line->p1.y + 48});
                point btm_R_point = apply_transform({highlight_vert_line->p1.x + 60, highlight_hori_line->p1.y + 48});
                point top_R_point = apply_transform({highlight_vert_line->p1.x + 60, highlight_hori_line->p1.y     });

                SDL_Color  red     = {255,0,0,SDL_ALPHA_OPAQUE};
                SDL_Color  green   = {0,255,0,SDL_ALPHA_OPAQUE};
                SDL_Color  blue    = {0,0,255,SDL_ALPHA_OPAQUE};
                SDL_Color  yellow  = {250,250,0,SDL_ALPHA_OPAQUE};
                SDL_Color  none    = {255,255,255,SDL_ALPHA_OPAQUE};
                SDL_FPoint uv      = {0,0}; // unused
                SDL_FPoint top_L   = {(float) top_L_point.x, (float) top_L_point.y};
                SDL_FPoint btm_L   = {(float) btm_L_point.x, (float) btm_L_point.y};
                SDL_FPoint btm_R   = {(float) btm_R_point.x, (float) btm_R_point.y};
                SDL_FPoint top_R   = {(float) top_R_point.x, (float) top_R_point.y};
                SDL_Vertex verts[8] = {
                                        {top_L,none, {0,0}}, // first triangle
                                        {btm_L,none, {0,1}},
                                        {btm_R,none, {1,1}},
                                        {top_R,none, {1,0}}, // second triangle
                                        {top_L,none, {0,0}},
                                        {btm_R,none, {1,1}},
                                      };
                SDL_RenderGeometry(renderer, texture, verts, 6, NULL, 0);
            }

            SDL_RenderPresent(renderer);
        } /* end render */
    }
}
