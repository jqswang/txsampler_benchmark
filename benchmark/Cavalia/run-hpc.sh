#!/bin/bash
set -x
NUM_CORES=12
NUM_TX=$(($NUM_CORES * 1000000))

#TXSAMPLER_CMD="hpcrun -e cycles:precise=2@10000000 -e RTM_RETIRED:ABORTED@1000 -e RTM_RETIRED:COMMIT@1000"
rm -rf Tpcc/txn_*
sleep 1
${TXSAMPLER_CMD} build/TpccBenchmark/tpcc_benchmark -a2 -sf12 -sf1 -t$NUM_TX -c$NUM_CORES -p. -o0

