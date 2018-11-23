
.PHONY: origin atomic opt clean all-clean

origin: snappy/install
	-rm -rf db
	-cp -r db-origin db
	-$(MAKE) -j -f Makefile.spec
	-$(MAKE) -j -f Makefile.spec db_bench && mv db_bench db_bench_origin

atomic: snappy/install
	-rm -rf db
	-cp -r db-atomic db
	-$(MAKE) -j -f Makefile.spec
	-$(MAKE) -j -f Makefile.spec db_bench && mv db_bench db_bench_atomic

opt: snappy/install
	-rm -rf db
	-cp -r db-small db
	-$(MAKE) -j -f Makefile.spec
	-$(MAKE) -j -f Makefile.spec db_bench && mv db_bench db_bench_opt

clean:
	-rm -rf db db_bench_*
	-$(MAKE) -f Makefile.spec clean

snappy/install:
	cd snappy && ./autogen.sh && ./configure --prefix=$(shell pwd)/snappy/install \
	&& make -j && make install

all-clean: clean
	cd snappy;$(MAKE) distclean;rm -rf install








