/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*   File:         wtime.c                                                   */
/*   Description:  a timer that reports the current wall time                */
/*                                                                           */
/*   Author:  Wei-keng Liao                                                  */
/*            ECE Department Northwestern University                         */
/*            email: wkliao@ece.northwestern.edu                             */
/*   Copyright, 2005, Wei-keng Liao                                          */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>

/*
*	Function: wtime
*	---------------
*	Provides a millisecond-resolution timer for measurement purposes.
*/
double wtime(void) 
{
    double          now_time;
    struct timeval  etstart;
    struct timezone tzp;

    if (gettimeofday(&etstart, &tzp) == -1) {
		fprintf(stderr, "Timing Error: Could Not Get Current Time\n");
		exit(EXIT_FAILURE);
	}

    now_time = ((double)etstart.tv_sec) +              /* in seconds */
               ((double)etstart.tv_usec) / 1000000.0;  /* in microseconds */
    return now_time;
}


