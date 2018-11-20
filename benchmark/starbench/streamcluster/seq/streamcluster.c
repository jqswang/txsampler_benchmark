/*
 *  Copyright (C) 2008 Princeton University
 *  All rights reserved.
 *  Authors: Jia Deng, Gilberto Contreras
 *
 *  streamcluster - Online clustering algorithm
 *
 *  Modified by Michael Andersch, 2011, TU Berlin.
 *
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include <limits.h>
#include "streamcluster.h"
#include "util.h"

static bool* switch_membership; //whether to switch membership in pgain
static bool* is_center; //whether a point is a center
static int* center_table; //index table of centers

float pspeedy(Points *points, float z, long *kcenter)
{
    static double totalcost; 
    static int i;

    /* create center at first point, send it to itself */
    for( int k = 0; k < points->num; k++ )    {
        float distance = dist(points->p[k],points->p[0],points->dim);
        points->p[k].cost = distance * points->p[k].weight;
        points->p[k].assign=0;
    }

    *kcenter = 1;

    for(i = 1; i < points->num; i++ )  {
            bool to_open = ((float)lrand48()/(float)INT_MAX)<(points->p[i].cost/z);
            if( to_open ) {
                (*kcenter)++;
                for( int k = 0; k < points->num; k++ )  {
                    float distance = dist(points->p[i],points->p[k],points->dim);
                    if( distance*points->p[k].weight < points->p[k].cost )  {
                        points->p[k].cost = distance * points->p[k].weight;
                        points->p[k].assign=i;
                    }
                }
            }
        }

    // aggregate costs from each thread
    totalcost=z*(*kcenter);
    
    for( int k = 0; k < points->num; k++ ) {
        totalcost += points->p[k].cost;
    }
    
    return(totalcost);
}

/* For a given point x, find the cost of the following operation:
 * -- open a facility at x if there isn't already one there,
 * -- for points y such that the assignment distance of y exceeds dist(y, x),
 *    make y a member of x,
 * -- for facilities y such that reassigning y and all its members to x 
 *    would save cost, realize this closing and reassignment.
 * 
 * If the cost of this operation is negative (i.e., if this entire operation
 * saves cost), perform this operation and return the amount of cost saved;
 * otherwise, do nothing.
 */

/* numcenters will be updated to reflect the new number of centers */
/* z is the facility cost, x is the number of this point in the array 
   points */

