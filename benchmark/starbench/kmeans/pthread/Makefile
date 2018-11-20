CC = gcc
CFLAGS = -g -O2 -std=gnu99 -D_GNU_SOURCE
LDFLAGS = 

LIBS = -lm -lpthread
TARGET = kmeans
OBJ  = wtime.o file_io.o pthreads_kmeans.o pthreads_main.o

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $@ $(LDFLAGS) $(OBJ) $(LIBS)

.c.o: 
	$(CC) -c $(CFLAGS) $*.c

clean:
	rm -f $(TARGET) $(OBJ)
