PROGRAM = netferret

INSTDIR = ${PARSECDIR}/pkgs/netapps/netferret/inst/${PARSECPLAT}

SUBDIRS = server client

.PHONY: subtargets  $(SUBDIRS) install clean 

$(PROGRAM): $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

install: 
	mkdir -p ${INSTDIR}
	cp -r ./server/parsec/bin ${INSTDIR}
	cp -r ./server/parsec/lib ${INSTDIR}
	mv ${INSTDIR}/bin/ferret-pthreads ${INSTDIR}/bin/server 
	mv ${INSTDIR}/bin/ferret-pthreads_sim ${INSTDIR}/bin/server_sim 
	cp ./client/client ${INSTDIR}/bin 
	cp ./client/client_sim ${INSTDIR}/bin 
	cp ./run.sh ${INSTDIR}/bin
	if [ ! -L ${PARSECDIR}/pkgs/netapps/netferret/inputs ]; \
	then \
		ln -s ${PARSECDIR}/pkgs/apps/ferret/inputs ${PARSECDIR}/pkgs/netapps/netferret/; \
	fi;
clean: 
	@for subdir in $(SUBDIRS); do \
		$(MAKE) -C $$subdir clean; \
	done
	unlink ${PARSECDIR}/pkgs/netapps/netferret/inputs
