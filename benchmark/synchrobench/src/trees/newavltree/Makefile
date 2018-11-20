ROOT = ../../..

include $(ROOT)/common/Makefile.common

ifeq ($(STM),RTM)
  TARGETS = origin rle
else
  TARGETS = origin
endif

.PHONY:	all origin rle clean

all:
	for t in $(TARGETS) ; do \
		rm -rf $(BUILDIR); \
		$(MAKE) -f Makefile.$$t; \
	done

clean:
	for t in $(TARGETS) ; do \
		$(MAKE) -f Makefile.$$t clean; \
	done
