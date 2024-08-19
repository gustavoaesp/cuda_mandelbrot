#include "mandelbrot.h"
#include "cuda_mandelbrot.h"

#include <stdio.h>
#include <stdlib.h>

struct cuda_mandelbrot
{
    uint32_t *v_color_list;
    uint32_t *v_output;
};

__device__
void mb_iteration(float *r_out, float *i_out, float z_r, float z_i, float c_r, float c_i)
{
    *r_out = z_r*z_r - z_i*z_i;
    //*i_out = z_i*z_r + z_r*z_i;
    *i_out = 2*z_r*z_i;

    *r_out += c_r;
    *i_out += c_i;
}

__device__
void mandelbrot_pixel(uint32_t max_iterations, float r, float i, uint32_t *colors, uint32_t *dest)
{
    float z_r = 0;
    float z_i = 0;

    for (int iter = 0; iter < max_iterations; ++iter) {
        mb_iteration(&z_r, &z_i, z_r, z_i, r, i);
        if (z_r * z_r + z_i * z_i > 4.0f) {
            *dest = colors[iter];
            return;
        }
    }

    *dest = 0;
}

__global__
void cu_mandelbrot_kernel(
    int screen_offset_x,
    float zoom,
    float center_x, float center_y,
    float aspect,
    uint32_t max_iterations,
    int w, int h,
    uint32_t *colors,
    uint32_t *output)
{
    /* Given that a maximum of 1024 threads per block can be spawned, we add the screen_offset_x*/
    int x = threadIdx.x + screen_offset_x;
    int y = blockIdx.x;

    /* convert from pixel(x, y) to complex plane(r, i) */
    float r = center_x + ((float)x - (float)w / 2) * zoom / w;
    float i = center_y - ((float)y - (float)h / 2) * zoom * aspect / w;

    mandelbrot_pixel(
        max_iterations,
        r, i,
        colors,
        &output[y * w + x]
    );
}

static void cuda_mandelbrot_step(struct mandelbrot_state *s, int w, int h)
{
    struct cuda_mandelbrot *cuda_ctx = (struct cuda_mandelbrot*)s->accel->__priv;

    for (int off_x = 0; off_x < w; off_x += 1024) {
        int kernel_w = (w - off_x > 1024) ? 1024 : (w - off_x);

        cu_mandelbrot_kernel<<<h, kernel_w>>>(
            off_x,
            s->zoom,
            s->center_x, s->center_y,
            s->aspect, s->max_iters,
            w, h,
            cuda_ctx->v_color_list,
            cuda_ctx->v_output
        );
    }

    cudaMemcpy(
        s->output,
        cuda_ctx->v_output,
        sizeof(uint32_t) * w * h,
        cudaMemcpyDeviceToHost
    );
}

static void cuda_mandelbrot_destroy(struct mandelbrot_state *s)
{
    struct cuda_mandelbrot *cuda_ctx = (struct cuda_mandelbrot *)s->accel->__priv;
    cudaFree(cuda_ctx->v_output);
    cudaFree(cuda_ctx->v_color_list);

    free(cuda_ctx);
    free(s->accel);

    s->accel = NULL;
}

struct accelerator_ops cuda_accel_ops {
    .step = cuda_mandelbrot_step,
    .destroy = cuda_mandelbrot_destroy
};

int cuda_init_mandelbrot(struct mandelbrot_state *s, int w, int h)
{
    struct cuda_mandelbrot *cuda_ctx = (struct cuda_mandelbrot*)malloc(
        sizeof(struct cuda_mandelbrot)
    );
    if (!cuda_ctx) {
        goto err_cuda_ctx;
    }

    cudaMalloc(&cuda_ctx->v_color_list, sizeof(uint32_t) * k_num_colors);
    if (!cuda_ctx->v_color_list) {
        goto err_cuda_v_color_list;
    }
    cudaMalloc(&cuda_ctx->v_output, sizeof(float) * w * h);
    if (!cuda_ctx->v_output) {
        goto err_cuda_v_output;
    }

    cudaMemcpy(
        cuda_ctx->v_color_list,
        s->color_list,
        sizeof(uint32_t) * k_num_colors,
        cudaMemcpyHostToDevice
    );

    s->accel = (struct accelerator*)malloc(sizeof(struct accelerator));
    if (!s->accel) {
        goto err_accelerator;
    }
    s->accel->__priv = cuda_ctx;
    s->accel->ops = cuda_accel_ops;

    return 0;

err_accelerator:
    cudaFree(cuda_ctx->v_output);
err_cuda_v_output:
    cudaFree(cuda_ctx->v_color_list);
err_cuda_v_color_list:
    free(cuda_ctx);
err_cuda_ctx:
    return -1;
}
