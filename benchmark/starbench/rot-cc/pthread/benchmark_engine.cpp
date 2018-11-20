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

#include <sched.h>
#include <sys/syscall.h>
#include <errno.h>
#include <unistd.h>
#include "benchmark_engine.h"

extern unsigned int numthreads;
extern bool pinning;

/*
 *  Thread function for parallel processing of both kernels. Uses dynamic load balancing.
 */
void* thread_func(void* arg) {
    threadarg_t* args = (threadarg_t*)arg;
    RotateEngine* re = args->re;
    ConvertEngine* ce = args->ce;
    int rotate_nextline, convert_nextline;

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

    while((rotate_nextline = __sync_fetch_and_add(args->global_sl, 1)) < args->height) {
        re->computeRow(rotate_nextline, args->height, args->width, args->x_offset_target, args->y_offset_target, args->x_offset_source, args->y_offset_source, args->rev_angle);
        ce->convertLine(rotate_nextline);
    }
    return NULL;
}

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

/*
 *  Manages the startup and shutdown of threads.
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

    pthread_t* threads = new pthread_t[numthreads];
    threadarg_t* args = new threadarg_t[numthreads];

    int global_sl = 0;

    for(int i = 0; i < numthreads; i++) {
        args[i].re = re;
        args[i].ce = ce;
        args[i].global_sl = &global_sl;
        args[i].rev_angle = rev_angle;
        args[i].x_offset_source = x_offset_source;
        args[i].y_offset_source = y_offset_source;
        args[i].x_offset_target = x_offset_target;
        args[i].y_offset_target = y_offset_target;
        args[i].width = re->gettargetw();
        args[i].height = re->gettargeth();
        args[i].tid = i;
        pthread_create(&threads[i], NULL, &thread_func, &args[i]);
    }

    for(int i = 0; i < numthreads; i++) {
        pthread_join(threads[i], NULL);
    }

    delete[] threads;
    delete[] args;
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