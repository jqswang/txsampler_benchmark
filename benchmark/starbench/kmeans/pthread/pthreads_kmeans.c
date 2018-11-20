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
#include <pthread.h>
#include <time.h>
#include <math.h>
#include <sched.h>
#include <sys/syscall.h>
#include <errno.h>
#include <unistd.h>
#include "kmeans.h"

#define PREC 300

extern int nthreads; /* Thread count */
extern int pinning;

double delta; /* Delta is a value between 0 and 1 describing the percentage of objects which changed cluster membership */
volatile int finished;

pthread_barrier_t barr;
pthread_mutex_t lock1;
pthread_attr_t attr;

/*
*	Struct: input
*	-------------
*	Encapsulates all the input data for the benchmark, i.e. the object list,
*	clustering output and the input statistics.
*/
struct input {
    int tid;
    int pin_thread;
    double **objects;				/* Object list */
    double  **clusters;       		/* Cluster list, out: [numClusters][numCoords] */
    int    *membership;				/* For each object, contains the index of the cluster it currently belongs to */
    int    **local_newClusterSize; 	/* Thread-safe, [nthreads][numClusters] */
    double ***local_newClusters;    	/* Thread-safe, [nthreads][numClusters][numCoords] */
    int numObjs,numClusters,numCoords;
};

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

void work(struct input *x){
    int tid = x->tid;
    double local_delta=0;
    for (int i = tid; i < x->numObjs; i += nthreads) {
        /* find the array index of nearest cluster center */
        int index = find_nearest_cluster(x->numClusters, x->numCoords,
                                    x->objects[i], x->clusters);

        /* if membership changes, increase delta by 1 */
        if (x->membership[i] != index)
            local_delta += 1.0;
        /* assign the membership to object i */
        x->membership[i] = index;

        /* update new cluster centers : sum of all objects located
        within (average will be performed later) */
        x->local_newClusterSize[tid][index]++;
        for (int j=0; j < x->numCoords; j++)
            x->local_newClusters[tid][index][j] += x->objects[i][j];

    }
    pthread_mutex_lock(&lock1);
    delta +=local_delta;
    pthread_mutex_unlock(&lock1);
}
/*
*	Function: thread function work
*	--------------
*	Worker function for threading. Work distribution is done so that each thread computers
*/
void* tfwork(void *ip)
{
    struct input *x;
    x = (struct input *)ip;

    if (x->pin_thread){
      cpu_set_t mask;
      CPU_ZERO(&mask);
      CPU_SET(x->tid, &mask);
      pid_t tid = syscall(SYS_gettid); //glibc does not provide a wrapper for gettid
      int err= sched_setaffinity(tid, sizeof(cpu_set_t), &mask);
      if (err){
        fprintf(stderr, "failed to set core affinity for thread %d\n", x->tid);
        switch (errno){
        case EFAULT:
          fprintf(stderr, "EFAULT\n");
          break;
        case EINVAL:
          fprintf(stderr, "EINVAL\n");
          break;
        case EPERM:
          fprintf(stderr, "EPERM\n");
          break;
        case ESRCH:
          fprintf(stderr, "ESRCH\n");
          break;
        default:
          fprintf(stderr, "unknown error %d\n", err);
          break;
        }
      }
    }

    for(;;){
        pthread_barrier_wait(&barr);
        if (finished){
            break;
        }
        work(x);
        pthread_barrier_wait(&barr);
    }

	pthread_exit(NULL);
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
*	Function: create_array_2Dd_i
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
double** pthreads_kmeans(int is_perform_atomic, 	/* in: */
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
    int    **local_newClusterSize; /* [nthreads][numClusters] */
    double ***local_newClusters;    /* [nthreads][numClusters][numCoords] */

	pthread_t *thread;

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
	local_newClusterSize = create_array_2d_i(nthreads, numClusters);

    /* local_newClusters is a 3D array */
    local_newClusters    = (double***)malloc(nthreads * sizeof(double**));
    assert(local_newClusters != NULL);
    local_newClusters[0] = (double**) malloc(nthreads * numClusters * sizeof(double*));
    assert(local_newClusters[0] != NULL);

	/* Set up the pointers */
    for (i = 1; i < nthreads; i++)
        local_newClusters[i] = local_newClusters[i-1] + numClusters;

    for (i = 0; i < nthreads; i++) {
        for (j = 0; j < numClusters; j++) {
            local_newClusters[i][j] = (double*)calloc(numCoords, sizeof(double));
            assert(local_newClusters[i][j] != NULL);
        }
    }
    /* Perform thread setup */
    thread = (pthread_t*)calloc(nthreads, sizeof(pthread_t));

    pthread_barrier_init(&barr, NULL, nthreads);
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    pthread_mutex_init(&lock1, NULL);
    finished=0;

    struct input *ip = malloc(nthreads * sizeof(struct input));
    /* Provide thread-safe memory locations for each worker */
    for(i = 0; i < nthreads; i++){
        ip[i].tid = i;
        ip[i].pin_thread = pinning;
        ip[i].objects=objects;
        ip[i].clusters=clusters;
        ip[i].membership=membership;
        ip[i].local_newClusterSize=local_newClusterSize;
        ip[i].local_newClusters=local_newClusters;
        ip[i].numObjs=numObjs;
        ip[i].numClusters=numClusters;
        ip[i].numCoords=numCoords;

        if (i>0){
            rc = pthread_create(&thread[i], &attr, tfwork, (void *)&ip[i]);
            if (rc) {
                fprintf(stderr, "ERROR: Return Code For Thread Creation Is %d\n", rc);
                exit(EXIT_FAILURE);
            }
        }
    }

	/* === COMPUTATIONAL PHASE === */

    do {
        delta = 0.0;
        pthread_barrier_wait(&barr);
        work(&ip[0]);

        pthread_barrier_wait(&barr);
		/* Let the main thread perform the array reduction */
		for (i = 0; i < numClusters; i++) {
			for (j = 0; j < nthreads; j++) {
				newClusterSize[i] += local_newClusterSize[j][i];
				local_newClusterSize[j][i] = 0.0;
				for (k = 0; k < numCoords; k++) {
					newClusters[i][k] += local_newClusters[j][i][k];
					local_newClusters[j][i][k] = 0.0;
				}
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

	// Changing to a fixed number of iterations is for benchmarking reasons. I know it affects the results compared to the original program,
    // but minor double precision floating point inaccuracies caused by threading would otherwise lead to huge differences in computed
    // iterations, therefore making benchmarking completely unreliable.

    finished=1;
    pthread_barrier_wait(&barr);

    for(i = 1; i < nthreads; i++) {
        rc = pthread_join(thread[i], NULL);
        if (rc) {
            fprintf(stderr, "ERROR: Return Code For Thread Join Is %d\n", rc);
            exit(EXIT_FAILURE);
        }
    }

    free(ip);
    free(thread);
    pthread_barrier_destroy(&barr);
    pthread_mutex_destroy(&lock1);
	pthread_attr_destroy(&attr);

    free(local_newClusterSize[0]);
    free(local_newClusterSize);

    for (i = 0; i < nthreads; i++)
        for (j = 0; j < numClusters; j++)
            free(local_newClusters[i][j]);
    free(local_newClusters[0]);
    free(local_newClusters);

    free(newClusters[0]);
    free(newClusters);
    free(newClusterSize);
    return clusters;
}

