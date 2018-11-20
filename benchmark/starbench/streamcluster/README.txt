Application: Streamcluster

This application benchmark performs online, cost-optimal clustering on streamed, arbitrary-dimension input data using the kmedian clustering algorithm.

Installation:

To install the benchmark, navigate to the directory this file is located in, open up a terminal and simply type 'make'. For certain architectures 
or special compilation options, you might need to change compilation parameters in the makefile.

Usage:

You may execute the benchmark by navigating to this directory after compilation and typing

./streamcluster 

The benchmark will then list a number of parameters you have to enter in order to execute. The (recommended) large input set for benchmarking
is invoked typing
	./streamcluster 10 20 128 1000000 200000 5000 none output.txt

The specification of the number of threads used to perform the clustering process depends on the parallel programming model.

Benchmark versions:

Serial
POSIX Threads
OpenMP SuperScalar
