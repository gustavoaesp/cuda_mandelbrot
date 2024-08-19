# CUDA mandelbrot set implementation
This is a very small 'real-time' CUDA implementation of the mandelbrot set using C.

## Build
Building should be as simple as
```bash
mkdir build && cd build
cmake ..
make
./mandelbrot
```
As long as the required dependencies are installed in the system (CUDA libraries and SDL2).

## Controls
WASD - move
I - zoom in;
K - zoom out;
Arrow up - Increase max mandelbrot iterations
Arrow down - Decrease max mandelbrot iterations
