#!/bin/bash
set -x
MEMCACHED=$(pwd)/memcached/build_rtm/bin/memcached
MCPERF=$(pwd)/twemperf/install/bin/mcperf
#echo $HTM_TRETRY
${TXSAMPLER_CMD} $MEMCACHED -t ${THREADS} > output.txt 2>&1 &

sleep 2

/usr/bin/time -f %e -o measure_time_2.tmp $MCPERF --linger=0 --timeout=5 --conn-rate=1000 --call-rate=1000 --num-calls=10 --num-conns=10000 --sizes=u1,16 > mcperf.output 2>&1
#$MCPERF --linger=0 --timeout=5 --conn-rate=1000 --call-rate=1000 --num-calls=2 --num-conns=1000 --sizes=u1,16 > mcperf.output 2>&1
cat mcperf.output
echo "TIME " $(cat measure_time_2.tmp)
#rm -f measure_time_2.tmp

killall $MEMCACHED

sleep 2

echo "TIME " $(cat measure_time_2.tmp) > /tmp/measured_time.tmp
rm -f measure_time_2.tmp




