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

#define BS 1024
#define PREC 300

double delta; /* Delta is a value between 0 and 1 describing the percentage of objects which changed cluster membership */

/*
*	Function: euclid_dist_2
*	-----------------------
*	Computes the square of the euclidean distance between two multi-dimensional points.
*/
__inline static double euclid_dist_2(int numdims, double *coord1, double *coord2) {
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
__inline static int find_nearest_cluster(int numClusters, int numCoords, double *object, double **clusters) {
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

void cluster(int numClusters, int numCoords, int numObjs, int* membership, double** objects, int* local_newClusterSize, double* local_newClusters, double** clusters) {
    for(int i = 0; i < numObjs; i++) {
        int index = find_nearest_cluster(numClusters, numCoords, objects[i], clusters);

        if(membership[i] != index) {
            delta += 1.0;
        }

        membership[i] = index;

        local_newClusterSize[index]++;
        for(int j = 0; j < numCoords; j++)
            local_newClusters[index*numCoords+j] += objects[i][j];
    }
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
double** kmeans(int is_perform_atomic, 	/* in: */
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
    int     *local_newClusterSize;
    double   *local_newClusters;

    /* Compute task parameters */
//     int rem = numObjs%BS;
//     int even = numObjs-rem;
//     int tasks = numObjs/BS + (rem == 0 ? 0 : 1);

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

    local_newClusterSize = calloc(numClusters, sizeof(int));
    assert(local_newClusterSize != NULL);

    local_newClusters = calloc(numClusters * numCoords, sizeof(double));
    assert(local_newClusters != NULL);

	/* === COMPUTATIONAL PHASE === */
    do {
		delta = 0.0;

        cluster(numClusters, numCoords, numObjs, membership, objects, local_newClusterSize, local_newClusters, clusters);

		/* Let the main thread perform the array reduction */
		for (i = 0; i < numClusters; i++) {

            newClusterSize[i] += local_newClusterSize[i];
            local_newClusterSize[i] = 0.0;
            for (k = 0; k < numCoords; k++) {
                newClusters[i][k] += local_newClusters[i*numCoords + k];
                local_newClusters[i*numCoords + k] = 0.0;
            }

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
    } while (loop++ < PREC && delta > threshold);

    // This was changed for benchmarking reasons. I know it affects the results compared to the original program,
    // but minor double precision floating point inaccuracies caused by threading would otherwise lead to huge differences in computed
    // iterations, therefore making benchmarking completely unreliable.

    free(local_newClusterSize);
    free(local_newClusters);

    free(newClusters[0]);
    free(newClusters);
    free(newClusterSize);
    return clusters;
}

