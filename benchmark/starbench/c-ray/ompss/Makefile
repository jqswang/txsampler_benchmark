CC = sscc
CFLAGS  = -O3 -ffast-math
LDFLAGS =
LIBS = -lm
SOURCES = c-ray-mt.c
OBJECTS = $(SOURCES:.c=.o)
EXECUTABLE = c-ray-mt

all: $(SOURCES) $(EXECUTABLE)
 
$(EXECUTABLE): $(OBJECTS)  
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf *.o $(EXECUTABLE) *.ppm

