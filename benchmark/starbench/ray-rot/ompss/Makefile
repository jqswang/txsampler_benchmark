CC = g++
CFLAGS  = -O2 -fopenmp
LDFLAGS = -fopenmp
SOURCES = image.cpp rotation_engine.cpp ray_engine.cpp program.cpp
OBJECTS = $(SOURCES:.cpp=.o)
LIBS=
EXECUTABLE = ray-rot

all: $(SOURCES) $(EXECUTABLE)
 
$(EXECUTABLE): $(OBJECTS)  
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@ $(LIBS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@ 

clean:
	rm -rf *.o $(EXECUTABLE) out.ppm output.ppm

