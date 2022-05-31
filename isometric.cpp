#include <stdio.h>

#include <SDL.h>
#include <SDL_events.h>
#include <SDL_render.h>

#include <algorithm> // for std::min/max TODO use own...

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

SDL_FPoint apply_transform(SDL_FPoint p) // apply a shear transform
{
    SDL_FPoint sheared_p = {0,0};
    float transform[9] = {      1, shear_x, -camera_pos_x,
                          shear_y,       1, -camera_pos_y,
                                0,       0, 1};
    sheared_p.x = p.x * transform[0] + p.y * transform[1] + 1 * transform[2];
    sheared_p.y = p.x * transform[3] + p.y * transform[4] + 1 * transform[5];
    return sheared_p;
};

SDL_FPoint apply_inverse(SDL_FPoint p) // remove shear
{
    SDL_FPoint unsheared_p = {0,0};
    float inverse[9]  = {(1.f / (1.f - (shear_x * shear_y))), shear_x/(shear_x*shear_y - 1.f),  (camera_pos_x-(camera_pos_y*shear_x))/(1.f - shear_x*shear_y),
                             shear_y/(shear_x*shear_y - 1.f),     1.f/(1.f - shear_x*shear_y),  (camera_pos_y-(shear_y*camera_pos_x))/(1.f - shear_x*shear_y),
                                                           0,                               0,  1};
    unsheared_p.x = p.x * inverse[0] + p.y * inverse[1] + 1 * inverse[2];
    unsheared_p.y = p.x * inverse[3] + p.y * inverse[4] + 1 * inverse[5];
    return unsheared_p;
};
SDL_FPoint apply_inverse_no_translation(SDL_FPoint p) // remove shear
{
    SDL_FPoint unsheared_p = {0,0};
    float inverse[9]  = {(1.f / (1.f - (shear_x * shear_y))), shear_x/(shear_x*shear_y - 1.f),  0,
                             shear_y/(shear_x*shear_y - 1.f),     1.f/(1.f - shear_x*shear_y),  0,
                                                           0,                               0,  1};
    unsheared_p.x = p.x * inverse[0] + p.y * inverse[1] + 1 * inverse[2];
    unsheared_p.y = p.x * inverse[3] + p.y * inverse[4] + 1 * inverse[5];
    return unsheared_p;
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
#define CAMERA_WIDTH        SCREEN_WIDTH  // TODO use own width
#define CAMERA_HEIGHT       SCREEN_HEIGHT // TODO use own height
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

    SDL_RendererInfo info; SDL_version vers;
    SDL_GetRenderDriverInfo(0, &info); SDL_GetVersion(&vers);
    printf("SDL %u.%u.%u, renderer: %s\n", vers.major, vers.minor, vers.patch, info.name); // 2.0.22, opengl

    // load in a texture
    SDL_Surface* bitmap  = SDL_LoadBMP("stone.bmp");
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, bitmap);

