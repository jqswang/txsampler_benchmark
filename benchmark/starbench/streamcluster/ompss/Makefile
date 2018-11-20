# Makefile for streamcluster

TARGET=streamcluster
OBJS=util.o streamcluster.o
CFLAGS=-g -D_GNU_SOURCE -O2 -std=c99 -fopenmp
LIBS= -lm
CC=gcc

all: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET) $(LIBS)

%.o : %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o $(TARGET) output.txt
