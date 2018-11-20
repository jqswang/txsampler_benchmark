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


/* INCLUDES */
#include "rotation_engine.h"
#include <pthread.h>
#include <string.h>
#include <sched.h>
#include <sys/syscall.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <ctime>

#define PI M_PI
#define PRECISION 3
#define printPoint(a) printf("(%d,%d)\n",(int)a.x,(int)a.y)
#define TIME(x) gettimeofday(&x,NULL)

using namespace std;

typedef struct timeval timer;

extern timer a, b;

/**********************************************************************************
				PUBLIC FUNCTIONS
***********************************************************************************/

/*
*	Function: Constructor
*	---------------------
*	Performs initial state setup.
*/
RotateEngine::RotateEngine() {
	done = false;
	initialized = false;
}

/*
*	Function: init
*	------------------
*	Prepares the rotation core for running the kernel. Sets up needed
*   parameters and loads in the input image.
*/
bool RotateEngine::init(string srcname, string destname, unsigned int angle, unsigned int numthreads, int pinning) {
    this->angle = angle;
    this->srcname = srcname;
    this->destname = destname;
    this->numthreads = numthreads;
    this->pinning = pinning;
    sl = 0;
	c1.x = 0.0;
	c1.y = 0.0;
	c2.x = 0.0;
	c2.y = 0.0;
	c3.x = 0.0;
	c3.y = 0.0;
	c4.x = 0.0;
	c4.y = 0.0;
	cout << "Trying to open image file " << srcname << " ... ";
    if(!input.createImageFromFile(srcname.c_str())) {
        cout << "failed." << endl;
        return false;
    } else
        cout << "OK." << endl;

	// Fill input image corner coordinates
	float xc = (float)input.getWidth()/2.0;
	float yc = (float)input.getHeight()/2.0;
	ul.x = -xc;
	ul.y = yc;
	ur.x = xc;
	ur.y = yc;
	ll.x = -xc;
	ll.y = -yc;
	lr.x = xc;
	lr.y = -yc;
	initialized = true;
    return true;
}

