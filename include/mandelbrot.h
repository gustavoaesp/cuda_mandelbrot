#ifndef _MANDELBROT_H_
#define _MANDELBROT_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

static const int k_num_colors = 512;

struct mandelbrot_state
{
    /*
     *  Keeps track of what length the width of the screen has on the complex plane,
     *  so as 'zoom' gets bigger, the *actual* perceived zoom in the image is reduced.
     */
    float zoom;
    float aspect;
    float center_x, center_y;

    /* max number of iterations per pixel of f(Zn) */
    uint32_t max_iters;
    /* this is the color list for the iteration escape */
    uint32_t *color_list;
    /*
    *   the output 'framebuffer', this is a uint32_t color buffer that gets rewritten
    *   every step.
    */
    uint32_t *output;

    /* If any acceleration is used, this pointer will not be NULL. */
    struct accelerator *accel;
};

struct accelerator_ops 
{
    void (*step)(struct mandelbrot_state *, int w, int h);
    void (*destroy)(struct mandelbrot_state *);
};

struct accelerator
{
    struct accelerator_ops ops;
    /* pointer to a library/implementation-specific struct */
    void *__priv;
};

/*
*   Creates and initializes a 'mandelbrot_state' object, this object will live
*   through the lifetime of the application.
*/
struct mandelbrot_state *create_mandelbrot(int w, int h, uint32_t max_iters);

/*
*   Called each frame to compute the image based on the position and zoom.
*/
void mandelbrot_step(struct mandelbrot_state *, int w, int h);

/*
*   Self-explanatory.
*/
void mandelbrot_destroy(struct mandelbrot_state *);

#ifdef __cplusplus
}
#endif

#endif