CC = g++
CFLAGS  = -O2 -ffast-math -fopenmp
LDFLAGS = -fopenmp
SOURCES = image.cpp rotation_engine.cpp program.cpp
OBJECTS = $(SOURCES:.cpp=.o)
LIBS=
EXECUTABLE = rot

all: $(SOURCES) $(EXECUTABLE)
 
$(EXECUTABLE): $(OBJECTS)  
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@ $(LIBS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@ 

clean:
	rm -rf *.o $(EXECUTABLE) out.ppm output.ppm

