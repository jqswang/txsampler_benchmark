
.PHONY: origin opt clean all-clean

origin: clean
	-cp betweennessCentrality.c.origin betweennessCentrality.c
	-$(MAKE) -j -f Makefile.spec

opt: clean
	-cp betweennessCentrality.c.opt betweennessCentrality.c
	-$(MAKE) -j -f Makefile.spec CFLAGS=-DTX_SPLIT 
clean:
	-rm -f betweennessCentrality.c
	-$(MAKE) -f Makefile.spec clean

all-clean: clean








