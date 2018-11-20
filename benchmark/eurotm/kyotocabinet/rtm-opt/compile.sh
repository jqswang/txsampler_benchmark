#!/bin/bash



./configure CPPFLAGS=-I$TSX_ROOT/lib/rtm LDFLAGS="-L$TSX_ROOT/lib/rtm -lrtm"
make -j
