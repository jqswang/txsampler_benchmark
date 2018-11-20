ssl :
ifeq (, $(wildcard ${PARSEC_ROOT}/pkgs/lib/ssl/libssl.a))
	cd ${PARSEC_ROOT}/pkgs/lib/ssl;\
	chmod u+x configure;\
	./configure -no-asm;\
	${TSX_ROOT}/tool/makefile_editor.py Makefile Makefile.new "DIRS= crypto ssl";\
	mv Makefile.new Makefile;\
	make -j AR_BIN=ar RANLIB=ranlib
endif

gsl :
ifeq (, $(wildcard ${PARSEC_ROOT}/pkgs/lib/install/include/gsl/gsl_rng.h))
	cd ${PARSEC_ROOT}/pkgs/lib/gsl;\
	chmod u+x configure;\
	./configure --prefix=${PARSEC_ROOT}/pkgs/lib/install;\
	make -j && make install
endif

imagick : zlib jpeg
ifeq (, $(wildcard ${PARSEC_ROOT}/pkgs/lib/install/include/Magick++.h))
	cd ${PARSEC_ROOT}/pkgs/lib/imagick;\
	chmod u+x configure;\
	./configure --without-perl --prefix=${PARSEC_ROOT}/pkgs/lib/install LDFLAGS=-L${PARSEC_ROOT}/pkgs/lib/install/lib CPPFLAGS=-I${PARSEC_ROOT}/pkgs/lib/install/include;\
	make -j && make install
endif

jpeg : 
ifeq (, $(wildcard ${PARSEC_ROOT}/pkgs/lib/install/include/jpeglib.h))
	cd ${PARSEC_ROOT}/pkgs/lib/jpeg-8c;\
	chmod u+x configure;\
	./configure --prefix=${PARSEC_ROOT}/pkgs/lib/install;\
	make -j && make install
endif

zlib :
ifeq (, $(wildcard ${PARSEC_ROOT}/pkgs/lib/install/include/zlib.h))
	cd ${PARSEC_ROOT}/pkgs/lib/zlib;\
	chmod u+x configure;\
	./configure --prefix=${PARSEC_ROOT}/pkgs/lib/install;\
	make -j && make install
endif


librtm.so :
ifeq (, $(wildcard $(TM_DIR)/librtm.so))
	cd $(TM_DIR); make -C $(TM_DIR) CC=$(CC)
endif
