/*
 * Copyright (C) 2013 Michael Andersch <michael.andersch@mailbox.tu-berlin.de>
 *
 * This file is part of Starbench.
 *
 * Starbench is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Starbench is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Starbench.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "kmeans.h"

#define BS 25
#define PREC 300

double delta; /* Delta is a value between 0 and 1 describing the percentage of objects which changed cluster membership */

/*
 *	Function: euclid_dist_2
 *	-----------------------
 *	Computes the square of the euclidean distance between two multi-dimensional points.
 */
inline double euclid_dist_2(int numdims, double *coord1, double *coord2) {
  int i;
  double ans=0.0;

  for (i=0; i<numdims; i++)
    ans += (coord1[i]-coord2[i]) * (coord1[i]-coord2[i]);

  return(ans);
}

/*
 *	Function: find_nearest_cluster
 *	------------------------------
 *	Function determining the cluster center which is closest to the given object.
 *	Returns the index of that cluster center.
 */
inline int find_nearest_cluster(int numClusters, int numCoords, double *object, double **clusters) {
  int   index, i;
  double dist, min_dist;

  /* find the cluster id that has min distance to object */
  index    = 0;
  min_dist = euclid_dist_2(numCoords, object, clusters[0]);
  for (i=1; i<numClusters; i++) {
    dist = euclid_dist_2(numCoords, object, clusters[i]);

    /* no need square root */
    if (dist < min_dist) { /* find the min and its array index */
      min_dist = dist;
      index    = i;
    }
  }
  return index;
}
//       cluster(tid, numClusters, numCoords, BS, &membership[i], &objects[i], &local_newClusterSize[tid*numClusters], &local_newClusters[tid*numClusters*numCoords], clusters);
static void cluster(int t, int numClusters, int numCoords, int bs, int* membership, double** objects, int* newClusterSize, double** newClusters, double** clusters) {
  int i,j;
  double local_delta=0;
  for(i=0; i < bs; i++) {
    int index = find_nearest_cluster(numClusters, numCoords, objects[i], clusters);
    if(membership[i] != index) {
      local_delta += 1.0;
    }
    membership[i] = index;
    #pragma omp atomic
    newClusterSize[index]++;

    //ugly because no support for this in omp/ompss (at least no aware)
    //every index can be updated in parallel, but cannot specify a lock based on value of index
    //therefor use 16 critical clauses
    switch (index %16){
#define STRINGIFY(a) #a
#define CRITICAL_SEC(x) \
      case x: \
        _Pragma( STRINGIFY( omp critical (i ## x) ) ) \
        for(j = 0; j < numCoords; j++){ \
          newClusters[index][j] += objects[i][j]; \
        } \
        break;\

      CRITICAL_SEC(0);
      CRITICAL_SEC(1);
      CRITICAL_SEC(2);
      CRITICAL_SEC(3);
      CRITICAL_SEC(4);
      CRITICAL_SEC(5);
      CRITICAL_SEC(6);
      CRITICAL_SEC(7);
      CRITICAL_SEC(8);
      CRITICAL_SEC(9);
      CRITICAL_SEC(10);
      CRITICAL_SEC(11);
      CRITICAL_SEC(12);
      CRITICAL_SEC(13);
      CRITICAL_SEC(14);
      CRITICAL_SEC(15);
#undef STRINGIFY
#undef CRITICAL_SEC
    }
  }
  #pragma omp atomic
  delta+=local_delta;
}

/*
 *	Function: create_array_2d_f
 *	--------------------------
 *	Allocates memory for a 2-dim double array as needed for the algorithm.
 */
double** create_array_2d_f(int height, int width) {
  double** ptr;
  int i;
  ptr = calloc(height, sizeof(double*));
  assert(ptr != NULL);
  ptr[0] = calloc(width * height, sizeof(double));
  assert(ptr[0] != NULL);
  /* Assign pointers correctly */
  for(i = 1; i < height; i++)
    ptr[i] = ptr[i-1] + width;
  return ptr;
}

/*
 *	Function: create_array_2D_i
 *	--------------------------
 *	Allocates memory for a 2-dim integer array as needed for the algorithm.
 */
int** create_array_2d_i(int height, int width) {
  int** ptr;
  int i;
  ptr = calloc(height, sizeof(int*));
  assert(ptr != NULL);
  ptr[0] = calloc(width * height, sizeof(int));
  assert(ptr[0] != NULL);
  /* Assign pointers correctly */
  for(i = 1; i < height; i++)
    ptr[i] = ptr[i-1] + width;
  return ptr;
}

/*
 *	Function: pthreads_kmeans
 *	-------------------------
 *	Algorithm main function. Returns a 2D array of cluster centers of size [numClusters][numCoords].
 */
double** starss_kmeans(int is_perform_atomic, 	/* in: */
                       double **objects,           	/* in: [numObjs][numCoords] */
                       int     numCoords,         	/* no. coordinates */
                       int     numObjs,           	/* no. objects */
                       int     numClusters,       	/* no. clusters */
                       double   threshold,         	/* % objects change membership */
                       int    *membership)        	/* out: [numObjs] */
{
  int      i, j, k, index, loop = 0, rc;
  int     *newClusterSize; /* [numClusters]: no. objects assigned in each
  new cluster */
  double  **clusters;       /* out: [numClusters][numCoords] */
  double  **newClusters;    /* [numClusters][numCoords] */
  double   timing;

  /* Compute task parameters */
  int rem = numObjs%BS;
  int even = numObjs-rem;
  int tasks = numObjs/BS + (rem == 0 ? 0 : 1);

  /* === MEMORY SETUP === */

  /* [numClusters] clusters of [numCoords] double coordinates each */
  clusters = create_array_2d_f(numClusters, numCoords);

  /* Pick first numClusters elements of objects[] as initial cluster centers */
  for (i=0; i < numClusters; i++)
    for (j=0; j < numCoords; j++)
      clusters[i][j] = objects[i][j];

  /* Initialize membership, no object belongs to any cluster yet */
  for (i = 0; i < numObjs; i++)
    membership[i] = -1;

  /* newClusterSize holds information on the count of members in each cluster */
  newClusterSize = (int*)calloc(numClusters, sizeof(int));
  assert(newClusterSize != NULL);

  /* newClusters holds the coordinates of the freshly created clusters */
  newClusters = create_array_2d_f(numClusters, numCoords);

  /* === COMPUTATIONAL PHASE === */
  do {
    delta = 0.0;

    /* Perform task spawn */
    int tid = 0;

//=================================================================================================================================
    
	  #pragma omp parallel for
	  for(i = 0; i < even; i += BS) {
	    cluster(tid, numClusters, numCoords, BS, &membership[i], &objects[i], newClusterSize, newClusters,  clusters);
	    tid++;
	  }
	  
//=================================================================================================================================
	  
	  if(rem != 0){
	    cluster(tid, numClusters, numCoords, rem, &membership[even], &objects[even],  newClusterSize, newClusters, clusters);
	  }
	

    /* Average the sum and replace old cluster centers with newClusters */
    for (i = 0; i < numClusters; i++) {
      for (j = 0; j < numCoords; j++) {
        if (newClusterSize[i] > 1)
          clusters[i][j] = newClusters[i][j] / newClusterSize[i];
        newClusters[i][j] = 0.0;   /* set back to 0 */
      }

      newClusterSize[i] = 0;   /* set back to 0 */
    }
    delta /= numObjs;
    //    } while (loop++ < PREC); //&& delta > threshold
  } while (loop++ < PREC && delta > threshold);

  // This was changed for benchmarking reasons. It affects the results compared to the original program,
  // but minor double precision floating point inaccuracies caused by threading would otherwise lead to differences in computed
  // iterations, therefore making benchmarking completely unreliable.

  free(newClusters[0]);
  free(newClusters);
  free(newClusterSize);
  return clusters;
}