/*
*   Function: rethread
*   ------------------
*   Rotate Engine thread. Computes several scanlines of the target image.
*/
void* rethread(void* args) {
    threadarg_t* arg = (threadarg_t*)args;

    RotateEngine* re = arg->re;

    int local_sl;

    if (arg->pin_thread){
      cpu_set_t mask;
      CPU_ZERO(&mask);
      CPU_SET(arg->tid, &mask);
      pid_t tid = syscall(SYS_gettid); //glibc does not provide a wrapper for gettid
      int err= sched_setaffinity(tid, sizeof(cpu_set_t), &mask);
      if (err){
        fprintf(stderr, "failed to set core affinity for thread %d\n", arg->tid);
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

    while((local_sl = __sync_fetch_and_add(&(re->sl), BLOCK)) < arg->imagerows){
        for(int i = local_sl; i < local_sl + BLOCK && i < arg->imagerows; i++) {
            re->computeRow(i, arg->rowwidth, arg->x_offset_target, arg->y_offset_target, arg->x_offset_source, arg->y_offset_source, arg->rev_angle);
        }
    }
}

/*
*   Function: computeRow
*   --------------------
*   Computes the <row>th image scanline of the target image. Rewritten as a separate function for threading.
*/
void RotateEngine::computeRow(int row, int rowwidth, float xot, float yot, float xos, float yos, unsigned rev_angle) {

    for(int j = 0; j < rowwidth; j++) {
        /* Find origin pixel for current destination pixel */
        Coord cur = {-xot + (float)j, yot - (float)row};
        Coord origin_pix;
        rotatePoint((float*)&cur,(float*)&origin_pix,rev_angle);

        /* If original image contains point, sample colour and write back */
        if(input.containsPixel(&origin_pix)) {
            int samples[4][2];
            Pixel colors[4];

            /* Get sample positions */
            for(int k = 0; k < 4; k++) {
                samples[k][0] = (int)(origin_pix.x + xos) + ((k == 2 || k == 3) ? 1 : 0);
                samples[k][1] = (int)abs(origin_pix.y - yos) + ((k == 1 || k == 3) ? 1 : 0);
				// Must make sure sample positions are still valid image pixels
                if(samples[k][0] >= input.getWidth())
                    samples[k][0] = input.getWidth()-1;
                if(samples[k][1] >= input.getHeight())
                    samples[k][1] = input.getHeight()-1;
            }

            /* Get colors for samples */
            for(int k = 0; k < 4; k++) {
                colors[k] = input.getPixelAt(samples[k][0], samples[k][1]);
            }

            /* Filter colors */
            Pixel final;
            filter(colors, &final, &origin_pix);

            /* Write output */
            output.setPixelAt(j, row, &final);
        } else {
            /* Pixel is not in source image, write black color */
            Pixel final = {0,0,0};
            output.setPixelAt(j, row, &final);
        }
    }
}

/*
*   Function: run
*   -------------
*   Runs the benchmark kernel. When completed successfully, done will be set to true
*   and output will contain the output image.
*/
void RotateEngine::run() {
	if(!initialized) {
		fprintf(stderr, "Kernel Called Without Initialization\n");
		return;
	}
    if(done) {
        fprintf(stderr, "Kernel Already Executed\n");
        return;
    }

	unsigned int height = input.getHeight();
	unsigned int width = input.getWidth();
	unsigned int depth = input.getDepth();

	float x_offset_source = (float)width / 2.0;
	float y_offset_source = (float)height / 2.0;

	/* Steps for rotation:
		1. Determine target image size by rotating corners
		2. For each pixel in target image, do
			- backwards rotation to determine origin location
			- for origin location, sample and filter 4 closest neighbour pixels
			- write colour value appropriately
	*/

	/* STEP 1 */
	rotatePoint((float*)&ul, (float*)&c1, angle);
	rotatePoint((float*)&ur, (float*)&c2, angle);
	rotatePoint((float*)&ll, (float*)&c3, angle);
	rotatePoint((float*)&lr, (float*)&c4, angle);
	int target_h = computeTargetHeight();
	int target_w = computeTargetWidth();
    output.createImageFromTemplate(target_w, target_h, depth);

	/* STEP 2 */
    TIME(a);

	unsigned int rev_angle = 360 - angle;
	float x_offset_target = (float)target_w/2.0;
	float y_offset_target = (float)target_h/2.0;

    pthread_t* thread = new pthread_t[numthreads];
    threadarg_t* args = new threadarg_t[numthreads];
    unsigned int rows_per_thread = target_h / numthreads;
    unsigned int rows_remain = target_h % numthreads;
    unsigned int nextrow = 0;

    for(int i = 0; i < numthreads; i++) {
        args[i].re = this;
        args[i].rownum = nextrow;
        nextrow += rows_per_thread;
        args[i].rowcnt = rows_per_thread + (i == numthreads-1 ? rows_remain : 0);
        args[i].rowwidth = target_w;
        args[i].imagerows = target_h;
        args[i].pin_thread = pinning;
        args[i].tid = i;
        args[i].rev_angle = rev_angle;
        args[i].x_offset_source = x_offset_source;
        args[i].y_offset_source = y_offset_source;
        args[i].x_offset_target = x_offset_target;
        args[i].y_offset_target = y_offset_target;

        pthread_create(&thread[i], NULL, rethread, &args[i]);
    }

    for(int i = 0; i < numthreads; i++) {
        pthread_join(thread[i], NULL);
    }

    delete [] thread;
    delete [] args;
    TIME(b);

	done = true;
}

static long timevaldiff(struct timeval* start, struct timeval* finish){
    long msec;
    msec = (finish->tv_sec - start->tv_sec)*1000;
    msec += (finish->tv_usec - start->tv_usec)/1000;
    return msec;
}

/*
*   Function: finish
*   ----------------
*   Performs garbage collection, i.e. deletes dynamically allocated memory
*   and writes the output back to disk.
*/
void RotateEngine::finish() {
    if(!writeOutImage()) fprintf(stderr, "Could Not Write Rotation Output\n");
	input.clean();
	output.clean();
}

/*
*   Function: printRotationState
*   ----------------------------
*   Prints the current information contained in the RotateEngine object
*   on the console.
*/
void RotateEngine::printRotationState() {
    fprintf(stdout, "_____ Kernel State _____\n");
	fprintf(stdout, "Width: %d\t Height: %d\n", input.getWidth(), input.getHeight());
	fprintf(stdout, "Pixels: %.2fM\t Angle: %d°\n", (double)(input.getWidth()*input.getHeight())/1000000.0, (int)angle);
	fprintf(stdout, "Source file: %s\t Dest. File: %s\n", srcname.c_str(), destname.c_str());
}

/**********************************************************************************
				PRIVATE FUNCTIONS
***********************************************************************************/

/*
*	Function: writeOutImage
*	-----------------------
*	Writes output image of the kernel back to disk. Returns false if
*	called before an output image is produced. Returns true if writing back
*	the image was successful.
*/
bool RotateEngine::writeOutImage() {
	if(!done)
		return false;
    fstream out;
    out.open(destname.c_str(),fstream::out);
    if(!out.is_open()) {
        return false;
	}
    if(output.getDepth() == 3) {
        out << "P6\n";
    } else {
        return false;
	}
    out << output.getWidth() << " " << output.getHeight() << "\n" << output.getMaxcolor() << "\n";
    for(int i = 0; i < (int)output.getHeight(); i++) {
        for(int j = 0; j < (int)output.getWidth(); j++) {
            Pixel p = output.getPixelAt(j, i);
			out.put(p.r);
			out.put(p.g);
			out.put(p.b);
        }
    }
	out.close();
    return true;
}

/*
*	Function: rotatePoint
*	---------------------
*	Takes a point in 2d space and rotates it by angle.
*/
inline void RotateEngine::rotatePoint(float *pt, float* target, unsigned int angle) {
	float rad = (float)angle/180 * PI;
	((Coord*)target)->x = ((Coord*)pt)->x * cos(rad) - ((Coord*)pt)->y * sin(rad);
	((Coord*)target)->y = ((Coord*)pt)->x * sin(rad) + ((Coord*)pt)->y * cos(rad);
}

/*
*	Function: round
*	---------------
*	Simple, optimized function for rounding fp numbers.
*/
double RotateEngine::round(double num, int digits) {
    double v[] = {1, 10, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8};
	if(digits > (sizeof(v)/sizeof(double))) return num;
    return floor(num * v[digits] + 0.5) / v[digits];
}

/*
*	Function: computeTargetHeight
*	-----------------------
*	Computes the height of the output image.
*/
int RotateEngine::computeTargetHeight() {
	float seq[4] = {c1.y, c2.y, c3.y, c4.y};
	float max = findMax(seq);
	float min = findMin(seq);
	int height = (int)(max - min);
	return height;
}

/*
*	Function: computeTargetWidth
*	-----------------------
*	Computes the width of the output image.
*/
int RotateEngine::computeTargetWidth() {
	float seq[4] = {c1.x, c2.x, c3.x, c4.x};
	float max = findMax(seq);
	float min = findMin(seq);
	int width = (int)(max - min);
	return width;
}

/*
*	Function: findMax
*	-----------------
*	Find the maximum number in a 4-element sequence.
*/
inline float RotateEngine::findMax(float* seq) {
	float max = FLT_MIN;
	for(int i = 0; i < 4; i++) {
		if(seq[i] > max)
			max = seq[i];
	}
	return max;
}

/*
*	Function: findMin
*	-----------------
*	Find the minimum number in a 4-element sequence.
*/
inline float RotateEngine::findMin(float* seq) {
	float min = FLT_MAX;
	for(int i = 0; i < 4; i++) {
		if(seq[i] < min)
			min = seq[i];
	}
	return min;

}

/*
*	Function: filter
*	---------------------
*	Filters a given array of pixel colours of length 4, blending
*	color values into a final pixel. The algorithm used is bilinear
*	filtering, using the sample position as a weight for color blend.
*/
void RotateEngine::filter(Pixel* colors, Pixel* dest, Coord* sample_pos) {
	uint32_t r, g, b;
	Pixel sample_v_upper, sample_v_lower;
	float x_weight = round(sample_pos->x - floor(sample_pos->x), PRECISION);
	float y_weight = round(sample_pos->y - floor(sample_pos->y), PRECISION);

	interpolateLinear(&colors[0], &colors[3], &sample_v_upper, x_weight);
	interpolateLinear(&colors[1], &colors[2], &sample_v_lower, x_weight);
	interpolateLinear(&sample_v_upper, &sample_v_lower, dest, y_weight);
}

/*
*	Function: interpolateLinear
*	---------------------------
*	Linearly interpolates two pixel colors according to a given weight factor.
*/
void RotateEngine::interpolateLinear(Pixel* a, Pixel* b, Pixel* dest, float weight) {
	dest->r = a->r * (1.0-weight) + b->r * weight;
	dest->g = a->g * (1.0-weight) + b->g * weight;
	dest->b = a->b * (1.0-weight) + b->b * weight;
}

