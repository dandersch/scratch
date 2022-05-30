#include <stdio.h>

#include <SDL.h>
#include <SDL_events.h>
#include <SDL_render.h>

struct point { int x;    int y;};
struct line  { point p1; point p2; };

static float shear_x = 1;
static float shear_y = 0;

// NOTE for now camera w/h == screen w/h
// TODO use a rect with a width & height
static float camera_pos_x = 0;
static float camera_pos_y = 0;

point apply_transform(point p) // apply a shear transform
{
    point sheared_p = {0,0};
    float transform[9] = {      1, shear_x, -camera_pos_x,
                          shear_y,       1, -camera_pos_y,
                                0,       0, 1};
    sheared_p.x = p.x * transform[0] + p.y * transform[1] + 1 * transform[2];
    sheared_p.y = p.x * transform[3] + p.y * transform[4] + 1 * transform[5];
    return sheared_p;
};

point apply_inverse(point p) // remove shear
{
    point unsheared_p = {0,0};
    float inverse[9]  = {(1.f / (1.f - (shear_x * shear_y))), shear_x/(shear_x*shear_y - 1.f),  (-camera_pos_x-(-camera_pos_y*shear_x))/(shear_x*shear_y - 1),
                             shear_y/(shear_x*shear_y - 1.f),     1.f/(1.f - shear_x*shear_y),  (-camera_pos_y-(shear_y*-camera_pos_x))/(shear_x*shear_y - 1),
                                                           0,                               0,  1};
    unsheared_p.x = p.x * inverse[0] + p.y * inverse[1] + 1 * inverse[2];
    unsheared_p.y = p.x * inverse[3] + p.y * inverse[4] + 1 * inverse[5];
    return unsheared_p;
};

