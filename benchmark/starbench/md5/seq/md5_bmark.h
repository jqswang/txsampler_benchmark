/*
*   File: md5_bmark.h
*   -----------------
*   Structure definitions for the md5 kernel.
*/

#include <stdint.h>

#define DIGEST_SIZE 16

typedef struct md5bench {
    int input_set;
    int iterations;
    int numinputs;
    int size;
    int outflag;
    uint8_t** inputs;
    uint8_t* out;
} md5bench_t;

typedef struct {
    int numbufs;
    int bufsize;
    int rseed;
} data_t;