CC = g++
CFLAGS  = -O2
LIBS = -lpthread
SOURCES = image.cpp rotation_engine.cpp convert_engine.cpp benchmark_engine.cpp program.cpp
OBJECTS = $(SOURCES:.cpp=.o)
EXECUTABLE = rot-cc

all: $(SOURCES) $(EXECUTABLE)
 
$(EXECUTABLE): $(OBJECTS)  
	$(CC) $(OBJECTS) -o $@ $(LIBS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@ 

clean:
	rm -rf *.o $(EXECUTABLE) out.ppm output.ppm

