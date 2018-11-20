#!/bin/bash
#set -x
NUM_CORES=12
NUM_TX=$(($NUM_CORES * 1000000))


rm -rf Tpcc/txn_*
sleep 1
${HPCRUN_CMD} build/TpccBenchmark/tpcc_benchmark -a2 -sf12 -sf1 -t$NUM_TX -c$NUM_CORES -p. -o0

