#ifndef _CUDA_MANDELBROT_H_
#define _CUDA_MANDELBROT_H_

/*
*   Looks like nvcc will assume C++
*/
#ifdef __cplusplus
extern "C" {
#endif

/*
*   To be called after successfully initializing a mandelbrot state,
*   this function will initialize the 'accel' pointer in the state.
*/
int cuda_init_mandelbrot(struct mandelbrot_state *, int w, int h);

#ifdef __cplusplus
}
#endif

#endif