run pthread with:
./ray-rot ../sphfract ../kati 50 1920 1080 1 2 0

run omp and seq with:
./ray-rot ../sphfract ../kati 50 1920 1080 1

(produces kati.ppm - if not change to /dev/null)

Modified:
image.h
programm.cpp
ray_engine.cpp
rotation_engine.cpp
Makefile