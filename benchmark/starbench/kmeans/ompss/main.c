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

int numClusters, numCoords, numObjs;

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
    numClusters       = 1;		/* Amount of cluster centers */
    threshold         = 0.001; 	/* Percentage of objects that need to change membership for the clusting to continue */
    isBinaryFile      = 0;		/* 0 if the input file is in ASCII format, 1 for binary format */
    filename          = NULL;	/* Name of the input file */
    outfile           = NULL;

	/* Parse command line options */
    while ( (opt=getopt(argc,argv,"o:p:i:n:t:abd"))!= EOF) {
        switch (opt) {
            case 'i': filename=optarg;
                      break;
            case 'b': isBinaryFile = 1;
                      break;
            case 'n': numClusters = atoi(optarg);
                      break;
            case 'h': usage(argv[0]);
                      break;
            case 'o': outfile = optarg;
                      break;
            default: usage(argv[0]);
                      break;
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
    clusters = starss_kmeans(0, objects, numCoords, numObjs,
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
    printf("Input file:     %s\n", filename);
    printf("numObjs       = %d\n", numObjs);
    printf("numCoords     = %d\n", numCoords);
    printf("numClusters   = %d\n", numClusters);
    printf("threshold     = %.4f\n", threshold);

    printf("I/O time           = %10.4f sec\n", io_timing);
    printf("Computation timing = %10.4f sec\n", clustering_timing);

    return(0);
}

