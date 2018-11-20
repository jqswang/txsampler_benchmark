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

bool BenchmarkEngine::init(string srcname, string destname, unsigned int angle) {
    re = new RotateEngine();
    ce = new ConvertEngine();
    if(!re->init(srcname, destname, angle))
        return false;
    if(!ce->init(re->getOutput(), destname.c_str()))
        return false;
    return true;
};

void BenchmarkEngine::run() {
    re->run();
    ce->run();
}

void BenchmarkEngine::finish() {
    re->finish();
    ce->finish();
    delete re;
    delete ce;
}