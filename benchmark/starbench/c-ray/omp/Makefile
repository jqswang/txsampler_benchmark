CC = gcc
CFLAGS  = -fopenmp
LDFLAGS = -lm -fopenmp
SOURCES = c-ray-mt.c
OBJECTS = $(SOURCES:.c=.o)
EXECUTABLE = c-ray-mt

all: $(SOURCES) $(EXECUTABLE)
 
$(EXECUTABLE): $(OBJECTS)  
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@ -lm

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf *.o $(EXECUTABLE) *.ppm

