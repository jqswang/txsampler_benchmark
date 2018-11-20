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

#ifndef RAY_H
#define RAY_H

#include <string>
#include "image.h"

#define NRAN    1024
#define MASK    (NRAN - 1)

#define SL_BLOCK 8

#define MAX_LIGHTS      16              /* maximum number of lights */
#define RAY_MAG         1000.0          /* trace rays of this magnitude */
#define MAX_RAY_DEPTH   5               /* raytrace recursion limit */
#define FOV             0.78539816      /* field of view in rads (pi/4) */
#define HALF_FOV        (FOV * 0.5)
#define ERR_MARGIN      1e-6            /* an arbitrary error margin to avoid surface acne */

/* bit-shift ammount for packing each color into a 32bit uint */
#ifdef LITTLE_ENDIAN
#define RSHIFT  16
#define BSHIFT  0
#else   /* big endian */
#define RSHIFT  0
#define BSHIFT  16
#endif  /* endianess */
#define GSHIFT  8   /* this is the same in both byte orders */

/* some helpful macros... */
#define SQ(x)       ((x) * (x))
#define MAX(a, b)   ((a) > (b) ? (a) : (b))
#define MIN(a, b)   ((a) < (b) ? (a) : (b))
#define DOT(a, b)   ((a).x * (b).x + (a).y * (b).y + (a).z * (b).z)
#define NORMALIZE(a)  do {\
    double len = sqrt(DOT(a, a));\
    (a).x /= len; (a).y /= len; (a).z /= len;\
} while(0);

struct vec3 {
    double x, y, z;
};

struct ray {
    struct vec3 orig, dir;
};

struct material {
    struct vec3 col;    /* color */
    double spow;        /* specular power */
    double refl;        /* reflection intensity */
};

struct sphere {
    struct vec3 pos;
    double rad;
    struct material mat;
    struct sphere *next;
};

struct spoint {
    struct vec3 pos, normal, vref;  /* position, normal and view reflection */
    double dist;        /* parametric distance of intersection along the ray */
};

struct camera {
    struct vec3 pos, targ;
    double fov;
};

class RayEngine {
public:
    bool init(string srcname, int xres, int yres, int rpp, unsigned int numthreads, bool pinning);
    void run();
    void finish();
    Image* getOutputImage();
    void printRaytracingState();
    int getxres();
    int getyres();
    int getrpp();
    void render_scanline(int xsz, int ysz, int sl, int samples);

    bool pinning;
private:
    /* Data */
    int xres;
    int yres;
    int rays_per_pixel;
    int numthreads;
    double aspect;
    struct sphere *obj_list;
    struct vec3 lights[MAX_LIGHTS];
    int lnum;
    struct camera cam;
    struct vec3 urand[NRAN];
    int irand[NRAN];
    Image out;
    /* Functions */
    void load_scene(FILE *fp);
    struct vec3 trace(struct ray ray, int depth);
    struct vec3 shade(struct sphere *obj, struct spoint *sp, int depth);
    struct vec3 reflect(struct vec3 v, struct vec3 n);
    struct vec3 cross_product(struct vec3 v1, struct vec3 v2);
    struct ray get_primary_ray(int x, int y, int sample);
    struct vec3 get_sample_pos(int x, int y, int sample);
    struct vec3 jitter(int x, int y, int s);
    int ray_sphere(const struct sphere *sph, struct ray ray, struct spoint *sp);
};

struct thread_data {
    int* global_sl;
    RayEngine* re;
    int tid;
};


#endif