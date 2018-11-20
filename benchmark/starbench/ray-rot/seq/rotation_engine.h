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
***********************************************************************************/
#ifndef R_ENGINE_H
#define R_ENGINE_H

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <cmath>
#include <float.h>
#include "image.h"

using namespace std;

#define BLOCK 16

/*
*	Class: RotateEngine
*	-------------------
*   Container object for the benchmark. Contains benchmark state, i.e.
*   input and output images as well as additional kernel data.
*/
class RotateEngine {
    public:
		RotateEngine();
		void run();
		void finish();
		bool init(Image* input, unsigned int angle, string destname);
        void printRotationState();
    private:
        /* Variables */
        string destname;
		Image* input, *output;
		unsigned int angle;
        bool initialized, done;
		Coord ul, ur, ll, lr, c1, c2, c3, c4;
        /* Functions */
        bool writeOutImage();
		void rotatePoint(float *pt, float *target, unsigned int angle);
		double round(double num, int digits);
		int computeTargetHeight();
		int computeTargetWidth();
		float findMax(float* seq);
		float findMin(float* seq);
		void filter(Pixel* colors, Pixel* dest, Coord* sample_pos);
		void interpolateLinear(Pixel* a, Pixel* b, Pixel* dest, float weight);
};

#endif
