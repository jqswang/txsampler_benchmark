CFLAGS= -fopenmp -O3 -g -fno-inline
CXXFLAGS = $(CFLAGS) -std=c++11
TARGET = a.out

all: origin omp-lock rtm-naive rtm-el

origin:  main.cpp avl.h avl.c
	g++ -c main.cpp $(CXXFLAGS)
	gcc -c avl.c $(CFLAGS)
	g++  main.o avl.o -o $(TARGET) $(CXXFLAGS)
	mkdir -p $@
	mv $(TARGET) $@/$(TARGET)
omp-lock: main.cpp avl.h avl.c
	g++ -c main.cpp $(CXXFLAGS) -DOMP_LOCK
	gcc -c avl.c $(CFLAGS) -DOMP_LOCK
	g++  main.o avl.o -o $(TARGET) $(CXXFLAGS)
	mkdir -p $@
	mv $(TARGET) $@/$(TARGET)
rtm-naive: main.cpp avl.h avl.c
	g++ -c main.cpp $(CXXFLAGS) -DRTM -I${TSX_ROOT}/lib_simple
	gcc -c avl.c $(CFLAGS) -DRTM -I${TSX_ROOT}/lib_simple
	g++  main.o avl.o -o $(TARGET) $(CXXFLAGS) -L${TSX_ROOT}/lib_simple -lrtm
	mkdir -p $@
	mv $(TARGET) $@/$(TARGET)
rtm-el: main.cpp avl.h avl.c
	g++ -c main.cpp $(CXXFLAGS) -DRTM -DRW_TLE -I./lib_rw
	gcc -c avl.c $(CFLAGS) -DRTM -DRW_TLE -I./lib_rw
	g++  main.o avl.o -o $(TARGET) $(CXXFLAGS) -L./lib_rw -lrtm
	mkdir -p $@
	mv $(TARGET) $@/$(TARGET)
clean:
	rm -rf origin omp-lock rtm-naive rtm-el *.o