double pgain(long x, Points *points, double z, long int *numcenters)
{
    int i;
    int number_of_centers_to_close = 0;

    static double *work_mem;
    static double gl_cost_of_opening_x;
    static int gl_number_of_centers_to_close;

    //each thread takes a block of working_mem.
    int stride = *numcenters+2;
    //make stride a multiple of CACHE_LINE
    int cl = CACHE_LINE/sizeof(double);
    if( stride % cl != 0 ) { 
        stride = cl * ( stride / cl + 1);
    }
    int K = stride -2 ; // K==*numcenters
    
    //my own cost of opening x
    double cost_of_opening_x = 0;

    work_mem = (double*) malloc(2*stride*sizeof(double));
    gl_cost_of_opening_x = 0;
    gl_number_of_centers_to_close = 0;

    /*For each center, we have a *lower* field that indicates 
        how much we will save by closing the center. 
        Each thread has its own copy of the *lower* fields as an array.
        We first build a table to index the positions of the *lower* fields. 
    */

    int count = 0;
    for( int i = 0; i < points->num; i++ ) {
        if( is_center[i] ) {
            center_table[i] = count++;
        }
    }

    //now we finish building the table. clear the working memory.
    memset(switch_membership, 0, points->num*sizeof(bool));
    memset(work_mem, 0, 2*stride*sizeof(double));

    //my *lower* fields
    double* lower = work_mem;//&work_mem[pid*stride];
    //global *lower* fields
    double* gl_lower = &work_mem[stride];//&work_mem[nproc*stride];

    for ( i = 0; i < points->num; i++ ) {
        float x_cost = dist(points->p[i], points->p[x], points->dim) 
        * points->p[i].weight;
        float current_cost = points->p[i].cost;

        if ( x_cost < current_cost ) {

            // point i would save cost just by switching to x
            // (note that i cannot be a median, 
            // or else dist(p[i], p[x]) would be 0)
            
            switch_membership[i] = 1;
            cost_of_opening_x += x_cost - current_cost;

        } else {

            // cost of assigning i to x is at least current assignment cost of i

            // consider the savings that i's **current** median would realize
            // if we reassigned that median and all its members to x;
            // note we've already accounted for the fact that the median
            // would save z by closing; now we have to subtract from the savings
            // the extra cost of reassigning that median and its members 
            int assign = points->p[i].assign;
            lower[center_table[assign]] += current_cost - x_cost;
        }
    }

    // at this time, we can calculate the cost of opening a center
    // at x; if it is negative, we'll go through with opening it

    for ( int i = 0; i < points->num; i++ ) {
        if( is_center[i] ) {
            double low = z + work_mem[center_table[i]];
            gl_lower[center_table[i]] = low;
            if ( low > 0 ) {
                // i is a median, and
                // if we were to open x (which we still may not) we'd close i

                // note, we'll ignore the following quantity unless we do open x
                ++number_of_centers_to_close;  
                cost_of_opening_x -= low;
            }
        }
    }
    //use the rest of working memory to store the following
    work_mem[K] = number_of_centers_to_close;
    work_mem[K+1] = cost_of_opening_x;

    gl_cost_of_opening_x = z;
    //aggregate
    gl_number_of_centers_to_close += (int)work_mem[K];//p*stride + K];
    gl_cost_of_opening_x += work_mem[K+1];//p*stride+K+1];

    // Now, check whether opening x would save cost; if so, do it, and
    // otherwise do nothing

    if ( gl_cost_of_opening_x < 0 ) {
        //  we'd save money by opening x; we'll do it
        for ( int i = 0; i < points->num; i++ ) {
            bool close_center = gl_lower[center_table[points->p[i].assign]] > 0 ;
            if ( switch_membership[i] || close_center ) {
                // Either i's median (which may be i itself) is closing,
                // or i is closer to x than to its current median
                points->p[i].cost = points->p[i].weight *
                dist(points->p[i], points->p[x], points->dim);
                points->p[i].assign = x;
            }
        }
        for( int i = 0; i < points->num; i++ ) {
            if( is_center[i] && gl_lower[center_table[i]] > 0 ) {
                is_center[i] = false;
            }
        }
        if( x >= 0 && x < points->num ) {
            is_center[x] = true;
        }

        *numcenters = *numcenters + 1 - gl_number_of_centers_to_close;
    } else {
        gl_cost_of_opening_x = 0;  // the value we'll return
    }
    free(work_mem);
    return -gl_cost_of_opening_x;
}

/* facility location on the points using local search */
/* z is the facility cost, returns the total cost and # of centers */
/* assumes we are seeded with a reasonable solution */
/* cost should represent this solution's cost */
/* halt if there is < e improvement after iter calls to gain */
/* feasible is an array of numfeasible points which may be centers */

 float pFL(Points *points, int *feasible, int numfeasible,
	  float z, long *k, double cost, long iter, float e)
{
    long i;
    long x;
    double change;
    long numberOfPoints;

    change = cost;
    /* continue until we run iter iterations without improvement */
    /* stop instead if improvement is less than e */
    while (change/cost > 1.0*e) {
        change = 0.0;
        numberOfPoints = points->num;
        /* randomize order in which centers are considered */
        intshuffle(feasible, numfeasible);
        for (i=0;i<iter;i++) {
            x = i%numfeasible;
            change += pgain(feasible[x], points, z, k);
        }
        cost -= change;
    }
    return(cost);
}

int selectfeasible_fast(Points *points, int **feasible, int kmin)
{
    int numfeasible = points->num;
    if (numfeasible > (ITER*kmin*log((double)kmin)))
        numfeasible = (int)(ITER*kmin*log((double)kmin));
    *feasible = (int *)malloc(numfeasible*sizeof(int));
    
    float* accumweight;
    float totalweight;

    /* 
        Calculate my block. 
        For now this routine does not seem to be the bottleneck, so it is not parallelized. 
        When necessary, this can be parallelized by setting k1 and k2 to 
        proper values and calling this routine from all threads ( it is called only
        by thread 0 for now ). 
        Note that when parallelized, the randomization might not be the same and it might
        not be difficult to measure the parallel speed-up for the whole program. 
    */

    float w;
    int l,r,k;

    /* not many points, all will be feasible */
    if (numfeasible == points->num) {
        for (int i=0;i<numfeasible;i++)
            (*feasible)[i] = i;
        return numfeasible;
    }

    accumweight= (float*)malloc(sizeof(float)*points->num);

    accumweight[0] = points->p[0].weight;
    totalweight=0;
    for( int i = 1; i < points->num; i++ ) {
        accumweight[i] = accumweight[i-1] + points->p[i].weight;
    }
    totalweight=accumweight[points->num-1];

    for(int i=0; i<numfeasible; i++ ) {
        w = (lrand48()/(float)INT_MAX)*totalweight;
        //binary search
        l=0;
        r=points->num-1;
        if( accumweight[0] > w )  { 
            (*feasible)[i]=0; 
            continue;
        }
        while( l+1 < r ) {
            k = (l+r)/2;
            if( accumweight[k] > w ) {
                r = k;
            } else {
                l=k;
            }
        }
        (*feasible)[i]=r;
    }

    free(accumweight); 

    return numfeasible;
}

