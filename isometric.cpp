#include <stdio.h>

#include <SDL.h>
#include <SDL_events.h>
#include <SDL_render.h>

#include <algorithm> // for std::min/max TODO use own...

static float shear_x = 0.6f; // NOTE: we get a dent in the unsheared grid when starting with >= 0.7f...
static float shear_y = 0;    // TODO have transform matrix and take pointers to its elements instead of this

// NOTE for now camera w/h == screen w/h
// TODO use a rect with a width & height
static float camera_pos_x = 0;
static float camera_pos_y = 0;

//static float transform[9] = {1, 0.6, 0,
//                             0,   1, 0,
//                             0,   0, 1};
//static float* shear_x      = &transform[1];
//static float* shear_y      = &transform[3];
//static float* camera_pos_x = &transform[2];
//static float* camera_pos_y = &transform[5];

SDL_FPoint apply_transform(SDL_FPoint p) // apply a shear transform and the camera translation
{
    SDL_FPoint sheared_p = {0,0};
    float transform[9] = {      1, shear_x, -camera_pos_x,
                          shear_y,       1, -camera_pos_y,
                                0,       0, 1};
    sheared_p.x = p.x * transform[0] + p.y * transform[1] + 1 * transform[2];
    sheared_p.y = p.x * transform[3] + p.y * transform[4] + 1 * transform[5];
    return sheared_p;
};

SDL_FPoint apply_inverse(SDL_FPoint p, bool translate = true) // remove shear
{
    SDL_FPoint unsheared_p = {0,0};
    float inverse[9]  = {(1.f / (1.f - (shear_x * shear_y))), shear_x/(shear_x*shear_y - 1.f),  (camera_pos_x-(camera_pos_y*shear_x))/(1.f - shear_x*shear_y),
                             shear_y/(shear_x*shear_y - 1.f),     1.f/(1.f - shear_x*shear_y),  (camera_pos_y-(shear_y*camera_pos_x))/(1.f - shear_x*shear_y),
                                                           0,                               0,  1};
    unsheared_p.x = p.x * inverse[0] + p.y * inverse[1];
    unsheared_p.y = p.x * inverse[3] + p.y * inverse[4];

    // apply translation separately
    if (translate)
    {
        unsheared_p.x += 1 * inverse[2];
        unsheared_p.y += 1 * inverse[5];
    }

    return unsheared_p;
};

