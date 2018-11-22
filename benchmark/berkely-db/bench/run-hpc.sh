#!/bin/bash
set -x

make outputclean
sleep 1
${TXSAMPLER_CMD} ./ex_thread -i 300000 -r 6 -w 6