/* compute approximate kmedian on the points */
float pkmedian(Points *points, long kmin, long kmax, long* kfinal)
{
    int i;
    double cost;
    double lastcost;
    double hiz, loz, z;

    static long k;
    static int *feasible;
    static int numfeasible;

    hiz = loz = 0.0;
    long numberOfPoints = points->num;
    long ptDimension = points->dim;

    // Compute distance between first point and all other points
    // assigned to this thread
    for (long kk=0; kk < points->num; kk++ ) {
        hiz += dist(points->p[kk], points->p[0], ptDimension) * points->p[kk].weight;
    }

    loz=0.0; 
    z = (hiz+loz)/2.0;
    /* NEW: Check whether more centers than points! */
    if (points->num <= kmax) {
        /* just return all points as facilities */
        for (long kk=0; kk < points->num; kk++) {
            points->p[kk].assign = kk;
            points->p[kk].cost = 0;
        }
        cost = 0;
        *kfinal = k;
        return cost;
    }

    shuffle(points);
    cost = pspeedy(points, z, &k);

    i=0;
    /* give speedy SP chances to get at least kmin/2 facilities */
    while ((k < kmin) && (i < SP)) {
        cost = pspeedy(points, z, &k);
        i++;
    }

    /* if still not enough facilities, assume z is too high */
    while (k < kmin) {
        if (i >= SP) {
            hiz=z; z=(hiz+loz)/2.0; i=0;
        }

        shuffle(points);
        cost = pspeedy(points, z, &k);
        i++;
    }

    /* now we begin the binary search for real */
    /* must designate some points as feasible centers */
    /* this creates more consistancy between FL runs */
    /* helps to guarantee correct # of centers at the end */
    
    numfeasible = selectfeasible_fast(points,&feasible,kmin);
    for( int i = 0; i < points->num; i++ ) {
        is_center[points->p[i].assign]= true;
    }
    
    while(1) {
        /* first get a rough estimate on the FL solution */
        lastcost = cost;
        cost = pFL(points, feasible, numfeasible,
            z, &k, cost, (long)(ITER*kmax*log((double)kmax)), 0.1);

        /* if number of centers seems good, try a more accurate FL */
        if (((k <= (1.1)*kmax)&&(k >= (0.9)*kmin))||
        ((k <= kmax+2)&&(k >= kmin-2))) {

            /* may need to run a little longer here before halting without
            improvement */
            cost = pFL(points, feasible, numfeasible,
                z, &k, cost, (long)(ITER*kmax*log((double)kmax)), 0.001);
        }

        if (k > kmax) {
            /* facilities too cheap */
            /* increase facility cost and up the cost accordingly */
            loz = z; z = (hiz+loz)/2.0;
            cost += (z-loz)*k;
        }
        if (k < kmin) {
            /* facilities too expensive */
            /* decrease facility cost and reduce the cost accordingly */
            hiz = z; z = (hiz+loz)/2.0;
            cost += (z-hiz)*k;
        }

        /* if k is good, return the result */
        /* if we're stuck, just give up and return what we have */
        if (((k <= kmax)&&(k >= kmin))||((loz >= (0.999)*hiz)) ) { 
            break;
        }
    }

    //clean up...
    free(feasible); 
    *kfinal = k;
    
    return cost;
}

