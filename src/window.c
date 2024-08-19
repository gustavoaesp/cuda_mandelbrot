#include "window.h"

struct sdl_window
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    int w, h;
};

struct sdl_window *window_create(int w, int h, const char *name)
{
    struct sdl_window *out = malloc(sizeof(struct sdl_window));
    if (!out) {
        return NULL;
    }
    out->w = w;
    out->h = h;

    out->window = SDL_CreateWindow(

        name, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        w, h, 0
    );
    if (!out->window) {
        goto err_window;
    }
    out->renderer = SDL_CreateRenderer(out->window, -1, SDL_RENDERER_PRESENTVSYNC);
    if (!out->renderer) {
        goto err_renderer;
    }

    return out;

err_renderer:
    SDL_DestroyWindow(out->window);
    out->window = NULL;
err_window:
    free(out);
    return NULL;
}

void window_clear(struct sdl_window *window)
{
    SDL_SetRenderDrawColor(window->renderer, 0, 0, 0, 0);
    SDL_RenderClear(window->renderer);
}

void window_put_pixel_color(struct sdl_window *window, int x, int y, uint32_t color)
{
    if (x < 0 || x >= window->w || y < 0 || y > window->h) {
        return;
    }
    SDL_SetRenderDrawColor(
        window->renderer,
        (uint8_t)(color >> 24),
        (uint8_t)(color >> 16),
        (uint8_t)(color >> 8),
        0xff
    );
    SDL_RenderDrawPoint(window->renderer, x, y);
}

void window_present(struct sdl_window *window)
{
    SDL_RenderPresent(window->renderer);
}

void window_destroy(struct sdl_window *window)
{
    if (window->renderer) {
        SDL_DestroyRenderer(window->renderer);
        window->renderer = NULL;
    }
    if (window->window) {
        SDL_DestroyWindow(window->window);
        window->window = NULL;
    }

    free(window);
}