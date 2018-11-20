/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*   File:         file_io.c                                                 */
/*   Description:  This program reads point data from a file                 */
/*                 and write cluster output to files                         */
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
#include <fcntl.h>
#include <unistd.h>     /* read(), close() */

#include "kmeans.h"

#define MAX_CHAR_PER_LINE 128


/*
*	Function: file_read
*	-------------------
*	Function for loading input data into memory.
*/
double** file_read(int   isBinaryFile,  /* flag: 0 or 1 */
                  char *filename,      /* input file name */
                  int  *numObjs,       /* count of data objects (local) */
                  int  *numCoords)     /* count of coordinates */
{
    float **objects;
    int     i, j, len;
    ssize_t numBytesRead;

    if (isBinaryFile) {  /* input file is in raw binary format -------------*/
        int infile;
		fprintf(stderr, "Trying to read from binary file: %s", filename);
        if ((infile = open(filename, O_RDONLY, "0600")) == -1) {
            fprintf(stderr, "Error: Input File Not Found\n");
			exit(EXIT_FAILURE);
        }
        numBytesRead = read(infile, numObjs, sizeof(int));
        assert(numBytesRead == sizeof(int));
        numBytesRead = read(infile, numCoords, sizeof(int));
        assert(numBytesRead == sizeof(int));

        /* allocate space for objects[][] and read all objects */
        len = (*numObjs) * (*numCoords);
        objects    = (float**)malloc((*numObjs) * sizeof(float*));
        objects[0] = (float*) malloc(len * sizeof(float));

		if(objects == NULL || objects[0] == NULL) {
			fprintf(stderr, "Could Not Allocate Memory\n");
			exit(EXIT_FAILURE);
		}

        for (i = 1; i < (*numObjs); i++)
            objects[i] = objects[i-1] + (*numCoords);

        numBytesRead = read(infile, objects[0], len*sizeof(float));
        assert(numBytesRead == len*sizeof(float));
		fprintf(stderr, " ... Input read successfully!\n");
        close(infile);

    } else {  /* input file is in ASCII format -------------------------------*/
        FILE *infile;
        char *line, *ret;
        int   lineLen;

		fprintf(stderr, "Trying to read from ASCII file: %s", filename);
		if ((infile = fopen(filename, "r")) == NULL) {
            fprintf(stderr, "Error: Input File Not Found\n");
			exit(EXIT_FAILURE);
        }

        /* first find the number of objects */
        lineLen = MAX_CHAR_PER_LINE;
        line = (char*) malloc(lineLen);
        assert(line != NULL);

        (*numObjs) = 0;
        while (fgets(line, lineLen, infile) != NULL) {
            /* check each line to find the max line length */
            while (strlen(line) == lineLen-1) {
                /* this line read is not complete */
                len = strlen(line);
                fseek(infile, -len, SEEK_CUR);

                /* increase lineLen */
                lineLen += MAX_CHAR_PER_LINE;
                line = (char*) realloc(line, lineLen);
                assert(line != NULL);

                ret = fgets(line, lineLen, infile);
                assert(ret != NULL);
            }

            if (strtok(line, " \t\n") != 0)
                (*numObjs)++;
        }
        rewind(infile);

        /* find the no. objects of each object */
        (*numCoords) = 0;
        while (fgets(line, lineLen, infile) != NULL) {
            if (strtok(line, " \t\n") != 0) {
                /* ignore the id (first coordiinate): numCoords = 1; */
                while (strtok(NULL, " ,\t\n") != NULL) (*numCoords)++;
                break; /* this makes read from 1st object */
            }
        }
        rewind(infile);

        /* allocate space for objects[][] and read all objects */
        len = (*numObjs) * (*numCoords);
        objects    = (float**)malloc((*numObjs) * sizeof(float*));
        assert(objects != NULL);
        objects[0] = (float*) malloc(len * sizeof(float));
        assert(objects[0] != NULL);
        for (i=1; i<(*numObjs); i++)
            objects[i] = objects[i-1] + (*numCoords);

        i = 0;
        /* read all objects */
        while (fgets(line, lineLen, infile) != NULL) {
            if (strtok(line, " \t\n") == NULL) continue;
            for (j=0; j<(*numCoords); j++)
                objects[i][j] = atof(strtok(NULL, " ,\t\n"));
            i++;
        }
		fprintf(stderr, " ... Input read successfully!\n");
        fclose(infile);
        free(line);
    }


    double** objects_d = (double**)malloc((*numObjs) * sizeof(double*));
    objects_d[0] = (double*) malloc(len * sizeof(double));
    for (i = 1; i < (*numObjs); i++)
        objects_d[i] = objects_d[i-1] + (*numCoords);
    
    for (i=0; i< (*numObjs); i++){
        for (j=0; j<(*numCoords); j++){
            objects_d[i][j] = objects[i][j];
        }
    }
    free(objects[0]);
    free(objects);

    return objects_d;
}