/* compute the means for the k clusters */
int contcenters(Points *points)
{
    long i, ii;
    float relweight;

    for (i=0; i < points->num; i++) {
        /* compute relative weight of this point to the cluster */
        if (points->p[i].assign != i) {
            relweight=points->p[points->p[i].assign].weight + points->p[i].weight;
            relweight = points->p[i].weight/relweight;
            for (ii=0; ii<points->dim; ii++) {
                points->p[points->p[i].assign].coord[ii]*=1.0-relweight;
                points->p[points->p[i].assign].coord[ii]+=
                points->p[i].coord[ii]*relweight;
            }
            points->p[points->p[i].assign].weight += points->p[i].weight;
        }
    }
    
    return 0;
}

/* copy centers from points to centers */
void copycenters(Points *points, Points* centers, long* centerIDs, long offset)
{
    long i;
    long k;

    bool *is_a_median = (bool *) calloc(points->num, sizeof(bool));

    /* mark the centers */
    for ( i = 0; i < points->num; i++ ) {
        is_a_median[points->p[i].assign] = 1;
    }

    k=centers->num;

    /* count how many  */
    for ( i = 0; i < points->num; i++ ) {
        if ( is_a_median[i] ) {
            memcpy( centers->p[k].coord, points->p[i].coord, points->dim * sizeof(float));
            centers->p[k].weight = points->p[i].weight;
            centerIDs[k] = i + offset;
            k++;
        }
    }

    centers->num = k;

    free(is_a_median);
}

void localSearch( Points* points, long kmin, long kmax, long* kfinal ) {
    pkmedian(points, kmin, kmax, kfinal);
}

/* --------------------------------------------------*/
/*---------------Streaming functions-----------------*/
/*---------------------------------------------------*/

/* Synthetic input stream read */
size_t syn_read(float* dest, int dim, int num, SimStream* stream) {
	size_t count = 0;
	for(int i = 0; i < num && stream->n > 0; i++) {
		for(int k = 0; k < dim; k++) {
			dest[i*dim + k] = lrand48()/(float)INT_MAX;
		}
		stream->n--;
		count++;
		//printf("n=%d\t", *n);
	}
	return count;
}

/* Synthetic input stream EOF */
int syn_feof(SimStream* stream) {
	return (stream->n <= 0);
}

/* Native input stream read */
size_t file_read(float* dest, int dim, int num, FILE* fp) {
	return 0;
}

/* Native input stream EOF */
int file_feof(FILE* fp) {
	return feof(fp);
}

/*
*   Function: outcenterIDs
*   ----------------------
*   Writes the output of the computation to disk.
*/
void outcenterIDs( Points* centers, long* centerIDs, char* outfile ) {
    FILE* fp = fopen(outfile, "w");
    if( fp==NULL ) {
        fprintf(stderr, "error opening %s\n",outfile);
        exit(1);
    }
    int* is_a_median = (int*)calloc( sizeof(int), centers->num );
    for( int i =0 ; i< centers->num; i++ ) {
        is_a_median[centers->p[i].assign] = 1;
    }

    for( int i = 0; i < centers->num; i++ ) {
        if( is_a_median[i] ) {
            fprintf(fp, "%lu\n", centerIDs[i]);
            fprintf(fp, "%lf\n", centers->p[i].weight);
            for( int k = 0; k < centers->dim; k++ ) {
                fprintf(fp, "%lf ", centers->p[i].coord[k]);
            }
            fprintf(fp,"\n\n");
        }
    }
    fclose(fp);

    free(is_a_median);
}

/*
*   Function: timevaldiff
*   ---------------------
*   Calculates the time difference between start and finish in msecs.
*/
long timevaldiff(timer* start, timer* finish){
    long msec;
    msec = (finish->tv_sec - start->tv_sec)*1000;
    msec += (finish->tv_usec - start->tv_usec)/1000;
    return msec;
}

