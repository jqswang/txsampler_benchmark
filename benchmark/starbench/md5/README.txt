Kernel: Message-Digest 5

This is a kernel-type benchmark measuring the time it takes to MD5 some predefined buffers of bytes.

Installation:

To install the kernel benchmark, navigate to the directory this file is located in, go to any of the version subdirectories, open up a terminal and simply type 'make'. For certain architectures or special compilation options, you might need to change compilation parameters in the makefile.

Usage:

You may execute the benchmark by navigating to this directory after compilation and typing

./md5 -h

The -h option will display a help text showing which commandline options are available.

The specification of the number of threads used depends on the parallel programming model.

Benchmark versions:

Serial
POSIX Threads
OpenMP SuperScalar
