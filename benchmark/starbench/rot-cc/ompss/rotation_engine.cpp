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
#include <string.h>
#include <ctime>

#define PI M_PI
#define PRECISION 3
#define printPoint(a) printf("(%d,%d)\n",(int)a.x,(int)a.y)

using namespace std;

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

Image* RotateEngine::getOutput() {
    return &output;
}

Image* RotateEngine::getInput() {
    return &input;
}

int RotateEngine::gettargeth() {
    return target_h;
}

int RotateEngine::gettargetw() {
    return target_w;
}

unsigned int RotateEngine::getangle() {
    return angle;
}

/*
*	Function: init
*	------------------
*	Prepares the rotation core for running the kernel. Sets up needed
*   parameters and loads in the input image.
*/
bool RotateEngine::init(string srcname, string destname, unsigned int angle) {
    this->angle = angle;
    this->srcname = srcname;
    this->destname = destname;
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

    fprintf(stderr, "Width %d Height %d\n", input.getWidth(), input.getHeight());

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
    rotatePoint((float*)&ul, (float*)&c1, angle);
    rotatePoint((float*)&ur, (float*)&c2, angle);
    rotatePoint((float*)&ll, (float*)&c3, angle);
    rotatePoint((float*)&lr, (float*)&c4, angle);
    target_h = computeTargetHeight();
    target_w = computeTargetWidth();
    output.createImageFromTemplate(target_w, target_h, 3);

    return true;
}

/*
*   Function: computeRow
*   --------------------
*   Computes the <row>th image scanline of the target image. Rewritten as a separate function for threading.
*/
//#pragma omp task output(*dep)
void RotateEngine::computeRow(int row, int maxrow, int rowwidth, float xot, float yot, float xos, float yos, unsigned rev_angle, char* dep) {
    // Prevent compiler warnings
    *dep='a';

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

    if(row == maxrow-1)
        done = true;
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
		For each pixel in target image, do
			- backwards rotation to determine origin location
			- for origin location, sample and filter 4 closest neighbour pixels
			- write colour value appropriately
	*/
	unsigned int rev_angle = 360 - angle;
	float x_offset_target = (float)target_w/2.0;
	float y_offset_target = (float)target_h/2.0;

    for(int i = 0; i < target_h; i++) {
        for(int j = 0; j < target_w; j++) {
            /* Find origin pixel for current destination pixel */
            Coord cur = {-x_offset_target + (float)j, y_offset_target - (float)i};
            Coord origin_pix;
            rotatePoint((float*)&cur,(float*)&origin_pix,rev_angle);

            /* If original image contains point, sample colour and write back */
            if(input.containsPixel(&origin_pix)) {
                int samples[4][2];
                Pixel colors[4];

                /* Get sample positions */
                for(int k = 0; k < 4; k++) {
                    samples[k][0] = (int)(origin_pix.x + x_offset_source) + ((k == 2 || k == 3) ? 1 : 0);
                    samples[k][1] = (int)abs(origin_pix.y - y_offset_source) + ((k == 1 || k == 3) ? 1 : 0);
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
                output.setPixelAt(j, i, &final);
            } else {
                /* Pixel is not in source image, write black color */
                Pixel final = {0,0,0};
                output.setPixelAt(j, i, &final);
            }
        }
    }

	done = true;
}

/*
*   Function: finish
*   ----------------
*   Performs garbage collection, i.e. deletes dynamically allocated memory
*   and writes the output back to disk.
*/
void RotateEngine::finish() {
//     if(!writeOutImage()) fprintf(stderr, "Could Not Write Rotation Output\n");
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
inline void RotateEngine::filter(Pixel* colors, Pixel* dest, Coord* sample_pos) {
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
inline void RotateEngine::interpolateLinear(Pixel* a, Pixel* b, Pixel* dest, float weight) {
	dest->r = a->r * (1.0-weight) + b->r * weight;
	dest->g = a->g * (1.0-weight) + b->g * weight;
	dest->b = a->b * (1.0-weight) + b->b * weight;
}