#if 1
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
#endif

    SDL_FPoint mouse_pos = {0,0};
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
        float cam_min_x = 0;
        float cam_min_y = 0;
        float cam_max_x = 0;
        float cam_max_y = 0;
        { /* update loop */

            // find out which lines we have to draw for the current camera by clamping
            //cam_min_x = camera_pos_x - ((int) camera_pos_x % SPACING_X_AXIS); // TODO define SPACING_*_AXIS as float
            //cam_min_y = camera_pos_y - ((int) camera_pos_y % SPACING_Y_AXIS);
            //cam_max_x = ((int) (camera_pos_x + CAMERA_WIDTH)) + (SPACING_X_AXIS - (((int) (camera_pos_x + CAMERA_WIDTH)) % SPACING_X_AXIS));
            //cam_max_y = ((int) (camera_pos_y + CAMERA_HEIGHT)) + (SPACING_Y_AXIS - (((int) (camera_pos_y + CAMERA_HEIGHT)) % SPACING_Y_AXIS));

            // new idea:
            // 1. create topleft, topright, btmleft, btmright w/ camera_pos_x/y+w/h (no clamping)
            // 2. apply transform to those points (not inverse)
            SDL_FPoint topleft   = apply_inverse_no_translation(SDL_FPoint {camera_pos_x, camera_pos_y});
            SDL_FPoint topright  = apply_inverse_no_translation(SDL_FPoint {camera_pos_x + CAMERA_WIDTH, camera_pos_y});
            SDL_FPoint btmleft   = apply_inverse_no_translation(SDL_FPoint {camera_pos_x, camera_pos_y + CAMERA_HEIGHT});
            SDL_FPoint btmright  = apply_inverse_no_translation(SDL_FPoint {camera_pos_x + CAMERA_WIDTH, camera_pos_y + CAMERA_HEIGHT});

            // 3. get the min_x/max_x & min_y/max_y from those points
            cam_min_x = std::min(std::min(std::min(topleft.x, topright.x), btmleft.x), btmright.x);
            cam_min_y = std::min(std::min(std::min(topleft.y, topright.y), btmleft.y), btmright.y);
            cam_max_x = std::max(std::max(std::max(topleft.x, topright.x), btmleft.x), btmright.x);
            cam_max_y = std::max(std::max(std::max(topleft.y, topright.y), btmleft.y), btmright.y);

            // 4. clamp those min/max_x/y values like above
            cam_min_x = (int) cam_min_x - ((int) cam_min_x % SPACING_X_AXIS); // TODO define SPACING_*_AXIS as float
            cam_min_y = (int) cam_min_y - ((int) cam_min_y % SPACING_Y_AXIS);
            cam_max_x = (int) cam_max_x + (SPACING_X_AXIS - (((int) (cam_max_x + CAMERA_WIDTH)) % SPACING_X_AXIS));
            cam_max_y = (int) cam_max_y + (SPACING_Y_AXIS - (((int) (cam_max_y + CAMERA_HEIGHT)) % SPACING_Y_AXIS));

            // add one line more to the edges
            cam_min_x -= SPACING_X_AXIS;
            cam_min_y -= SPACING_Y_AXIS;
            cam_max_x += SPACING_X_AXIS;
            cam_max_y += SPACING_Y_AXIS;

            //printf("cam min x: %f, max x: %f, min y: %f, max y: %f\n", cam_min_x, cam_max_x, cam_min_y, cam_max_y);

            // 5. use those for determining which lines to draw


            // find out between which lines the mouse is
            SDL_FPoint grid_mouse = apply_inverse(mouse_pos);
            float vert_line = ((int) grid_mouse.x - ((int) grid_mouse.x % SPACING_X_AXIS));
            float hori_line = ((int) grid_mouse.y - ((int) grid_mouse.y % SPACING_Y_AXIS));
            //printf("mouse x: %f, y: %f\n", vert_line, hori_line);

            #if 0
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
            #endif

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


            #if 0
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
            #endif

            SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
            // TODO new way to draw the lines
            // TODO use floats...
            // TODO infinite lines after shearing
            // TODO better handle edge cases
            SDL_FPoint topleft  = {cam_min_x, cam_min_y};
            SDL_FPoint topright = {cam_max_x, cam_min_y};
            SDL_FPoint btmleft  = {cam_min_x, cam_max_y};
            SDL_FPoint btmright = {cam_max_x, cam_max_y};

            float x_step = cam_min_x;
            for(int i = 0; x_step <= cam_max_x; i++)
            {
                x_step = (cam_min_x + (i * SPACING_X_AXIS));
                SDL_FPoint top  = apply_transform(SDL_FPoint {x_step, cam_min_y}); // TODO change once we remove struct point
                SDL_FPoint btm  = apply_transform(SDL_FPoint {x_step, cam_max_y});
                SDL_RenderDrawLine(renderer, top.x, top.y, btm.x, btm.y);
            }
            float y_step = cam_min_y;
            for(int i = 0; y_step <= cam_max_y; i++)
            {
                y_step = (cam_min_y + (i * SPACING_Y_AXIS));
                SDL_FPoint top  = apply_transform(SDL_FPoint {cam_min_x, y_step}); // TODO change once we remove struct point
                SDL_FPoint btm  = apply_transform(SDL_FPoint {cam_max_x, y_step});
                SDL_RenderDrawLine(renderer, top.x, top.y, btm.x, btm.y);
            }

            #if 0
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
            #endif

            SDL_RenderPresent(renderer);
        } /* end render */
    }
}
