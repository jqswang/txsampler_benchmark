#! /bin/bash

#compile

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

#============================================================================
# Complile K-means
#============================================================================

cd $DIR/kmeans/ompss
make clean
make

cd ../pthread
make clean
make

cd ../seq
make clean
make

#============================================================================
# Complile ray-rot
#============================================================================

cd $DIR/ray-rot/ompss
make clean
make

cd ../pthread
make clean
make

cd ../seq
make clean
make

#============================================================================
# Complile rot-cc
#============================================================================

cd $DIR/rot-cc/ompss
make clean
make

cd ../pthread
make clean
make

cd ../seq
make clean
make

#============================================================================
# Complile rotate
#============================================================================

cd $DIR/rotate/ompss
make clean
make

cd ../pthread
make clean
make

cd ../seq
make clean
make

#============================================================================
# Complile c-ray
#============================================================================

cd $DIR/c-ray/omp
make clean
make

cd ../pthread
make clean
make

cd ../seq
make clean
make

#============================================================================
# Complile md5
#============================================================================

cd $DIR/md5/ompss
make clean
make

cd ../pthread
make clean
make

cd ../seq
make clean
make

#============================================================================
# Complile rgbyuv
#============================================================================

cd $DIR/rgbyuv/ompss
make clean
make

cd ../pthread
make clean
make

cd ../seq
make clean
make

#============================================================================
# Complile streamcluster
#============================================================================

cd $DIR/streamcluster/ompss
make clean
make

cd ../pthread
make clean
make

cd ../seq
make clean
make