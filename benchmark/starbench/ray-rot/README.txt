Workload: ray-rot

This is a workload-type benchmark measuring the time it takes to render an image using ray-tracing from a scene description file and rotate it by some degrees afterwards.

Installation:

To install the benchmark, navigate to the directory this file is located in, go to any subdirectory, open up a terminal and simply type 'make'. For certain architectures 
or special compilation options, you might need to change compilation parameters in the makefile.

Usage:

You may execute the benchmark by navigating to this directory after compilation and typing

./ray-rot scenefile outfile angle xres yres rpp

Scenefile is the file or path to file which contains the ray-tracer scene description.
Outfile is the file or path to file which will be created/overwritten for the output of the benchmark. Only .ppm files are supported.
Angle is the angle by which the rotation kernel will rotate the raytracing output.
xres and yres specify the horizontal and vertical resolution of the raytraced image, respectively.
rpp specifies the amount of per-pixel oversampling to be used; i.e. the number of rays shot into the scene for each pixel.

The specification of the number of threads used depends on the parallel programming model.

Benchmark versions:

Serial
POSIX Threads
OpenMP SuperScalar
