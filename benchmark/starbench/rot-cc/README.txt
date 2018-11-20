Workload: Image Rotation / RGBYUV Color Conversion

This workload is a combination of the two kernel benchmarks image rotation and rgb-to-yuv color conversion (in this order).

Installation:

To install the workload benchmark, navigate to the directory this file is located in, go to any subdirectory, open up a terminal and simply type 'make'. For certain architectures or special compilation options, you might need to change compilation parameters in the makefile.

Usage:

You may execute the benchmark by navigating to this directory after compilation and typing

./rot-cc infile outfile angle

infile is the file or path to file which contains the input image. Only .ppm binary images are supported.
outfile is the file or path to file which will be created/overwritten by the benchmark to contain the final output image.
Angle is the angle by which the rotation kernel will rotate the input image.

The specification of the number of threads used depends on the parallel programming model. The benchmark creates a yuvout.ppm image file which can be used to verify correctness of the second kernel.

Benchmark versions:

Serial
POSIX Threads
OpenMP SuperScalar
