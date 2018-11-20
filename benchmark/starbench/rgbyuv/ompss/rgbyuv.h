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

#ifndef _RGBCMY_H_
#define _RGBCMY_H_

#include <stdint.h>

#define IN_DEPTH 3
#define OUT_DEPTH 3
#define MAXCOLOR 255

#define BPELS 30000

/* Structure which contains all relevant benchmarking parameters */
struct rgbyuv_args {
    int width;          // width in bytes
    int height;         // height in bytes
    int pixels;         // overall pixels
    uint8_t* in_img;    // input image
    uint8_t* pY;   // Y
    uint8_t* pU;   // U
    uint8_t* pV;   // V
};

typedef struct rgbyuv_args rgbyuv_args_t;

#endif // _RGBCMY_H_