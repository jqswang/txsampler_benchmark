# Makefile for streamcluster

TARGET=streamcluster
OBJS=util.o streamcluster.o
CFLAGS=-g -std=gnu99 -O2
#-D ENABLE_THREADS
LDFLAGS= -pthread 
LIBS=-lm
CC=gcc

all: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -o $(TARGET) $(LIBS)

%.o : %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o $(TARGET) output.txt
