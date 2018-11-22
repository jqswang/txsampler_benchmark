.PHONY: all tpcc-populate clean all-clean

PAPI_LIB_PATH=/opt/papi/install/lib

CURRENT_DIR=$(shell pwd)
TPCC_DIR=$(CURRENT_DIR)/build/TpccBenchmark
AUX_PREFIX=aux-lib

JEMALLOC= $(AUX_PREFIX)/jemalloc/install/lib/libjemalloc.so
LIBCUCKOO= $(AUX_PREFIX)/libcuckoo/install/lib/libcityhash.so
LIBZMQ=	$(AUX_PREFIX)/libzmq/install/lib/libzmq.so
LZ4= $(AUX_PREFIX)/lz4/install/lib/liblz4.so

DEPENDENCIES= $(JEMALLOC) $(LIBCUCKOO) $(LIBZMQ) $(LZ4)

all: build/TpccBenchmark/tpcc_benchmark tpcc-populate

build/TpccBenchmark/tpcc_benchmark: build/Makefile $(DEPENDENCIES) 
	cd build && make -j

$(JEMALLOC):
	cd $(AUX_PREFIX)/jemalloc && \
	./autogen.sh && \
	./configure --prefix=$(CURRENT_DIR)/$(AUX_PREFIX)/jemalloc/install && \
	make -j && (make -k install || true)
$(LIBCUCKOO):
	cd $(AUX_PREFIX)/libcuckoo && \
	autoreconf -fis && \
	./configure --prefix=$(CURRENT_DIR)/$(AUX_PREFIX)/libcuckoo/install && \
	make -j && make install
$(LIBZMQ):
	cd $(AUX_PREFIX)/libzmq && \
	./autogen.sh && \
	./configure --prefix=$(CURRENT_DIR)/$(AUX_PREFIX)/libzmq/install && \
	make -j && make install
$(LZ4):
	cd $(AUX_PREFIX)/lz4 && \
	make PREFIX=$(CURRENT_DIR)/$(AUX_PREFIX)/lz4/install && \
	make install PREFIX=$(CURRENT_DIR)/$(AUX_PREFIX)/lz4/install

tpcc-populate: build/TpccBenchmark/tpcc_benchmark
	$(TPCC_DIR)/tpcc_benchmark -a0 -sf12 -sf1	

build/Makefile:
	mkdir -p build && cd build && \
	cmake .. -DTSX_ROOT=${TSX_ROOT} -DPAPI_LIB_PATH=$(PAPI_LIB_PATH)
clean:
	cd build && make clean

all-clean: clean
	rm -rf build Tpcc
