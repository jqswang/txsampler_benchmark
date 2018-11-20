TM_LIB=${TSX_ROOT}/lib_simple
CXXFLAGS= -g -O0 -fno-inline -I$(TM_LIB)
LDFLAGS = -L$(TM_LIB) -lrtm

test: main.cpp
	g++ -o $@ $^ $(CXXFLAGS) $(LDFLAGS)

clean:
	rm -rf build_* *.o test
