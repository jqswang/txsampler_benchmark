CC = g++
CFLAGS  = -g -O2 -fopenmp
LDFLAGS = -fopenmp
SOURCES = image.cpp rotation_engine.cpp convert_engine.cpp benchmark_engine.cpp program.cpp
OBJECTS = $(SOURCES:.cpp=.o)
LIBS=
EXECUTABLE = rot-cc

all: $(SOURCES) $(EXECUTABLE)
 
$(EXECUTABLE): $(OBJECTS)  
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@ $(LIBS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@ 

clean:
	rm -rf *.o $(EXECUTABLE) out.ppm output.ppm

