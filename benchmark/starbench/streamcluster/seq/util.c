/*
*   File: util.c
*   ------------
*   Implements utility functions for dealing with data points.
*/

#include <stdlib.h>
#include "streamcluster.h"

// tells whether two points of D dimensions are identical
int isIdentical(float *i, float *j, int D)
{
    int a = 0;
    int equal = 1;

    while (equal && a < D) {
        if (i[a] != j[a]) equal = 0;
        else a++;
    }
    if (equal) return 1;
    else return 0;
}

/* comparator for floating point numbers */
static int floatcomp(const void *i, const void *j)
{
    float a, b;
    a = *(float *)(i);
    b = *(float *)(j);
    if (a > b) return (1);
    if (a < b) return (-1);
    return(0);
}

/* shuffle points into random order */
void shuffle(Points *points)
{
    long i, j;
    Point temp;
    for (i = 0;i < points->num-1; i++) {
        j = ( lrand48() % (points->num - i) ) + i;
        temp = points->p[i];
        points->p[i] = points->p[j];
        points->p[j] = temp;
    }
}

/* shuffle an array of integers */
void intshuffle(int *intarray, int length)
{
    long i, j;
    int temp;
    for (i = 0; i < length; i++) {
        j=(lrand48()%(length - i))+i;
        temp = intarray[i];
        intarray[i]=intarray[j];
        intarray[j]=temp;
    }
}

/* compute Euclidean distance squared between two points */
float dist(Point p1, Point p2, int dim)
{
    int i;
    float result=0.0;
    for (i=0;i<dim;i++)
        result += (p1.coord[i] - p2.coord[i])*(p1.coord[i] - p2.coord[i]);
    return(result);
}