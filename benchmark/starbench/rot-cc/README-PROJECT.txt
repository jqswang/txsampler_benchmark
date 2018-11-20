run pthreads with:

./rot-cc ../Berlin_Botanischer-Garten_HB_02.ppm ../kati.ppm 50 2 0

run omp and seq with:

./rot-cc ../Berlin_Botanischer-Garten_HB_02.ppm ../kati.ppm 50

(produces kati.ppm if not change to /dev/null)

Modified:
benchmark_engine.cpp
image.h
Makefile

