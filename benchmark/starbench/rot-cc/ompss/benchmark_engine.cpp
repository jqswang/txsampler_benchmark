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

#include "benchmark_engine.h"

/*
 *  Intitialization of the benchmarking object. Creates the control objects for the two kernels.
 */
bool BenchmarkEngine::init(string srcname, string destname, unsigned int angle) {
    re = new RotateEngine();
    ce = new ConvertEngine();
    if(!re->init(srcname, destname, angle))
        return false;
    if(!ce->init(re->getOutput(), destname.c_str()))
        return false;
    return true;
};

/* Wrappers for OmpSs tasking */

//#pragma omp task output(*dep)
void computeRowTask(int row, int height, int width, int xot, int yot, int xos, int yos, int ra, char* dep, RotateEngine* re) {
	re->computeRow(row, height, width, xot, yot, xos, yos, ra, dep);
}

//#pragma omp task input(*dep)
void convertLineTask(int line, char* dep, ConvertEngine *ce) {
	ce->convertLine(line, dep);
}

/*
 *  Manages the adding of tasks to the runtime taskgraph.
 */
void BenchmarkEngine::run() {
    Image* input = re->getInput();

    unsigned int height = input->getHeight();
    unsigned int width = input->getWidth();
    unsigned int depth = input->getDepth();
    float x_offset_source = (float)width / 2.0;
    float y_offset_source = (float)height / 2.0;
    unsigned int rev_angle = 360 - re->getangle();
    float x_offset_target = (float)re->gettargetw()/2.0;
    float y_offset_target = (float)re->gettargeth()/2.0;

    char* dependencies = new char[re->gettargeth()];
    
//===================================================================================================================================================================
    
    #pragma omp parallel
    {
      #pragma omp single nowait 
      {
	  for(int i = 0; i < re->gettargeth(); i++) {
	      #pragma omp task 
	      {
	      computeRowTask(i, re->gettargeth(), re->gettargetw(), x_offset_target, y_offset_target, x_offset_source, y_offset_source, rev_angle, &dependencies[i], re);
	      convertLineTask(i, &dependencies[i], ce);
	      }//end task
	  }
      }//end single
    }//end parallel
    
//===================================================================================================================================================================

    delete[] dependencies;
}

/*
 *  Cleans up all memory used during the benchmark.
 */
void BenchmarkEngine::finish() {
    re->finish();
    ce->finish();
    delete re;
    delete ce;
}