#define SCREEN_WIDTH        1200
#define SCREEN_HEIGHT       720
#define CAMERA_WIDTH        SCREEN_WIDTH  // TODO use own width
#define CAMERA_HEIGHT       SCREEN_HEIGHT // TODO use own height
#define SPACING_X_AXIS      (60 * 1)
#define SPACING_Y_AXIS      (24 * 1)

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

    SDL_FPoint mouse_pos = {0,0};
    bool is_running = true;
    bool do_lerp = false;
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
                    if (event.key.keysym.sym == SDLK_l) { do_lerp = !do_lerp; }
                    if (event.key.keysym.sym == SDLK_x) { shear_x -= 0.1f; }
                    if (event.key.keysym.sym == SDLK_c) { shear_x += 0.1f; }
                    if (event.key.keysym.sym == SDLK_s) { shear_y -= 0.1f; }
                    if (event.key.keysym.sym == SDLK_d) { shear_y += 0.1f; }

                    if (event.key.keysym.sym == SDLK_UP)
                    {
                        SDL_FPoint up_vec = apply_inverse({ 0,-1}, false);
                        camera_pos_x -= 3 * up_vec.x; // TODO what's going with these signs
                        camera_pos_y += 3 * up_vec.y;
                    }
                    if (event.key.keysym.sym == SDLK_DOWN)
                    {
                        SDL_FPoint down_vec = apply_inverse({ 0, 1}, false);
                        camera_pos_x -= 3 * down_vec.x;
                        camera_pos_y += 3 * down_vec.y;
                    }
                    if (event.key.keysym.sym == SDLK_LEFT)
                    {
                        SDL_FPoint left_vec = apply_inverse({-1, 0}, false);
                        camera_pos_x += 3 * left_vec.x;
                        camera_pos_y -= 3 * left_vec.y;
                    }
                    if (event.key.keysym.sym == SDLK_RIGHT)
                    {
                        SDL_FPoint right_vec = apply_inverse({-1, 0}, false);
                        camera_pos_x -= 3 * right_vec.x;
                        camera_pos_y += 3 * right_vec.y;
                    }
                }


                if (event.type == SDL_MOUSEMOTION)
                {
                    mouse_pos.x = event.motion.x;
                    mouse_pos.y = event.motion.y;
                }
            }
        } /* end event */


        float vert_line = 0; // TODO set to some float that we consider invalid
        float hori_line = 0; // TODO set to some float that we consider invalid
        float cam_min_x = 0;
        float cam_min_y = 0;
        float cam_max_x = 0;
        float cam_max_y = 0;
        { /* update loop */

            // find out which lines we have to draw for the current camera:
            // 1. create topleft, topright, btmleft, btmright w/ camera_pos_x/y+w/h (no clamping)
            // 2. apply inverse to those points without the camera translation
            SDL_FPoint topleft   = apply_inverse({camera_pos_x, camera_pos_y},                                false);
            SDL_FPoint topright  = apply_inverse({camera_pos_x + CAMERA_WIDTH, camera_pos_y},                 false);
            SDL_FPoint btmleft   = apply_inverse({camera_pos_x, camera_pos_y + CAMERA_HEIGHT},                false);
            SDL_FPoint btmright  = apply_inverse({camera_pos_x + CAMERA_WIDTH, camera_pos_y + CAMERA_HEIGHT}, false);

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

            // add one line more to the top and left edge
            cam_min_x -= SPACING_X_AXIS;
            cam_min_y -= SPACING_Y_AXIS;

            // 5. use those for determining which lines to draw
            //printf("cam min x: %f, max x: %f, min y: %f, max y: %f\n", cam_min_x, cam_max_x, cam_min_y, cam_max_y);

            // find out between which lines the mouse is
            // TODO behaves wrong for negative grid_mouse.x/y
            SDL_FPoint grid_mouse = apply_inverse(mouse_pos, true);
            vert_line = (int) (grid_mouse.x - ((int) grid_mouse.x % SPACING_X_AXIS));
            hori_line = (int) (grid_mouse.y - ((int) grid_mouse.y % SPACING_Y_AXIS));
            //if (grid_mouse.x < 0) {}
            //if (grid_mouse.y < 0) {}
            //printf("mouse x: %f, y: %f\n", vert_line, hori_line);

            // do a lerp for some nice looking animation
            if (do_lerp)
            {
                float time_to_lerp       = 5000.f; // TODO use a timestep in the future
                static float timer       = 0;
                static bool  lerp_to_max = true;
                timer += 2.f;
                float interpolant = timer/time_to_lerp;
                float min = -0.2f; float max =  0.2f;
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
            }
        } /* end update */

        { /* render loop */
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
            SDL_RenderClear(renderer);

            SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE); // TODO green for highlighted lines

            { /* DRAW TEXTURES ON THE TILE GRID */
                for(float i = 0, x_step = cam_min_x; x_step < (cam_max_x - SPACING_X_AXIS); i++)
                  for(float j = 0, y_step = cam_min_y; y_step < (cam_max_y - SPACING_Y_AXIS); j++)
                {
                    x_step = (cam_min_x + (i * SPACING_X_AXIS));
                    y_step = (cam_min_y + (j * SPACING_Y_AXIS));

                    SDL_FPoint top_L = apply_transform({x_step, y_step});
                    SDL_FPoint btm_L = apply_transform({x_step, y_step + SPACING_Y_AXIS});
                    SDL_FPoint btm_R = apply_transform({x_step + SPACING_X_AXIS, y_step + SPACING_Y_AXIS});
                    SDL_FPoint top_R = apply_transform({x_step + SPACING_X_AXIS, y_step});
                    SDL_Color  none  = {255,255,255,SDL_ALPHA_OPAQUE};

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

                // TODO find a way to "if (..)" here...
                SDL_FPoint top_L = apply_transform({vert_line, hori_line});
                SDL_FPoint btm_L = apply_transform({vert_line, hori_line + SPACING_Y_AXIS});
                SDL_FPoint btm_R = apply_transform({vert_line + SPACING_X_AXIS, hori_line + SPACING_Y_AXIS});
                SDL_FPoint top_R = apply_transform({vert_line + SPACING_X_AXIS, hori_line});

                SDL_Color  red     = {120,0,0,100};
                SDL_Color  green   = {0,255,0,SDL_ALPHA_OPAQUE};
                SDL_Color  blue    = {0,0,255,SDL_ALPHA_OPAQUE};
                SDL_Color  yellow  = {250,250,0,SDL_ALPHA_OPAQUE};
                //SDL_Color  none    = {255,255,255,SDL_ALPHA_OPAQUE};
                SDL_Color  none    = {40,255,40,240}; // TODO can't draw transparent color over texture...
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

            // new way to draw the lines
            // NOTE float i for multiple var declaration inside for loop
            for(float i = 0, x_step = cam_min_x; x_step < cam_max_x; i++)
            {
                x_step = (cam_min_x + (i * SPACING_X_AXIS));
                SDL_FPoint top  = apply_transform({x_step, cam_min_y});
                SDL_FPoint btm  = apply_transform({x_step, cam_max_y});
                SDL_RenderDrawLine(renderer, top.x, top.y, btm.x, btm.y);
            }
            for(float i = 0, y_step = cam_min_y; y_step < cam_max_y; i++)
            {
                y_step = (cam_min_y + (i * SPACING_Y_AXIS));
                SDL_FPoint top  = apply_transform({cam_min_x, y_step});
                SDL_FPoint btm  = apply_transform({cam_max_x, y_step});
                SDL_RenderDrawLine(renderer, top.x, top.y, btm.x, btm.y);
            }


            SDL_RenderPresent(renderer);
        } /* end render */
    }
}
