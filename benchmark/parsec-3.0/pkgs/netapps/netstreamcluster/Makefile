.PHONY: origin opt measure clean all-clean outputclean

PARSECDIR=${TSX_ROOT}/benchmark-suite/parsec-3.0

origin:  clean
	-PARSEC_TM=RTM PARSECPLAT=linux-rtm $(PARSECDIR)/bin/parsecmgmt -a build -c gcc-pthreads -p netstreamcluster

#opt:  clean
#	-cp ./src/server/encoder.c.opt ./src/server/encoder.c
#	-PARSEC_TM=RTM PARSECPLAT=linux-rtm parsecmgmt -a build -c gcc-pthreads -p netdedup

measure:
	-PARSECPLAT=linux-rtm.gcc-pthreads PARSECDIR=${TSX_ROOT}/benchmark-suite/parsec-3.0 measure_benchmark.py $(ARGS)

clean:
	-PARSECPLAT=linux-rtm $(PARSECDIR)/bin/parsecmgmt -a clean -c gcc-pthreads -p netstreamcluster
	-PARSECPLAT=linux-rtm $(PARSECDIR)/bin/parsecmgmt -a uninstall -c gcc-pthreads -p netstreamcluster

all-clean: clean
	#-unlink inputs

