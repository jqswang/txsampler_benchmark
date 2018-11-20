# Makefile for rgbyuv

TARGET=rgbyuv
OBJS=imgio.o bmark.o
CFLAGS=-O2 -g -D_GNU_SOURCE -fopenmp
LIBS=-lm
CC=gcc

all: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET) $(LIBS)

%.o : %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o $(TARGET) *comp.ppm yuvout.ppm
