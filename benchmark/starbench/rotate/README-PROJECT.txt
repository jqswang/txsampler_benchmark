run pthreads with:

./rot ../Berlin_Botanischer-Garten_HB_02.ppm ../kati.ppm 50 2 0

run omp and seq with:

./rot ../Berlin_Botanischer-Garten_HB_02.ppm ../kati.ppm 50

(produces kati.ppm - if not change to /dev/null)

Modified:
image.h
rotation_engine.cpp
Makefile