Kernel: Ray Tracing

This is a kernel-type benchmark of a very simple and brute-force ray tracer.

Installation:

To install the kernel benchmark, navigate to the directory this file is located in, open up a terminal and simply type 'make'. For certain architectures 
or special compilation options, you might need to change compilation parameters in the makefile.

Usage:

You may execute the benchmark by navigating to this directory after compilation and typing

./c-ray-mt -i FILENAME -s RESOLUTION -o OUTPUT.ppm 

'FILENAME' has to be either "scene" or "sphfract" or another predefined scene description file if there is one.
'RESOLUTION' specifies the resolution of the produced image and has to be given in the form 1920x1200, for example.
'OUTPUT' is the name of the file the rendered image will be contained in after the benchmark ran.

The specification of how many threads are used to perform the rendering depends on the parallel programming model.

Benchmark Versions:

Serial
POSIX Threads
POSIX Threads with Line Interleaving
POSIX Threads with Dynamic Line Distribution
OpenMP SuperScalar
OpenMP


