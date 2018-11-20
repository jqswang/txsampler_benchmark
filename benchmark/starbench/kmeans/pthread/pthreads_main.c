/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*   File:         pthreads_main.c   (an OpenMP version)                          */
/*   Description:  This program shows an example on how to call a subroutine */
/*                 that implements a simple k-means clustering algorithm     */
/*                 based on Euclid distance.                                 */
/*   Input file format:                                                      */
/*                 ascii  file: each line contains 1 data object             */
/*                 binary file: first 4-byte integer is the number of data   */
/*                 objects and 2nd integer is the no. of features (or        */
/*                 coordinates) of each object                               */
/*                                                                           */
/*   Author:  Wei-keng Liao                                                  */
/*            ECE Department Northwestern University                         */
/*            email: wkliao@ece.northwestern.edu                             */
/*   Copyright, 2005, Wei-keng Liao                                          */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>     /* strtok() */
#include <sys/types.h>  /* open() */
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>     /* getopt() */
#include <time.h>
#include "kmeans.h"

#define seconds(tm) gettimeofday(&tp,(struct timezone *)0);\
   tm=tp.tv_sec+tp.tv_usec/1000000.0

struct timeval tp;

int numClusters, numCoords, numObjs, nthreads, pinning;

/*
*	Function: usage
*	---------------
*	Prints information on how to call the program.
*/
static void usage(char *argv0) {
    char *help =
        "Usage: %s [switches] -i filename -n num_clusters [OPTIONS]\n"
        "       -i filename    : file containing data to be clustered\n"
        "       -b             : input file is in binary format (default no)\n"
        "       -n num_clusters: number of clusters (K must be > 1)\n"
        "       -t nproc       : number of threads (default 1)\n"
        "       -p <0|1>       : pin threads (default 0)\n"
        "       -o filename    : write output to file\n";
    fprintf(stderr, help, argv0);
    exit(-1);
}

/*---< main() >-------------------------------------------------------------*/
int main(int argc, char **argv) {
    int     opt;
    extern char   *optarg;
    extern int     optind;
    int     i, j;
    int     isBinaryFile;
    
    int    *membership;    /* [numObjs] */
    char   *filename, *outfile;
    double **objects;       /* [numObjs][numCoords] data objects */
    double **clusters;      /* [numClusters][numCoords] cluster center */
    double   threshold;
    double  timing, io_timing, clustering_timing;
    
    /* some default values */
    nthreads          = 1;		/* Amount of threads to use */
    numClusters       = 1;		/* Amount of cluster centers */
    threshold         = 0.001; 	/* Percentage of objects that need to change membership for the clusting to continue */
    isBinaryFile      = 0;		/* 0 if the input file is in ASCII format, 1 for binary format */
    filename          = NULL;	/* Name of the input file */
	outfile   		  = NULL;	/* Name of the output file */

	/* Parse command line options */
    while ( (opt=getopt(argc,argv,"o:p:i:n:t:bh"))!= EOF) {
        switch (opt) {
            case 'i': filename=optarg; break;
            case 'b': isBinaryFile = 1; break;
            case 'n': numClusters = atoi(optarg); break;
            case 't': nthreads = atoi(optarg); break;
            case 'p': pinning = atoi(optarg)!=0; break;
            case 'h': usage(argv[0]); break;
			case 'o': outfile=optarg; break;
            default: usage(argv[0]); break;
        }
    }

    if (filename == NULL) usage(argv[0]);

	seconds(io_timing);

    /* Read input data points from given input file */
    objects = file_read(isBinaryFile, filename, &numObjs, &numCoords);
    assert(objects != NULL);

	seconds(timing);
	io_timing         = timing - io_timing;
	clustering_timing = timing;      

    membership = (int*) malloc(numObjs * sizeof(int));
	assert(membership != NULL);

	/* Launch the core computation algorithm */
    clusters = pthreads_kmeans(0, objects, numCoords, numObjs,
                          numClusters, threshold, membership);

    free(objects[0]);
    free(objects);

    seconds(timing);
    clustering_timing = timing - clustering_timing;

    /* Memory cleanup */
    free(membership);

	if(outfile != NULL) {
        int l;
        FILE* fp = fopen(outfile, "w");
        for(j = 0; j < numClusters; j++) {
            fprintf(fp, "Cluster %d: ", j);
            for(l = 0; l < numCoords; l++)
                fprintf(fp, "%f ", clusters[j][l]);
            fprintf(fp, "\n");
        }
        fclose(fp);
    }

    free(clusters[0]);
    free(clusters);

    /* Print performance numbers on stdout */
	double t1;
	io_timing += seconds(t1) - timing;

    printf("\n---- kMeans Clustering ----\n");
    printf("Number of threads = %d\n", nthreads);
    printf("Input file:     %s\n", filename);
    printf("numObjs       = %d\n", numObjs);
    printf("numCoords     = %d\n", numCoords);
    printf("numClusters   = %d\n", numClusters);
    printf("threshold     = %.4f\n", threshold);

    printf("I/O time           = %10.4f sec\n", io_timing);
    printf("Computation timing = %10.4f sec\n", clustering_timing);

    return(0);
}

