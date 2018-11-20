CC = g++
CFLAGS  = -O2 -ffast-math
SOURCES = image.cpp rotation_engine.cpp program.cpp
OBJECTS = $(SOURCES:.cpp=.o)
EXECUTABLE = rot

all: $(SOURCES) $(EXECUTABLE)
 
$(EXECUTABLE): $(OBJECTS)  
	$(CC) $(OBJECTS) -o $@

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@ 

clean:
	rm -rf *.o $(EXECUTABLE) out.ppm output.ppm

