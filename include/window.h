#ifndef _WINDOW_H_
#define _WINDOW_H_

#include <SDL2/SDL.h>

struct sdl_window *window_create(int w, int h, const char *name);
void window_clear(struct sdl_window*);
void window_put_pixel_color(struct sdl_window *, int x, int y, uint32_t color);
void window_present(struct sdl_window*);
void window_destroy(struct sdl_window*);

#endif