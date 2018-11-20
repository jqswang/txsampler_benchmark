.PHONY: all clean origin rtm_global rtm_fine
define make_instance
	rm -rf *.o dedup;
	$(MAKE) -f Makefile.$(1);
	rm -rf build_$(1);
	mkdir -p build_$(1);
	mv dedup build_$(1);
endef

all: origin rtm_global rtm_fine

origin: Makefile.origin
	$(call make_instance,origin)
rtm_global: Makefile.rtm_global
	$(call make_instance,rtm_global)
rtm_fine: Makefile.rtm_fine
	$(call make_instance,rtm_fine)

clean:
	make -f Makefile.origin clean
	make -f Makefile.rtm_global clean
	make -f Makefile.rtm_fine clean
	rm -rf build_origin build_rtm* *.o
