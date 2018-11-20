Kernel: Image Rotation

This is a kernel-type benchmark performing rotation of a .ppm input image by an arbitrary number of degrees.

Installation:

To install the kernel benchmark, navigate to the directory this file is located in, go to any subdirectory, open up a terminal and simply type 'make'. For certain architectures or special compilation options, you might need to change compilation parameters in the makefile.

The preprovided images used as input for the benchmark can be in any folder as long as the full path is provided to the benchmark.

Usage:

You may execute the benchmark by navigating to this directory after compilation and typing

./rot [input file] [output file] [angle]

Input file is the file (if in the same folder as the benchmark) or the path to the file that is to be rotated by the benchmark. Only .ppm binary files are supported.
Output file must be a file name or file path to a .ppm file that will be created/overwritten by the benchmark to contain the rotation output image.
Angle is the angle by which the input image will be rotated.

The specification of the number of threads used depends on the parallel programming model.

Benchmark versions:

Serial
POSIX Threads
OpenMP SuperScalar