/*
*   Function: streamCluster
*   -----------------------
*   Kernel main function.
*/
void streamCluster( SimStream* stream, 
		    long kmin, long kmax, int dim,
		    long chunksize, long centersize, char* outfile ) {

    float* block = (float*)malloc( chunksize*dim*sizeof(float) );
    float* centerBlock = (float*)malloc(centersize*dim*sizeof(float) );
    long* centerIDs = (long*)malloc(centersize*dim*sizeof(long));

    if( block == NULL ) { 
        fprintf(stderr,"not enough memory for a chunk!\n");
        exit(EXIT_FAILURE);
    }

    Points points;
    points.dim = dim;
    points.num = chunksize;
    points.p = 
    (Point *)malloc(chunksize*sizeof(Point));

    for( int i = 0; i < chunksize; i++ ) {
        points.p[i].coord = &block[i*dim];
    }

    Points centers;
    centers.dim = dim;
    centers.p = 
    (Point *)malloc(centersize*sizeof(Point));
    centers.num = 0;

    for( int i = 0; i < centersize; i++ ) {
        centers.p[i].coord = &centerBlock[i*dim];
        centers.p[i].weight = 1.0;
    }

    long IDoffset = 0;
    long kfinal;

    while(1) {

        size_t numRead = syn_read(block, dim, chunksize, stream);
        fprintf(stderr,"read %d points\n", (int)numRead);

        if(numRead < (unsigned int)chunksize && !syn_feof(stream) ) {
            fprintf(stderr, "error reading data!\n");
            exit(EXIT_FAILURE);
        }

        points.num = numRead;
        for( int i = 0; i < points.num; i++ ) {
            points.p[i].weight = 1.0;
        }
        switch_membership = (bool*)malloc(points.num*sizeof(bool));
        is_center = (bool*)calloc(points.num,sizeof(bool));
        center_table = (int*)malloc(points.num*sizeof(int));

        localSearch(&points, kmin, kmax, &kfinal); // parallel

        contcenters(&points); /* sequential */

        if( kfinal + centers.num > centersize ) {
            //here we don't handle the situation where # of centers gets too large. 
            fprintf(stderr,"oops! no more space for centers\n");
            exit(EXIT_FAILURE);
        }

        copycenters(&points, &centers, centerIDs, IDoffset); /* sequential */
        IDoffset += numRead;

        free(is_center);
        free(switch_membership);
        free(center_table);

        if(syn_feof(stream)) {
            break;
        }
    }

    //finally cluster all temp centers
    switch_membership = (bool*)malloc(centers.num*sizeof(bool));
    is_center = (bool*)calloc(centers.num,sizeof(bool));
    center_table = (int*)malloc(centers.num*sizeof(int));

    localSearch( &centers, kmin, kmax ,&kfinal ); // parallel
    contcenters(&centers);
    outcenterIDs( &centers, centerIDs, outfile);

    free(block);
    free(centerBlock);
    free(centerIDs);

    free(centers.p);
    free(points.p);

    free(switch_membership);
    free(is_center);
    free(center_table);
}

int main(int argc, char **argv)
{
    char *outfilename = (char*)calloc(MAXNAMESIZE, sizeof(char));
    char *infilename = (char*)calloc(MAXNAMESIZE, sizeof(char));
    long kmin, kmax, n, chunksize, clustersize;
    int dim;

    timer start, end;

    fprintf(stderr,"Streamcluster Kernel\n");
    fflush(NULL);

    if (argc<9) {
        fprintf(stderr,"usage: %s k1 k2 d n chunksize clustersize infile outfile\n",
            argv[0]);
        fprintf(stderr,"  k1:          Min. number of centers allowed\n");
        fprintf(stderr,"  k2:          Max. number of centers allowed\n");
        fprintf(stderr,"  d:           Dimension of each data point\n");
        fprintf(stderr,"  n:           Number of data points\n");
        fprintf(stderr,"  chunksize:   Number of data points to handle per step\n");
        fprintf(stderr,"  clustersize: Maximum number of intermediate centers\n");
        fprintf(stderr,"  infile:      Input file (if n<=0)\n");
        fprintf(stderr,"  outfile:     Output file\n");
        fprintf(stderr,"\n");
        fprintf(stderr, "if n > 0, points will be randomly generated instead of reading from infile.\n");
        exit(1);
    }

    kmin = atoi(argv[1]);
    kmax = atoi(argv[2]);
    dim = atoi(argv[3]);
    n = atoi(argv[4]);
    chunksize = atoi(argv[5]);
    clustersize = atoi(argv[6]);
    strcpy(infilename, argv[7]);
    strcpy(outfilename, argv[8]);

    srand48(SEED);
	SimStream stream;
	if(n > 0) {
		stream.n = n;
	} else {
		fprintf(stderr, "Currently no input files supported!\n");
		exit(EXIT_FAILURE);
	}

    TIME(start);

    streamCluster(&stream, kmin, kmax, dim, chunksize, clustersize, outfilename );
  
    TIME(end);

    double ctime_msec = (double)timevaldiff(&start, &end);

    printf("Compute time: %ds (%d msec)\n", (int)(ctime_msec/1000), (int)ctime_msec);

    free(outfilename);
    free(infilename);

    return 0;
}
