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
#include <unistd.h>
#include <pthread.h>
#include <sched.h>
#include <sys/syscall.h>
#include <errno.h>
#include <unistd.h>

#include <sys/time.h>
#include <math.h>

#include "rgbyuv.h"
#include "imgio.h"

typedef struct timeval timer;

#define TIME(x) gettimeofday(&x, NULL)

/*
*   This program is a benchmark kernel written as part of the StarBench benchmark suite.
*   It is a fully parallelized RGB to YUV color converter, processing .ppm format pictures.
*   This application is a common one for embedded domain multiprocessors such as DSPs.
*/

/* Function declarations */
int initialize(rgbyuv_args_t* a);
int finalize(rgbyuv_args_t* a);
void statsme();
long timediff(timer* starttime, timer* finishtime);
int processImage(rgbyuv_args_t* args);
void* convertThread(void* arg);
int writeComponents(rgbyuv_args_t* args);

/* Global data */
static char* usage =    "Usage: %s <options>\n"
                        "-i infile          give input file path\n"
                        "-c iterations      specify number of iterations\n"
                        "-t threads         specify number of threads\n"
                        "-p <0|1>           pin threads\n"
                        "-h                 this help text\n";;
char* infile = NULL;
int numthreads = 1;
int pinning = 0;
int iterations = 1;

/* For threading */
pthread_t* threads;
threadarg_t* threadargs;
pthread_mutex_t flag = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

/* For profiling */
timer launch, compute, disk;

/*
*   Function: initialize
*   --------------------
*   Load image information and color data and allocate output buffer.
*   Thread setup is NOT performed here in order to get accurate threading
*   overhead timing.
*/
int initialize(rgbyuv_args_t* a) {
    int maxcolor = 0;
    int depth = 0;
    int width = 0;
    int height = 0;
    // Get the image from disk
    a->in_img = loadPPMFile(infile, &width, &height, &maxcolor, &depth);

    // Check for correct image type
    if (depth != IN_DEPTH)
        exit(-1);

    // Compute image params
    a->width = width;
    a->height = height;
    a->pixels = a->height * a->width;
    a->pY = calloc(a->pixels, sizeof(uint8_t));
    a->pU = calloc(a->pixels, sizeof(uint8_t));
    a->pV = calloc(a->pixels, sizeof(uint8_t));

    // Set up threading
    threads = calloc(numthreads, sizeof(pthread_t));
    threadargs = calloc(numthreads, sizeof(rgbyuv_args_t));

    if(a->in_img == NULL || a->pY == NULL || a->pU == NULL || a->pV == NULL || threads == NULL || threadargs == NULL)
        return -1;

    int rows_divided = a->height/ numthreads;
    int rows_remaining = a->height % numthreads;
    int ptp = rows_divided * a->width;

    // Fill threadargs so threads operate on non-overlapping memory regions
    for(int i = 0; i < numthreads; i++) {
        threadargs[i].in = a->in_img + i * rows_divided * a->width * IN_DEPTH;
        threadargs[i].pY = a->pY + i * rows_divided * a->width;
        threadargs[i].pU = a->pU + i * rows_divided * a->width;
        threadargs[i].pV = a->pV + i * rows_divided * a->width;
        threadargs[i].pixels_to_process = ptp + (i==numthreads-1 ? rows_remaining * a->width : 0);
        threadargs[i].tid = i;
    }
    return 0;
}

/*
*   Function: finalize
*   ------------------
*   Releases memory after the processing is finished.
*/
int finalize(rgbyuv_args_t* a) {
    if(a->in_img)
        free(a->in_img);
    if(a->pY)
        free(a->pY);
    if(a->pU)
        free(a->pU);
    if(a->pY)
        free(a->pV);
    if(threads)
        free(threads);
    if(threadargs)
        free(threadargs);
    return 0;
}

/*
*   Function: statsme
*   -----------------
*   Print out input file which will be read from, thread count, and specified iterations.
*/
void statsme() {
    fprintf(stderr, "\nInput file: %s\nThreads: %d\nIterations: %d\n", infile, numthreads, iterations);
}


/*
*   Function: timediff
*   ------------------
*   Compute the difference between timers starttime and finishtime in msecs.
*/
long timediff(timer* starttime, timer* finishtime)
{
    long msec;
    msec=(finishtime->tv_sec-starttime->tv_sec)*1000;
    msec+=(finishtime->tv_usec-starttime->tv_usec)/1000;
    return msec;
}

