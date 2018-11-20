#!/bin/bash

if [ "$#" == 0 ]; then
	cd libtiff
	make
	cd ..
	make
	exit
fi

if [ "$1" == "clean" ]; then
        cd libtiff
        make clean
        cd ..
        make clean
        exit
fi
