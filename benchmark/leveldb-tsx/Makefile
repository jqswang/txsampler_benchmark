
.PHONY: origin atomic opt clean all-clean

origin: snappy/install
	-rm -rf db
	-cp -r db-origin db
	-$(MAKE) -j -f Makefile.spec
	-$(MAKE) -j -f Makefile.spec db_bench

atomic: snappy/install
	-rm -rf db
	-cp -r db-atomic db
	-$(MAKE) -j -f Makefile.spec
	-$(MAKE) -j -f Makefile.spec db_bench

opt: snappy/install
	-rm -rf db
	-cp -r db-small db
	-$(MAKE) -j -f Makefile.spec
	-$(MAKE) -j -f Makefile.spec db_bench

clean:
	-rm -rf db
	-$(MAKE) -f Makefile.spec clean

snappy/install:
	cd snappy;./configure --prefix=$(shell pwd)/snappy/install;\
	make -j && make install

all-clean: clean
	cd snappy;$(MAKE) distclean;rm -rf install