#define SCREEN_WIDTH        1200
#define SCREEN_HEIGHT       720
#define SPACING_X_AXIS      60
#define SPACING_Y_AXIS      24
#define VERTICAL_LINE_CNT   SCREEN_WIDTH/SPACING_X_AXIS
#define HORIZONTAL_LINE_CNT SCREEN_HEIGHT/SPACING_Y_AXIS

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

    // set up points to draw lines.
    line vert_lines[VERTICAL_LINE_CNT   + 1];  // +1 because we set
    line hori_lines[HORIZONTAL_LINE_CNT + 1];  // another one manually
    for(int i = 0; i < VERTICAL_LINE_CNT; i++)
    {
        point p1 = {i * SPACING_X_AXIS,             0};
        point p2 = {i * SPACING_X_AXIS, SCREEN_HEIGHT};
        vert_lines[i] = {p1, p2};
    }
    for(int i = 0; i < HORIZONTAL_LINE_CNT; i++)
    {
        point p1 = {           0, i * SPACING_Y_AXIS};
        point p2 = {SCREEN_WIDTH, i * SPACING_Y_AXIS};
        hori_lines[i] = {p1, p2};
    }

    vert_lines[VERTICAL_LINE_CNT]   = {{SCREEN_WIDTH,             0}, {SCREEN_WIDTH, SCREEN_HEIGHT}};
    hori_lines[HORIZONTAL_LINE_CNT] = {{           0, SCREEN_HEIGHT}, {SCREEN_WIDTH, SCREEN_HEIGHT}};

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
                    if (event.key.keysym.sym == SDLK_s) { shear_y -= 0.1f; printf("shear_y: %f\n", shear_y); }
                    if (event.key.keysym.sym == SDLK_d) { shear_y += 0.1f; printf("shear_y: %f\n", shear_y); }

                    if (event.key.keysym.sym == SDLK_UP)    { camera_pos_y -= 3.0f; printf("cam_x: %f\n", camera_pos_x);}
                    if (event.key.keysym.sym == SDLK_DOWN)  { camera_pos_y += 3.0f; printf("cam_x: %f\n", camera_pos_x);}
                    if (event.key.keysym.sym == SDLK_LEFT)  { camera_pos_x -= 3.0f; printf("cam_y: %f\n", camera_pos_y);}
                    if (event.key.keysym.sym == SDLK_RIGHT) { camera_pos_x += 3.0f; printf("cam_y: %f\n", camera_pos_y);}
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
            int vert_idx = (grid_mouse.x - (grid_mouse.x % SPACING_X_AXIS)) / SPACING_X_AXIS;
            int hori_idx = (grid_mouse.y - (grid_mouse.y % SPACING_Y_AXIS)) / SPACING_Y_AXIS;
            if (vert_idx >= 0 && vert_idx < VERTICAL_LINE_CNT) {
                highlight_vert_line = &vert_lines[vert_idx];
            }
            if (hori_idx >= 0 && hori_idx < HORIZONTAL_LINE_CNT) {
                highlight_hori_line = &hori_lines[hori_idx];
            }

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
            for(int i = 0; i < (VERTICAL_LINE_CNT+1); i++)
            {
                if (highlight_vert_line == &vert_lines[i] ||
                    highlight_vert_line == &vert_lines[i-1]) // TODO is this too much voodoo?
                {
                    SDL_SetRenderDrawColor(renderer, 0, 255, 0, SDL_ALPHA_OPAQUE);
                }
                else { SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE); }

                // apply a shear transform to the lines before rendering
                SDL_RenderDrawLine(renderer, apply_transform(vert_lines[i].p1).x,
                                             apply_transform(vert_lines[i].p1).y,
                                             apply_transform(vert_lines[i].p2).x,
                                             apply_transform(vert_lines[i].p2).y);
            }
            for(int i = 0; i < (HORIZONTAL_LINE_CNT+1); i++)
            {
                if (highlight_hori_line == &hori_lines[i] ||
                    highlight_hori_line == &hori_lines[i-1]) // TODO is this too much voodoo?
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
                point top_L_point = apply_transform({highlight_vert_line->p1.x,
                                                     highlight_hori_line->p1.y});
                point btm_L_point = apply_transform({highlight_vert_line->p1.x,
                                                     highlight_hori_line->p1.y + SPACING_Y_AXIS});
                point btm_R_point = apply_transform({highlight_vert_line->p1.x + SPACING_X_AXIS,
                                                     highlight_hori_line->p1.y + SPACING_Y_AXIS});
                point top_R_point = apply_transform({highlight_vert_line->p1.x + SPACING_X_AXIS,
                                                     highlight_hori_line->p1.y});

                SDL_Color  red     = {255,0,0,SDL_ALPHA_OPAQUE};
                SDL_Color  green   = {0,255,0,SDL_ALPHA_OPAQUE};
                SDL_Color  blue    = {0,0,255,SDL_ALPHA_OPAQUE};
                SDL_Color  yellow  = {250,250,0,SDL_ALPHA_OPAQUE};
                SDL_Color  none    = {255,255,255,SDL_ALPHA_OPAQUE};
                SDL_FPoint top_L   = {(float) top_L_point.x, (float) top_L_point.y};
                SDL_FPoint btm_L   = {(float) btm_L_point.x, (float) btm_L_point.y};
                SDL_FPoint btm_R   = {(float) btm_R_point.x, (float) btm_R_point.y};
                SDL_FPoint top_R   = {(float) top_R_point.x, (float) top_R_point.y};
                SDL_Vertex verts[8] = {
                                      /* vtx    col   uv   */
                                        {top_L, none, {0,0}}, // first triangle
                                        {btm_L, none, {0,1}},
                                        {btm_R, none, {1,1}},
                                        {top_R, none, {1,0}}, // second triangle
                                        {top_L, none, {0,0}},
                                        {btm_R, none, {1,1}},
                                      };
                SDL_RenderGeometry(renderer, texture, verts, 6, NULL, 0);
            }

            SDL_RenderPresent(renderer);
        } /* end render */
    }
}
