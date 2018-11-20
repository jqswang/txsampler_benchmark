#!/bin/bash


make outputclean
sleep 1
${HPCRUN_CMD} ./ex_thread -i 300000 -r 6 -w 6

