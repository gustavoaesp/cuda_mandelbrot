#include <stdlib.h>
#include <stdio.h>

#include "mandelbrot.h"
#include "cuda_mandelbrot.h"

#include "common.h"

static void mandelbrot_iteration(float *r_out, float *i_out, float z_r, float z_i, float c_r, float c_i)
{
    *r_out = z_r*z_r - z_i*z_i;
    *i_out = z_i*z_r + z_r*z_i;

    *r_out += c_r;
    *i_out += c_i;
}

static void mandelbrot_pixel(const struct mandelbrot_state *s, float r, float i, uint32_t *dest)
{
    float z_r = 0;
    float z_i = 0;
    for (int iter = 0; iter < s->max_iters; ++iter) {
        mandelbrot_iteration(&z_r, &z_i, z_r, z_i, r, i);
        if (z_r*z_r + z_i*z_i > 4.0f) {
            *dest = s->color_list[iter % k_num_colors];
            return;
        }
    }

    *dest = 0;
}

static void cpu_mandelbrot(struct mandelbrot_state *s, int w, int h)
{
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            /* convert from pixel(x, y) to complex plane(r, i) */
            float r = s->center_x + ((float)x - (float)w / 2) * s->zoom / w;
            float i = s->center_y - ((float)y - (float)h / 2) * s->zoom * s->aspect / w;

            mandelbrot_pixel(s, r, i, s->output + __array_idx(w, x, y) );
        }
    }
}

struct mandelbrot_state *create_mandelbrot(int w, int h, uint32_t max_iters)
{
    struct mandelbrot_state *state = malloc(sizeof(struct mandelbrot_state));
    state->zoom = 4.0f;
    state->aspect = (float)w / h;
    state->center_x = 0;
    state->center_y = 0;
    state->max_iters = max_iters;
    state->color_list = malloc (sizeof(uint32_t) * k_num_colors);
    state->output = malloc(sizeof(uint32_t) * w * h);

    /* secret seed :) */
    srand(0x55941197);

    for (int i = 0; i < k_num_colors; ++i) {
        /* maybe this is just luck but for some reason calling rand() directly
        * into the color uint32_t has not-so nice results */
        state->color_list[i] =
            (rand() & 0xff << 24) |
            (rand() & 0xff << 16) |
            (rand() & 0xff << 8) |
            0xff;
    }

    return state;
}

void mandelbrot_step(struct mandelbrot_state *s, int w, int h)
{
    if (s->accel) {
        s->accel->ops.step(s, w , h);
        return;
    }

    cpu_mandelbrot(s, w, h);
}

void mandelbrot_destroy(struct mandelbrot_state *s)
{
    if (s->accel) {
        s->accel->ops.destroy(s);
    }

    free(s->color_list);
    free(s->output);

    s->color_list = NULL;
    s->output = NULL;

    free(s);
}