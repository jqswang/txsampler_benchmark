run pthreads with:

./kmeans -b -i ../edge -n 500 2 0

run omp and seq with:

./kmeans -b -i ../edge -n 500

(script says to run with ./kmeans -b -i ../edge -n 2000 but for less time use 500)

Changed:
kmeans_ompss.c
kmeans.h
Makefile