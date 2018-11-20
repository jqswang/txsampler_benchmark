TM_LIB=${TSX_ROOT}/lib_simple
CXXFLAGS= -g -O3 -fno-inline -I$(TM_LIB)
LDFLAGS = -L$(TM_LIB) -lrtm

all: org.out opt.out 

org.out: main.cpp MersenneTwister.h
	rm -rf build_origin
	mkdir build_origin
	g++ -o generator $^ $(CXXFLAGS) -DORIGIN $(LDFLAGS)
	mv generator build_origin/
opt.out: main.cpp MersenneTwister.h
	rm -rf build_opt
	mkdir build_opt
	g++ -o generator $^ $(CXXFLAGS) $(LDFLAGS)
	mv generator build_opt/

clean:
	rm -rf build_* *.o
