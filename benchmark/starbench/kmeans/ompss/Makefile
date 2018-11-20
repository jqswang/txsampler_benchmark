CC = gcc
CFLAGS = -g -O2 -fopenmp
LDFLAGS = -fopenmp

LIBS = -lm
TARGET = kmeans
OBJ  = wtime.o file_io.o kmeans_ompss.o main.o

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $@ $(LDFLAGS) $(OBJ) $(LIBS)

.c.o: 
	$(CC) -c $(CFLAGS) $*.c

clean:
	rm -f $(TARGET) $(OBJ)
