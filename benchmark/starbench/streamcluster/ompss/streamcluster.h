/*
*   File: streamcluster.h
*   ---------------------
*   Header file providing global defines and structures
*   needed for the kernel.
*/

#ifndef __CLUSTER_H__
#define __CLUSTER_H__

#define MAXNAMESIZE 1024 // max filename length
#define SEED 1
/* increase this to reduce probability of random error */
/* increasing it also ups running time of "speedy" part of the code */
/* SP = 1 seems to be fine */
#define SP 1 // number of repetitions of speedy must be >=1

/* higher ITER --> more likely to get correct # of centers */
/* higher ITER also scales the running time almost linearly */
#define ITER 3 // iterate ITER* k log k times; ITER >= 1

#define CACHE_LINE 64 // cache line in byte

/* this structure represents a point */
/* these will be passed around to avoid copying coordinates */
typedef struct {
  float weight;
  float *coord;
  long assign;  /* number of point where this one is assigned */
  float cost;  /* cost of that assignment, weight*distance */
} Point;

/* this is the array of points */
typedef struct {
  long num; /* number of points; may not be N if this is a sample */
  int dim;  /* dimensionality */
  Point *p; /* the array itself */
} Points;

/* Synthetic input stream */
typedef struct {
	int n;
} SimStream;

/* Timing stuff */
typedef struct timeval timer;
#define TIME(X) gettimeofday(&X, NULL);

#endif // __CLUSTER_H__