/*
*   Function: convertThread
*   -----------------------
*   Thread which converts a block of pixels_to_process pixels from RGB to YUV.
*/
void* convertThread(void* arg) {
    threadarg_t* args = (threadarg_t*)arg;

    if (pinning){
      cpu_set_t mask;
      CPU_ZERO(&mask);
      CPU_SET(args->tid, &mask);
      pid_t tid = syscall(SYS_gettid); //glibc does not provide a wrapper for gettid
      int err= sched_setaffinity(tid, sizeof(cpu_set_t), &mask);
      if (err){
        fprintf(stderr, "failed to set core affinity for thread %d\n", args->tid);
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

    uint8_t R,G,B,Y,U,V;
    uint8_t* in = args->in;
    uint8_t* pY = args->pY;
    uint8_t* pU = args->pU;
    uint8_t* pV = args->pV;

    for(int i = 0; i < args->pixels_to_process; i++) {
        R = *in++;
        G = *in++;
        B = *in++;

        Y = round(0.256788*R+0.504129*G+0.097906*B) + 16;
        U = round(-0.148223*R-0.290993*G+0.439216*B) + 128;
        V = round(0.439216*R-0.367788*G-0.071427*B) + 128;

        *pY++ = Y;
        *pU++ = U;
        *pV++ = V;
    }
}

/*
*   Function: processImage
*   ----------------------
*   Called to perform the actual image conversion and disk output. Controls iterations.
*/
int processImage(rgbyuv_args_t* args) {

#ifdef VERBOSE
    fprintf(stderr, "Beginning conversion ... \n");
#endif
    for(int i = 0; i < iterations; i++) {

        for(int j = 0; j < numthreads; j++) {
            pthread_create(&(threads[j]), NULL, &convertThread, &threadargs[j]);
        }

        for(int j = 0; j < numthreads; j++) {
            pthread_join(threads[j], NULL);
        }

#ifdef VERBOSE
        fprintf(stderr, "Finished conversion, writing to disk ... \n");
#endif

    }
    return 0;
}

/*
*   Function: writeComponents
*   -------------------------
*   Receives the buffer containing the converted image and separately writes Y, U and V component
*   to a disk image file. Also writes YUV image as an RGB image to disk.
*/
int writeComponents(rgbyuv_args_t* args) {

    FILE* fp;
    char yuvheader[256];
    char planeheader[256];
    char* targets[] = { "ycomp.ppm", "ucomp.ppm", "vcomp.ppm" };
    uint8_t* pY = args->pY;
    uint8_t* pU = args->pU;
    uint8_t* pV = args->pV;
    int pels = args->pixels;

    // Write headers
    snprintf(yuvheader, (size_t)255, "P6\n%d %d\n%d\n", args->width, args->height, MAXCOLOR);
    snprintf(planeheader, (size_t)255, "P5\n%d %d\n%d\n", args->width, args->height, MAXCOLOR);
#ifdef VERBOSE
    fprintf(stderr, "Created .ppm header: \n%s\n\n%s\n", yuvheader, planeheader);
#endif

    // Y component
#ifdef VERBOSE
    fprintf(stderr, "Writing to file %s\n", targets[0]);
#endif
    fp = fopen(targets[0], "w");
    fprintf(fp, "%s", planeheader);
    fwrite(pY, sizeof(uint8_t), pels, fp);
    fclose(fp);

    // For U component

#ifdef VERBOSE
    fprintf(stderr, "Writing to file %s\n", targets[1]);
#endif
    fp = fopen(targets[1], "w");

    if (fp == NULL)
        return -1;

    fprintf(fp, "%s", planeheader);
    fwrite(pU, sizeof(uint8_t), pels, fp);
    fclose(fp);

    // For V component

#ifdef VERBOSE
    fprintf(stderr, "Writing to file %s\n", targets[1]);
#endif
    fp = fopen(targets[2], "w");

    if (fp == NULL)
        return -1;

    fprintf(fp, "%s", planeheader);
    fwrite(pV, sizeof(uint8_t), pels, fp);
    fclose(fp);

    return 0;
}

/** MAIN */
int main(int argc, char** argv) {

    int opt;
    extern char* optarg;
    extern int optind;

    timer io_start, b_start, b_end;

    // Who we are
    fprintf(stderr, "StarBench - RGBYUV Kernel\n");

    // Parse command line options
    while ( (opt=getopt(argc,argv,"i:t:c:p:h")) != EOF) {
        switch (opt) {
            case 'i':
                infile = optarg;
                break;
            case 'c':
                iterations = atoi(optarg);
                break;
            case 't':
                numthreads = atoi(optarg);
                break;
            case 'p':
                pinning = atoi(optarg)!=0;
                break;
            case 'h':
                fprintf(stderr, usage, argv[0]);
                return 0;
                break;
            default:
                fprintf(stderr, usage, argv[0]);
                return -1;
                break;
        }
    }

    if (infile == NULL || numthreads < 1 || iterations < 0) {
        fprintf(stderr, usage, argv[0]);
        return -1;
    }

#ifdef VERBOSE
    statsme();
#endif

    rgbyuv_args_t args;

    TIME(io_start);

    if(initialize(&args)) {
        fprintf(stderr, "Could Not Initialize Kernel Data\n");
        return -1;
    }

    TIME(b_start);

    processImage(&args);

    TIME(b_end);

    writeComponents(&args);

    if(finalize(&args)) {
        fprintf(stderr, "Could Not Free Allocated Memory\n");
        return -1;
    }

    double io_time = (double)timediff(&io_start, &b_start)/1000;
    double b_time = (double)timediff(&b_start, &b_end)/1000;

    printf("\nI/O time: %.3f\nTime: %.3f\n", io_time, b_time);

    return 0;
}
