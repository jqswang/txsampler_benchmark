.PHONY: all clean

DIRS = c-ray

VERSION = pthread # omp | ompss | pthread | seq

all:
	for d in $(DIRS) ; do \
		$(MAKE) -C $$d/$(VERSION) ; \
	done ;

clean:
	for d in $(DIRS) ; do \
		$(MAKE) -C $$d/$(VERSION) clean ; \
	done ;
