#include <getopt.h>
#include <stdio.h>

#include <SDL2/SDL.h>

#include "common.h"
#include "mandelbrot.h"
#include "cuda_mandelbrot.h"
#include "window.h"


static const int k_screen_width = 1600;
static const int k_screen_height = 900;

static uint32_t keymap = 0;

struct configuration
{
    int use_cpu;
    int iterations;
};

enum keymap_values {
    key_move_up = 1,
    key_move_right = (1 << 1),
    key_move_down = (1 << 2),
    key_move_left = (1 << 3),
    key_zoom_in = (1 << 4),
    key_zoom_out = (1 << 5)
};

void sdl_key_events(struct mandelbrot_state *s, SDL_Event *e)
{
    uint32_t pressed = (e->type == SDL_KEYDOWN);

#   define KEYMAP_ASSIGN(pressed, kmap, value) \
        (pressed) ? \
            kmap | (value) : \
            kmap & (~(value))

    switch (e->key.keysym.sym) {
    case SDLK_UP:
        if (e->type == SDL_KEYDOWN) {
            s->max_iters++;
            fprintf(stderr, "iters: %d\n", s->max_iters);
        }
        break;
    case SDLK_DOWN:
        if (e->type == SDL_KEYDOWN) {
            s->max_iters--;
            if (s->max_iters < 1) {
                s->max_iters = 1;
            }
            fprintf(stderr, "iters: %d\n", s->max_iters);
        }
        break;
    case SDLK_w:
        keymap = KEYMAP_ASSIGN(pressed, keymap, key_move_up);
        break;
    case SDLK_d:
        keymap = KEYMAP_ASSIGN(pressed, keymap, key_move_right);
        break;
    case SDLK_s:
        keymap = KEYMAP_ASSIGN(pressed, keymap, key_move_down);
        break;
    case SDLK_a:
        keymap = KEYMAP_ASSIGN(pressed, keymap, key_move_left);
        break;
    case SDLK_i:
        keymap = KEYMAP_ASSIGN(pressed, keymap, key_zoom_in);
        break;
    case SDLK_k:
        keymap = KEYMAP_ASSIGN(pressed, keymap, key_zoom_out);
        break;
    }
}

static void print_help()
{
    printf("Usage: mandelbrot [OPTION]\n");
    printf("     --cpu\tUse the cpu instead of any acceleration\n");
    printf(" -i       \tMax number of iterations per pixel (default 512)\n");
}

static int parse_options(struct configuration *config, int argc, char **argv)
{
    int help = 0;
    struct option long_options[] = {
        {"cpu", no_argument, &config->use_cpu, 1},
        {"help", no_argument, &help, 1}
    };
    int c;

    while(1) {
        int option_index = 0;
        c = getopt_long(argc, argv, "i:h", long_options, &option_index);
        if (c == -1) {
            break;
        }
        switch(c) {
        case 0:
            break;
        case 'h':
            help = 1;
            break;
        case 'i':
            char *end;
            config->iterations = strtol(optarg, &end, 10);
            if (!config->iterations) {
                help = 1;
            }
            break;
        case '?':
            help = 1;
            break;
        }
    }

    if (help) {
        print_help();
        return -1;
    }

    return 0;
}

int main(int argc, char **argv)
{
    int ret;
    uint32_t running = 1;
    struct sdl_window *window;
    struct mandelbrot_state *mandelbrot_state;
    struct configuration config = {
        .use_cpu = 0,
        .iterations = 512
    };

    if(ret = parse_options(&config, argc, argv)) {
        return ret;
    }
    
    if (!(mandelbrot_state = create_mandelbrot(k_screen_width, k_screen_height, config.iterations))) {
        fprintf (stderr, "Error creating a mandelbrot state object\n");
        return -1;
    }

    /* TODO if I *ever* implement another way to accelerate this, make it an
     * argv (getopt) parameter.
     */
    if (!config.use_cpu) {
        if (ret = cuda_init_mandelbrot(mandelbrot_state, k_screen_width, k_screen_height)) {
            fprintf(stderr, "Error initializing the cuda mandelbrot context (%d)\n", ret);
            return ret;
        }
    }

    if (ret = SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Could not initialize SDL (%d)\n", ret);
        return -1;
    }

    window = window_create(
        k_screen_width,
        k_screen_height,
        (config.use_cpu) ? "Mandelbrot [CPU]" : "Mandelbrot [CUDA]"
    );

    while (running) {
        float mov_speed = mandelbrot_state->zoom / 32.0f;
        SDL_Event e;

        while (SDL_PollEvent(&e) > 0) {
            switch(e.type) {
            case SDL_QUIT:
                running = 0;
                break;
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                sdl_key_events(mandelbrot_state, &e);
                break;
            }
        }

        if (keymap & key_move_up) {
            mandelbrot_state->center_y += mov_speed;
        } else if (keymap & key_move_down) {
            mandelbrot_state->center_y -= mov_speed;
        }

        if (keymap & key_move_right) {
            mandelbrot_state->center_x += mov_speed;
        } else if (keymap & key_move_left) {
            mandelbrot_state->center_x -= mov_speed;
        }

        if (keymap & key_zoom_in) {
            mandelbrot_state->zoom -= mandelbrot_state->zoom / 10.0f;
        } else if (keymap & key_zoom_out) {
            mandelbrot_state->zoom += mandelbrot_state->zoom / 10.0f;
        }

        window_clear(window);

        mandelbrot_step(mandelbrot_state, k_screen_width, k_screen_height);

        /* TODO find a way to do this more efficiently, there should be a way to blit the
        *   output buffer more directly to the screen in SDL.
        */
        for (int y = 0; y < k_screen_height; ++y) {
            for (int x = 0; x < k_screen_width; ++x) {
                window_put_pixel_color(
                    window,
                    x,y,
                    mandelbrot_state->output[__array_idx(k_screen_width, x, y)]
                );
            }
        }

        window_present(window);
    }

    mandelbrot_destroy(mandelbrot_state);
    window_destroy(window);
    SDL_Quit();
    return 0;
}