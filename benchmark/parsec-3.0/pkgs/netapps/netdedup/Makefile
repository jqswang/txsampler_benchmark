.PHONY: origin opt measure clean all-clean outputclean

PARSECDIR=${TSX_ROOT}/benchmark-suite/parsec-3.0

origin: inputs clean
	-cp ./src/server/encoder.c.origin ./src/server/encoder.c
	-PARSEC_TM=RTM PARSECPLAT=linux-rtm $(PARSECDIR)/bin/parsecmgmt -a build -c gcc-pthreads -p netdedup

opt: inputs clean
	-cp ./src/server/encoder.c.opt ./src/server/encoder.c
	-PARSEC_TM=RTM PARSECPLAT=linux-rtm $(PARSECDIR)/bin/parsecmgmt -a build -c gcc-pthreads -p netdedup

inputs:
	ln -s ../../kernels/dedup/inputs

measure:
	-PARSECPLAT=linux-rtm.gcc-pthreads PARSECDIR=$(PARSECDIR) measure_benchmark.py $(ARGS)

clean:
	-PARSECPLAT=linux-rtm $(PARSECDIR)/bin/parsecmgmt -a clean -c gcc-pthreads -p netdedup
	-PARSECPLAT=linux-rtm $(PARSECDIR)/bin/parsecmgmt -a uninstall -c gcc-pthreads -p netdedup
	-rm -f ./src/server/encoder.c

all-clean: clean
	-rm -f *.dat.ddp
	-unlink inputs

outputclean:
	-killall -9 netdedup_client
	-killall -9 netdedup
