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

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

char ppm_getc(FILE *file)
{
   char c;

   c = (char)getc(file);

    // Skip commented lines
   if (c == '#')
   {
      do
      {
         c = (char)getc(file);
      }
      while (c!='\n' && c!='\r');
   }

   return c;
}

int ppm_getint(FILE *file)
{
   char c;
   int i = 0;

   do
   {
      c = ppm_getc(file);
   }
   while (c==' ' || c=='\t' || c=='\n' || c=='\r');

   do
   {
      i = i*10 + c-'0';
      c = ppm_getc(file);
   }
   while (c>='0' && c<='9');

   return i;
}

int read_ppmheader(FILE *handle,int *width,int *height,int *maxcolor,int *depth) {
    int bytes;
    char buf[2];

    /* trap bad arguments */
    if (!handle || !width || !height || !maxcolor || !depth)
        exit(-1);

    *width = *height = *maxcolor = *depth = 0;

    /* PPM Header Key, 5 or 6 for now (others are possible) */
    bytes = fread(buf,1,2,handle);
    if ( (bytes < 2) || (buf[0] != 'P') || ((buf[1] != '6' ) && (buf[1] != '5')))
        return -1;

    if (buf[1]=='6')
        *depth=3;
    else
        return -1;

    *width=ppm_getint(handle);
    *height=ppm_getint(handle);
    *maxcolor=ppm_getint(handle);

    if(*width < 0 || *height < 0 || *maxcolor != 255)
        return -1;

    return 0;
}

uint8_t* loadPPMFile(char* fname, int* width, int* height, int* maxcolor, int* depth) {
    FILE    *fd;
    int     expected_size;
    uint8_t *buf;

    // Locate the exact file path
    if ((fd=fopen(fname,"rb"))==NULL){
        exit(-1);
    }
    // Get the header information and trap errors
    if (read_ppmheader(fd,width,height,maxcolor,depth))
        exit(-1);
    if ((expected_size=(*width) * (*height) * (*depth))==0 || *maxcolor != 255)
        exit(-1);

    // Allocate and read actual color values
    buf = malloc(expected_size);
    if ((fread(buf,expected_size,1,fd))!=1)
        exit(-1);

    fclose(fd);
    return buf;
}