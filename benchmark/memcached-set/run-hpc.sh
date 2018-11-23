#!/bin/bash
set -x
MEMCACHED=$(pwd)/memcached/build_rtm/bin/memcached
#MEMCACHED=$(pwd)/memcached-psu/memcached
MEMASLAP=$(pwd)/libmemcached-1.0.18/clients/memaslap
#echo $HTM_TRETRY
${TXSAMPLER_CMD} $MEMCACHED -l 127.0.0.1 -p 11212 -t ${THREADS} & #> output.txt 2>&1 &

sleep 5

$MEMASLAP -s 127.0.0.1:11212 -T 14 -c 896 -t 2m

killall $MEMCACHED

sleep 2




