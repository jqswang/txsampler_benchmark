/*
*   File: util.h
*   ------------
*   Header file for utility function definitions.
*/

#ifndef __UTIL_H__
#define __UTIL_H__

int isIdentical(float *i, float *j, int D);

/* comparator for floating point numbers */
static int floatcomp(const void *i, const void *j);

/* shuffle points into random order */
void shuffle(Points *points);

/* shuffle an array of integers */
void intshuffle(int *intarray, int length);

/* compute Euclidean distance squared between two points */
float dist(Point p1, Point p2, int dim);

#endif // __CLUSTER_H__