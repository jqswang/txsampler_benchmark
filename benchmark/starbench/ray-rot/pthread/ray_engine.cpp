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

#include "ray_engine.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>
#include <sys/syscall.h>
#include <errno.h>
#include <unistd.h>
/*
 *  Initializes the RayEngine object for kernel execution. Loads the object list
 *  from disk into memory, fills jitter tables and sets global raytracing params.
 */
bool RayEngine::init(string srcname, int xres, int yres, int rpp, unsigned int numthreads, bool pinning) {
    int i;
    lnum = 0;
    for(i=0; i<NRAN; i++) urand[i].x = (double)rand() / RAND_MAX - 0.5;
    for(i=0; i<NRAN; i++) urand[i].y = (double)rand() / RAND_MAX - 0.5;
    for(i=0; i<NRAN; i++) irand[i] = (int)(NRAN * ((double)rand() / RAND_MAX));
    cerr << "Loading from file " << srcname << endl;
    FILE* fp = fopen(srcname.c_str(), "r");
    if(!fp)
        return false;
    load_scene(fp);
    fclose(fp);

    this->xres = xres;
    this->yres = yres;
    this->rays_per_pixel = rpp;
    this->numthreads = numthreads;
    this->pinning = pinning;
    aspect = (double)xres / (double)yres;

    out.createImageFromTemplate(xres, yres, 3);
    return true;
};

/*
 *  Thread for ray tracing. Fetches a block of image scanlines from a global counter
 *  and processes those independently.
 */
