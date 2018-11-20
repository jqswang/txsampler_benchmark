TM_DIR=${TSX_ROOT}/lib_simple
INCLUDE_DIR=-I$(TM_DIR)

CFLAGS=-fopenmp -g  -pthread -O0 -fno-inline $(INCLUDE_DIR)
CXXFLAGS=$(CFLAGS)
LDFLAGS=-lpthread -L$(TM_DIR) -lrtm

CPP=g++
CXX=g++
CC=gcc

test : transaction.c
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

clean:
	rm -rf hpctoolkit-* *.hpcstruct test
	

