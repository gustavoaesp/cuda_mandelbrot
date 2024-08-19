cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
project(cuda_test LANGUAGES C CUDA)

if(NOT DEFINED CMAKE_CUDA_ARCHITECTURES)
  set(CMAKE_CUDA_ARCHITECTURES 75)
endif()

include_directories(include)

set (MANDELBROT_SRC
    src/mandelbrot.cu
    src/mandelbrot.c
    src/window.c
)

add_executable(mandelbrot src/main.c ${MANDELBROT_SRC})
target_link_libraries(mandelbrot SDL2)
set_target_properties(mandelbrot PROPERTIES CUDA_SEPARABLE_COMPILATION ON)
set_property(TARGET mandelbrot PROPERTY CUDA_SEPARABLE_COMPILATION ON)
