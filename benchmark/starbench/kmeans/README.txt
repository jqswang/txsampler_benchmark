Kernel: k-means Clustering

This is a kernel-type benchmark of a simple off-line clustering algorithm. Careful: Hash Checksum Verification of the benchmark output is not feasible for kmeans. Usage of the float data type leads to imprecisions, caused by threading, up to the fifth place after the decimal point in the results.

Installation:

To install the kernel benchmark, navigate to the directory this file is located in, open up a terminal and simply type 'make'. For certain architectures 
or special compilation options, you might need to change compilation parameters in the makefile.

Usage:

You may execute the benchmark by navigating to this directory after compilation and typing

./kmeans -b -i <input filename> -n <cluster center count>

The specification of the number of threads used to perform the clustering process depends on the parallel programming model.

Benchmark versions:

Serial
POSIX Threads
OpenMP SuperScalar
