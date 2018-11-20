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

#include "convert_engine.h"

typedef struct timeval timer;

ConvertEngine::ConvertEngine() {
    done = false;
    initialized = false;
};

bool ConvertEngine::init(Image* in, const char* destname) {
    input = in;
    width = input->getWidth();
    height = input->getHeight();
    output = new Image();
    output->createImageFromTemplate(width, height, 3);
    initialized = true;
    outfilename = (char*) destname;
    return true;
}

void ConvertEngine::convertLine(int line) {
    uint8_t R, G, B;
    Pixel p; // Output, components used for YUV

    for(int j = 0; j < width; j++) {
        R=input->getPixelAt(j,line).r;
        G=input->getPixelAt(j,line).g;
        B=input->getPixelAt(j,line).b;

        p.r = round(0.256788*R+0.504129*G+0.097906*B) + 16;
        p.g = round(-0.148223*R-0.290993*G+0.439216*B) + 128;
        p.b = round(0.439216*R-0.367788*G-0.071427*B) + 128;

        output->setPixelAt(j,line,&p);
    }

    if(line == height-1)
        done = true;
}

void ConvertEngine::run() {
    for(int i = 0; i < height; i++) {
        convertLine(i);
    }
    done = true;
    return;
}

void ConvertEngine::finish() {
    char uvheader[256];
    snprintf(uvheader, (size_t)255, "P6\n%d %d\n%d\n", width, height, RGB_MAX_COLOR);

    // Write the full YUV image
    FILE* fp = fopen(outfilename, "w");
    if (fp == NULL) {
        fprintf(stderr, "Cannot open output file\n");
        output->clean();
        delete output;
    }

    fprintf(fp, "%s", uvheader);
    char writebuf[3*width];
    for(int i = 0; i < height; i++) {
        for(int j = 0; j < width; j++) {
            writebuf[3*j] = output->getPixelAt(j,i).r;
            writebuf[3*j+1] = output->getPixelAt(j,i).g;
            writebuf[3*j+2] = output->getPixelAt(j,i).b;
        }
        fwrite(writebuf, 1, 3*width, fp);
    }
    fclose(fp);


    // Input image is cleaned up by the rotation engine, so no need to delete it
    output->clean();
    delete output;
}