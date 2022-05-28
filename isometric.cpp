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
    line lines[(720/60) + 1]; // TODO hardcoded
    for(int i = 0; i < 12; i++)
    {
        point p1 = {i * 60, 0};
        point p2 = {i * 60, 576};
        lines[i] = {p1, p2};
    }
    lines[12] = {{719, 0}, {719, 576}}; // TODO hardcoded

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

        line* highlight_line = NULL;
        { /* update loop */
            // find out between which lines the mouse is
            // NOTE "bruteforce" version for now
            for(int i = 0; i < 12; i++)
            {
                if (lines[i].p1.x < mouse_pos.x && lines[i+1].p1.x > mouse_pos.x)
                {
                    highlight_line = &lines[i];
                }
            }
        }

        {/* render loop */
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
            SDL_RenderClear(renderer);

            SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
            for(int i = 0; i < 13; i++)
            {
                SDL_RenderDrawLine(renderer, lines[i].p1.x, lines[i].p1.y,
                                             lines[i].p2.x, lines[i].p2.y);
            }

            // TODO hardcoded
            if (highlight_line)
            {
                SDL_Rect highlight_box = { highlight_line->p1.x, highlight_line->p1.y,
                                           60, 576 };
                SDL_RenderFillRect(renderer, &highlight_box);
            }

            SDL_RenderPresent(renderer);
        }
    }
}
