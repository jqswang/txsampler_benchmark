# Makefile for md5

TARGET=md5
OBJS=md5.o md5_bmark.o
CFLAGS=-O2 -D_GNU_SOURCE -std=c99
LDFLAGS=-pthread
CC=gcc

all: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -o $(TARGET)

%.o : %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o $(TARGET) output.txt
