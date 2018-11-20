#!/bin/bash
CURRENT=$(pwd)
cd libevent-2.0.21-stable
chmod u+x configure
./configure --prefix=$CURRENT/libevent --no-create --no-recursion
make -j && make install
cd ../libevent
ln -s lib64 lib
cd ..

chmod u+x make.sh
#./make.sh origin
./make.sh rtm

