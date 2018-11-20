.PHONY: all clean origin rtm_global rtm_fine
define make_instance
	rm -rf bin;
	$(MAKE) -f Makefile.$(1) clean;
	$(MAKE) -f Makefile.$(1) EXTRA_DEFINE=$(3);
	rm -rf build_$(2);
	mkdir -p build_$(2);
	mv bin/ferret-parallel build_$(2)/ferret;
endef

all: origin rtm_naive_global_internal rtm_naive_global_external rtm_naive_fine rtm_balanced_global_internal rtm_balanced_global_external rtm_balanced_fine

origin: Makefile.origin
	$(call make_instance,origin,origin,)
rtm_naive_global_internal: Makefile.rtm
	$(call make_instance,rtm,$@,-DINTERNAL_GLOBAL_LOCK)
rtm_naive_global_external: Makefile.rtm
	$(call make_instance,rtm,$@,-DEXTERNAL_GLOBAL_LOCK)
rtm_naive_fine: Makefile.rtm
	$(call make_instance,rtm,$@,-DFINE_GRAINED_LOCK) 
rtm_balanced_global_internal: Makefile.rtm
	$(call make_instance,rtm,$@,"-DINTERNAL_GLOBAL_LOCK -DBALANCED")
rtm_balanced_global_external: Makefile.rtm
	$(call make_instance,rtm,$@,"-DEXTERNAL_GLOBAL_LOCK -DBALANCED")
rtm_balanced_fine: Makefile.rtm
	$(call make_instance,rtm,$@,"-DFINE_GRAINED_LOCK -DBALANCED")

clean:
	make -f Makefile.origin clean
	make -f Makefile.rtm clean
	rm -rf build_origin build_rtm* *.o
