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

/**********************************************************************************
				INCLUDES & DEFINES
*************************************************************************************/
#include <sys/time.h>
#include <stdlib.h>
#include "rotation_engine.h"

#define BAD_EXIT -1;
#define TIME(x) gettimeofday(&x,NULL)

typedef struct timeval timer;
using namespace std;

/**********************************************************************************
				FUNCTION PROTOTYPES
*************************************************************************************/
static long timevaldiff(timer* start, timer* finish);
bool parseArgs(char* argv[], int argc, unsigned int &angle, string &inname, string &outname, unsigned int &numthreads, bool& pinning);

/* GLOBAL VARIABLES */
string usage =  "Usage: ./rot <infile> <outfile> <angle> <threads> <pinning>\n\n"
                "infile:      input file\n"
                "outfile:     output file\n"
                "angle:       angle to be rotated\n"
                "threads:     number of threads\n"
                "pinning:     thread pinning \n";
string p_name = "--- StarBENCH - rotate Kernel ---\n";

timer a,b;

/*
*	Function: main
*	--------------
*	The program main function.
*/
int main(int argc, char* argv[]) {
    cout << p_name;

    if(argc != 6) {
		cerr << usage;
		return BAD_EXIT;
    }

    string srcfile, destfile;
    unsigned int angle, numthreads;
    bool pinning;
    timer start, finish;
    RotateEngine re;

    if(!parseArgs(argv, argc, angle, srcfile, destfile, numthreads, pinning)) {
        cerr << usage;
        return BAD_EXIT;
    }

    if(!re.init(srcfile, destfile, angle, numthreads, pinning)) return BAD_EXIT;

	//re.printRotationState();

	TIME(start);
    re.run();
	TIME(finish);

    re.finish();

    cout << "Conversion timing:" << endl;
    cout << "Compute: " << (double)timevaldiff(&a, &b)/1000 << "s" << endl;
    cout << "Total: " << (double)timevaldiff(&start, &finish)/1000 << "s" << endl;

    return 0;
}

/*
*   Function: convertToString
*   -------------------------
*   Converts the c-string program arguments into c++-strings and returns
*   a pointer to an array of such strings.
*/
string* convertToString(char** in, size_t size) {
    string* args = new string[size];
    for(size_t i = 0; i < size; i++) {
       args[i] = in[i];
    }
    return args;
}

/*
*   Function: parseArgs
*   -------------------
*   Extracts the rotation angle as well as the in- and output file names
*   from the string array args, storing them in the specified variables.
*/
bool parseArgs(char* argv[], int argc, unsigned int &angle, string &inname, string &outname, unsigned int &numthreads, bool& pinning) {
    if (argc!=6)
        return false;

    angle = atoi(argv[3]) % 360;
    numthreads = atoi(argv[4]);
    pinning = atoi(argv[5])!=0;

    if (angle < 0 || numthreads < 1) {
        cerr << "Bad arguments, exiting" << endl;
        exit(-1);
    }

    inname = argv[1];
    outname = argv[2];
    return true;
}

/*
*   Function: timevaldiff
*   ---------------------
*   Provides a millisecond-resolution timer, computing the elapsed time
*   in between the two given timeval structures.
*/
static long timevaldiff(timer* start, timer* finish){
	long msec;
	msec = (finish->tv_sec - start->tv_sec)*1000;
	msec += (finish->tv_usec - start->tv_usec)/1000;
	return msec;
}
