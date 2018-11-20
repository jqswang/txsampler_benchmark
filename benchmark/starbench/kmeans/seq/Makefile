CC = gcc
CFLAGS = -g -O2 -std=gnu99
LDFLAGS = 

LIBS = -lm
TARGET = kmeans
OBJ  = wtime.o file_io.o kmeans.o main.o

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $@ $(LDFLAGS) $(OBJ) $(LIBS)

.c.o: 
	$(CC) -c $(CFLAGS) $*.c

clean:
	rm -f $(TARGET) $(OBJ)
