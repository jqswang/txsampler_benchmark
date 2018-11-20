

SUBDIR=blackscholes  facesim  fluidanimate  Makefile  swaptions  x264 bodytrack ferret freqmine raytrace  vips 


.PHONY: all-clean clean

all-clean:
	for d in $(SUBDIR); do \
		rm -rf $$d/inputs/*;\
	done
	

clean:
	#for d in $(SUBDIR); do \
		$(MAKE) -C pkgs/$$d clean;\
	done
