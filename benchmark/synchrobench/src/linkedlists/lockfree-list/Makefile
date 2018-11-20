ROOT = ../../..

include $(ROOT)/common/Makefile.common

ifeq ($(STM),RTM)
  TARGETS = origin locknest rle
else
  TARGETS = origin
endif

.PHONY:	all clean 

all:
	for t in $(TARGETS) ; do \
		rm -f $(BUILDIR)/*; \
		$(MAKE) -f Makefile.$$t; \
	done

clean:
	for t in $(TARGETS) ; do \
		$(MAKE) -f Makefile.$$t clean; \
	done

