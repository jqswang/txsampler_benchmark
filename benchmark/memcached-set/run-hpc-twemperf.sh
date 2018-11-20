#!/bin/bash
set -x
MEMCACHED=$(pwd)/memcached/build_rtm/bin/memcached
MCPERF=$(pwd)/twemperf/install/bin/mcperf
#echo $HTM_TRETRY
$HPCRUN_CMD $MEMCACHED -t 14 > output.txt 2>&1 &

sleep 2

/usr/bin/time -f %e -a -o measure_time_2.tmp numactl --physcpubind=90 $MCPERF --linger=0 --timeout=5 --conn-rate=1000 --call-rate=1000 --num-calls=10 --num-conns=10000 --sizes=u1,16 > mcperf.output 2>&1
#$MCPERF --linger=0 --timeout=5 --conn-rate=1000 --call-rate=1000 --num-calls=2 --num-conns=1000 --sizes=u1,16 > mcperf.output 2>&1
cat mcperf.output
echo "TIME " $(cat measure_time_2.tmp)
rm -f measure_time_2.tmp

killall $MEMCACHED

sleep 2




