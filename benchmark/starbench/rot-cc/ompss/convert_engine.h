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
#ifndef C_ENGINE_H
#define C_ENGINE_H

#include <stdint.h>
#include <stdio.h>
#include "image.h"
#include <sys/time.h>
#include <cmath>

class ConvertEngine {
public:
    ConvertEngine();
    bool init(Image* in, const char* destname);
    void run();
    void finish();
    void convertLine(int line, char* dep);
private:
    Image* input, *output;
    int width, height;
    bool initialized, done;
    char* outfilename;
};

#endif // C_ENGINE_H