void* rtthread(void* arg) {
    struct thread_data* data = (struct thread_data*)arg;
    RayEngine* re = data->re;

    if (re->pinning){
      cpu_set_t mask;
      CPU_ZERO(&mask);
      CPU_SET(data->tid, &mask);
      pid_t tid = syscall(SYS_gettid); //glibc does not provide a wrapper for gettid
      int err= sched_setaffinity(tid, sizeof(cpu_set_t), &mask);
      if (err){
        fprintf(stderr, "failed to set core affinity for thread %d\n", data->tid);
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


    int sl;
    int numsl;
    int yres = re->getyres();
    int xres = re->getxres();
    while((sl = __sync_fetch_and_add(data->global_sl, SL_BLOCK)) < yres) {
        if(yres - sl < SL_BLOCK)
            numsl = yres - sl;
        else
            numsl = SL_BLOCK;
        for(int i = 0; i < numsl; i++) {
            re->render_scanline(xres, yres, sl++, re->getrpp());
        }
    }
}

/*
 *  Renders an image from the object list scene description linewise.
 */
void RayEngine::run() {
    pthread_t* threads = new pthread_t[numthreads];
    struct thread_data* args = new struct thread_data[numthreads];
    int global_sl = 0;

    for(int i = 0; i < numthreads; i++) {
        args[i].global_sl = &global_sl;
        args[i].re = this;
        args[i].tid = i;
        pthread_create(&threads[i], NULL, &rtthread, &args[i]);
    }

    for(int i = 0; i < numthreads; i++) {
        pthread_join(threads[i], NULL);
    }

    delete[] threads;
    delete[] args;
}

/*
 *  Frees associated buffers and memory after execution is finished.
 *  Careful: This deletes the content of the output image.
 */
void RayEngine::finish() {
    out.clean();
    struct sphere *walker = obj_list;
    while(walker) {
        struct sphere *tmp = walker;
        walker = walker->next;
        delete tmp;
    }
};

/* Some getters for threading */

int RayEngine::getxres() {
    return xres;
}

int RayEngine::getyres() {
    return yres;
}

int RayEngine::getrpp() {
    return rays_per_pixel;
}

/*
 *  Returns a pointer to the output image of the raytracer.
 */
Image* RayEngine::getOutputImage() {
    return &out;
}

/*
*   Function: printRaytracingState
*   ----------------------------
*   Prints the current information contained in the RayEngine object
*   on the console.
*/
void RayEngine::printRaytracingState() {
    fprintf(stdout, "RayTracing Kernel:\n");
    fprintf(stdout, "Width: %d\t Height: %d\n", xres, yres);
    fprintf(stdout, "Aspect Ratio: %.1f\t RPP: %d\n", aspect, rays_per_pixel);
    fprintf(stdout, "Number of lights: %d\n\n", lnum);
}

/*  =========================== *
 *  ==== PRIVATE FUNCTIONS ==== *
 *  =========================== */

void RayEngine::render_scanline(int xsz, int ysz, int sl, int samples) {
        int i, s;
        double rcp_samples = 1.0 / (double)samples;
        Pixel p;

        for(i=0; i<xsz; i++) {
            double r, g, b;
            r = g = b = 0.0;

            for(s=0; s<samples; s++) {
                struct vec3 col = trace(get_primary_ray(i, sl, s), 0);
                r += col.x;
                g += col.y;
                b += col.z;
            }

            r = r * rcp_samples;
            g = g * rcp_samples;
            b = b * rcp_samples;
            p.r = MIN(r,1.0) * 255.0;
            p.g = MIN(g,1.0) * 255.0;
            p.b = MIN(b,1.0) * 255.0;
            out.setPixelAt(i, sl, &p);
        }
}

/* trace a ray throught the scene recursively (the recursion happens through
 * shade() to calculate reflection rays if necessary).
 */
struct vec3 RayEngine::trace(struct ray ray, int depth) {
    struct vec3 col;
    struct spoint sp, nearest_sp;
    struct sphere *nearest_obj = 0;
    struct sphere *iter = obj_list->next;

    /* if we reached the recursion limit, bail out */
    if(depth >= MAX_RAY_DEPTH) {
        col.x = col.y = col.z = 0.0;
        return col;
    }

    /* find the nearest intersection ... */
    while(iter) {
        if(ray_sphere(iter, ray, &sp)) {
            if(!nearest_obj || sp.dist < nearest_sp.dist) {
                nearest_obj = iter;
                nearest_sp = sp;
            }
        }
        iter = iter->next;
    }

    /* and perform shading calculations as needed by calling shade() */
    if(nearest_obj) {
        col = shade(nearest_obj, &nearest_sp, depth);
    } else {
        col.x = col.y = col.z = 0.0;
    }

    return col;
}

/* Calculates direct illumination with the phong reflectance model.
 * Also handles reflections by calling trace again, if necessary.
 */
struct vec3 RayEngine::shade(struct sphere *obj, struct spoint *sp, int depth) {
    int i;
    struct vec3 col = {0, 0, 0};

    /* for all lights ... */
    for(i=0; i<lnum; i++) {
        double ispec, idiff;
        struct vec3 ldir;
        struct ray shadow_ray;
        struct sphere *iter = obj_list->next;
        int in_shadow = 0;

        ldir.x = lights[i].x - sp->pos.x;
        ldir.y = lights[i].y - sp->pos.y;
        ldir.z = lights[i].z - sp->pos.z;

        shadow_ray.orig = sp->pos;
        shadow_ray.dir = ldir;

        /* shoot shadow rays to determine if we have a line of sight with the light */
        while(iter) {
            if(ray_sphere(iter, shadow_ray, 0)) {
                in_shadow = 1;
                break;
            }
            iter = iter->next;
        }

        /* and if we're not in shadow, calculate direct illumination with the phong model. */
        if(!in_shadow) {
            NORMALIZE(ldir);

            idiff = MAX(DOT(sp->normal, ldir), 0.0);
            ispec = obj->mat.spow > 0.0 ? pow(MAX(DOT(sp->vref, ldir), 0.0), obj->mat.spow) : 0.0;

            col.x += idiff * obj->mat.col.x + ispec;
            col.y += idiff * obj->mat.col.y + ispec;
            col.z += idiff * obj->mat.col.z + ispec;
        }
    }

    /* Also, if the object is reflective, spawn a reflection ray, and call trace()
     * to calculate the light arriving from the mirror direction.
     */
    if(obj->mat.refl > 0.0) {
        struct ray ray;
        struct vec3 rcol;

        ray.orig = sp->pos;
        ray.dir = sp->vref;
        ray.dir.x *= RAY_MAG;
        ray.dir.y *= RAY_MAG;
        ray.dir.z *= RAY_MAG;

        rcol = trace(ray, depth + 1);
        col.x += rcol.x * obj->mat.refl;
        col.y += rcol.y * obj->mat.refl;
        col.z += rcol.z * obj->mat.refl;
    }

    return col;
}

/* calculate reflection vector */
struct vec3 RayEngine::reflect(struct vec3 v, struct vec3 n) {
    struct vec3 res;
    double dot = v.x * n.x + v.y * n.y + v.z * n.z;
    res.x = -(2.0 * dot * n.x - v.x);
    res.y = -(2.0 * dot * n.y - v.y);
    res.z = -(2.0 * dot * n.z - v.z);
    return res;
}

struct vec3 RayEngine::cross_product(struct vec3 v1, struct vec3 v2) {
    struct vec3 res;
    res.x = v1.y * v2.z - v1.z * v2.y;
    res.y = v1.z * v2.x - v1.x * v2.z;
    res.z = v1.x * v2.y - v1.y * v2.x;
    return res;
}

/* determine the primary ray corresponding to the specified pixel (x, y) */
struct ray RayEngine::get_primary_ray(int x, int y, int sample) {
    struct ray ray;
    float m[3][3];
    struct vec3 i, j = {0, 1, 0}, k, dir, orig, foo;

    k.x = cam.targ.x - cam.pos.x;
    k.y = cam.targ.y - cam.pos.y;
    k.z = cam.targ.z - cam.pos.z;
    NORMALIZE(k);

    i = cross_product(j, k);
    j = cross_product(k, i);
    m[0][0] = i.x; m[0][1] = j.x; m[0][2] = k.x;
    m[1][0] = i.y; m[1][1] = j.y; m[1][2] = k.y;
    m[2][0] = i.z; m[2][1] = j.z; m[2][2] = k.z;

    ray.orig.x = ray.orig.y = ray.orig.z = 0.0;
    ray.dir = get_sample_pos(x, y, sample);
    ray.dir.z = 1.0 / HALF_FOV;
    ray.dir.x *= RAY_MAG;
    ray.dir.y *= RAY_MAG;
    ray.dir.z *= RAY_MAG;

    dir.x = ray.dir.x + ray.orig.x;
    dir.y = ray.dir.y + ray.orig.y;
    dir.z = ray.dir.z + ray.orig.z;
    foo.x = dir.x * m[0][0] + dir.y * m[0][1] + dir.z * m[0][2];
    foo.y = dir.x * m[1][0] + dir.y * m[1][1] + dir.z * m[1][2];
    foo.z = dir.x * m[2][0] + dir.y * m[2][1] + dir.z * m[2][2];

    orig.x = ray.orig.x * m[0][0] + ray.orig.y * m[0][1] + ray.orig.z * m[0][2] + cam.pos.x;
    orig.y = ray.orig.x * m[1][0] + ray.orig.y * m[1][1] + ray.orig.z * m[1][2] + cam.pos.y;
    orig.z = ray.orig.x * m[2][0] + ray.orig.y * m[2][1] + ray.orig.z * m[2][2] + cam.pos.z;

    ray.orig = orig;
    ray.dir.x = foo.x + orig.x;
    ray.dir.y = foo.y + orig.y;
    ray.dir.z = foo.z + orig.z;

    return ray;
}


struct vec3 RayEngine::get_sample_pos(int x, int y, int sample) {
    struct vec3 pt;
    static double sf = 0.0;

    if(sf == 0.0) {
        sf = 1.5 / (double)xres;
    }

    pt.x = ((double)x / (double)xres) - 0.5;
    pt.y = -(((double)y / (double)yres) - 0.65) / aspect;

    if(sample) {
        struct vec3 jt = jitter(x, y, sample);
        pt.x += jt.x * sf;
        pt.y += jt.y * sf / aspect;
    }
    return pt;
}

/* jitter function taken from Graphics Gems I. */
struct vec3 RayEngine::jitter(int x, int y, int s) {
    struct vec3 pt;
    pt.x = urand[(x + (y << 2) + irand[(x + s) & MASK]) & MASK].x;
    pt.y = urand[(y + (x << 2) + irand[(y + s) & MASK]) & MASK].y;
    return pt;
}

/* Calculate ray-sphere intersection, and return {1, 0} to signify hit or no hit.
 * Also the surface point parameters like position, normal, etc are returned through
 * the sp pointer if it is not NULL.
 */
int RayEngine::ray_sphere(const struct sphere *sph, struct ray ray, struct spoint *sp) {
    double a, b, c, d, sqrt_d, t1, t2;

    a = SQ(ray.dir.x) + SQ(ray.dir.y) + SQ(ray.dir.z);
    b = 2.0 * ray.dir.x * (ray.orig.x - sph->pos.x) +
                2.0 * ray.dir.y * (ray.orig.y - sph->pos.y) +
                2.0 * ray.dir.z * (ray.orig.z - sph->pos.z);
    c = SQ(sph->pos.x) + SQ(sph->pos.y) + SQ(sph->pos.z) +
                SQ(ray.orig.x) + SQ(ray.orig.y) + SQ(ray.orig.z) +
                2.0 * (-sph->pos.x * ray.orig.x - sph->pos.y * ray.orig.y - sph->pos.z * ray.orig.z) - SQ(sph->rad);

    if((d = SQ(b) - 4.0 * a * c) < 0.0) return 0;

    sqrt_d = sqrt(d);
    t1 = (-b + sqrt_d) / (2.0 * a);
    t2 = (-b - sqrt_d) / (2.0 * a);

    if((t1 < ERR_MARGIN && t2 < ERR_MARGIN) || (t1 > 1.0 && t2 > 1.0)) return 0;

    if(sp) {
        if(t1 < ERR_MARGIN) t1 = t2;
        if(t2 < ERR_MARGIN) t2 = t1;
        sp->dist = t1 < t2 ? t1 : t2;

        sp->pos.x = ray.orig.x + ray.dir.x * sp->dist;
        sp->pos.y = ray.orig.y + ray.dir.y * sp->dist;
        sp->pos.z = ray.orig.z + ray.dir.z * sp->dist;

        sp->normal.x = (sp->pos.x - sph->pos.x) / sph->rad;
        sp->normal.y = (sp->pos.y - sph->pos.y) / sph->rad;
        sp->normal.z = (sp->pos.z - sph->pos.z) / sph->rad;

        sp->vref = reflect(ray.dir, sp->normal);
        NORMALIZE(sp->vref);
    }
    return 1;
}

/*
 *  Loads the scene description in form of an object list
 *  from the disk file specified by the given file pointer.
 */
#define DELIM   " \t\n"
void RayEngine::load_scene(FILE *fp) {
        char line[256], *ptr, type;

        obj_list = new sphere();
        obj_list->next = 0;

        while((ptr = fgets(line, 256, fp))) {
            int i;
            struct vec3 pos, col;
            double rad, spow, refl;

            while(*ptr == ' ' || *ptr == '\t') ptr++;
            if(*ptr == '#' || *ptr == '\n') continue;

            if(!(ptr = strtok(line, DELIM))) continue;
            type = *ptr;

            for(i=0; i<3; i++) {
                if(!(ptr = strtok(0, DELIM))) break;
                *((double*)&pos.x + i) = atof(ptr);
            }

            if(type == 'l') {
                lights[lnum++] = pos;
                continue;
            }

            if(!(ptr = strtok(0, DELIM))) continue;
            rad = atof(ptr);

            for(i=0; i<3; i++) {
                if(!(ptr = strtok(0, DELIM))) break;
                *((double*)&col.x + i) = atof(ptr);
            }

            if(type == 'c') {
                cam.pos = pos;
                cam.targ = col;
                cam.fov = rad;
                continue;
            }

            if(!(ptr = strtok(0, DELIM))) continue;
            spow = atof(ptr);

            if(!(ptr = strtok(0, DELIM))) continue;
            refl = atof(ptr);

            if(type == 's') {
                struct sphere *sph = new sphere();
                sph->next = obj_list->next;
                obj_list->next = sph;

                sph->pos = pos;
                sph->rad = rad;
                sph->mat.col = col;
                sph->mat.spow = spow;
                sph->mat.refl = refl;
            } else {
                fprintf(stderr, "unknown type: %c\n", type);
            }
        }
    }
