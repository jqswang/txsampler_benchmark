Kernel: RGB to YUV Color Conversion

This is a kernel-type benchmark doing color conversion of an input .ppm RGB image to the YUV color space.

Installation:

To install the kernel benchmark, navigate to the directory this file is located in, go to any of the subdirectories, open up a terminal and simply type 'make'. For certain architectures or special compilation options, you might need to change compilation parameters in the makefile.

The preprovided images used as input for the benchmark can be in any folder as long as the full path to them is provided to the program.

Usage:

You may execute the benchmark by navigating to this directory after compilation and typing

./rgbyuv -h

The -h option will display a help text explaining how to pass parameters to the benchmark. This benchmark produces as output images the YUV image and three separate images for the single color components. These images can be used to verify correctness of the output.

The specification of the number of threads used depends on the parallel programming model.

Benchmark versions:

Serial
POSIX Threads
OpenMP SuperScalar
