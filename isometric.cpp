#include <stdio.h>

#include <SDL.h>
#include <SDL_events.h>
#include <SDL_render.h>

struct point { int x;    int y;};
struct line  { point p1; point p2; };

int main(int argc, char** argv)
{
    // init sdl
    SDL_Init(SDL_INIT_EVERYTHING);

    // open window, init renderer
    SDL_Window*   window;
    SDL_Renderer* renderer;
    SDL_CreateWindowAndRenderer(720, 576, 0, &window, &renderer); // 720x576

    // set up points to draw lines. 720/12 = 60, i.e. every 60 pixels two new
    // points: one point up (0), one point down (576)
    line vert_lines[(720/60) + 1]; // TODO hardcoded
    line hori_lines[(576/48) + 1]; // TODO hardcoded
    for(int i = 0; i < 12; i++)
    {
        point p1 = {i * 60, 0};
        point p2 = {i * 60, 576};
        vert_lines[i] = {p1, p2};

        p1 = {0,   i * 48};
        p2 = {720, i * 48};
        hori_lines[i] = {p1, p2};
    }
    vert_lines[12] = {{719,   0}, {719, 576}}; // TODO hardcoded
    hori_lines[12] = {{0,   575}, {720, 575}}; // TODO hardcoded

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

                if (event.type == SDL_MOUSEMOTION)
                {
                    mouse_pos.x = event.motion.x;
                    mouse_pos.y = event.motion.y;
                }
            }
        }

        // TODO probably better to not use pointers
        line* highlight_vert_line = NULL;
        line* highlight_hori_line = NULL;
        { /* update loop */
            // find out between which lines the mouse is (NOTE "bruteforce" version for now)
            for(int i = 0; i < 12; i++)
            {
                if (vert_lines[i].p1.x < mouse_pos.x && vert_lines[i+1].p1.x > mouse_pos.x)
                {
                    highlight_vert_line = &vert_lines[i];
                }
                if (hori_lines[i].p1.y < mouse_pos.y && hori_lines[i+1].p1.y > mouse_pos.y)
                {
                    highlight_hori_line = &hori_lines[i];
                }
            }
        }

        {/* render loop */
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
            SDL_RenderClear(renderer);

            SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
            for(int i = 0; i < 13; i++)
            {
                SDL_RenderDrawLine(renderer, vert_lines[i].p1.x, vert_lines[i].p1.y,
                                             vert_lines[i].p2.x, vert_lines[i].p2.y);
                SDL_RenderDrawLine(renderer, hori_lines[i].p1.x, hori_lines[i].p1.y,
                                             hori_lines[i].p2.x, hori_lines[i].p2.y);
            }

            // TODO hardcoded
            if (highlight_vert_line && highlight_hori_line)
            {
                SDL_Rect highlight_box = { highlight_vert_line->p1.x,
                                           highlight_hori_line->p1.y,
                                           60, 48 };
                SDL_RenderFillRect(renderer, &highlight_box);
            }

            SDL_RenderPresent(renderer);
        }
    }
}
