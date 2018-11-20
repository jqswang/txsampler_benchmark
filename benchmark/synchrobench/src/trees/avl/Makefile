ROOT = ../../..

include $(ROOT)/common/Makefile.common

BINS = $(BINDIR)/$(STM)-avl $(BINDIR)/$(STM)-avl-move

CFLAGS += -DNO_UNITLOADS -DTFAVLSEQ -DMICROBENCH -I./include


.PHONY:	all clean 

all:	clean main_move #main

avlnounit.o:
	$(CC) $(CFLAGS) -c -o avlnounit.o avlnounit.c

intset.o: avlnounit.h
	$(CC) $(CFLAGS) -c -o intset.o intset.c

test.o: test.c avlnounit.h intset.h
	$(CC) $(CFLAGS) -c -o $@ $<

move_test.o: move_test.c avlnounit.h intset.h
	$(CC) $(CFLAGS) -c -o $@ $<


main: avlnounit.o intset.o test.o 
	$(CC) $(CFLAGS) avlnounit.o intset.o test.o -o $(BINDIR)/$(STM)-avl  $(LDFLAGS)

main_move: avlnounit.o intset.o move_test.o
	$(CC) $(CFLAGS) $^ -o $(BINDIR)/$(STM)-avl-move  $(LDFLAGS)

	
clean:
	-rm -f $(BINS) *.o

