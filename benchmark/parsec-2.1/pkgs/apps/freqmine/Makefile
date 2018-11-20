TARGET = freqmine
all: origin rtm

origin:
	make -f Makefile.$@ clean
	rm -rf build_$@
	make -f Makefile.$@ all
	mkdir -p build_$@
	mv $(TARGET) build_$@
	make -f Makefile.$@ clean
rtm:
	make -f Makefile.$@ clean
	rm -rf build_$@
	make -f Makefile.$@ all
	mkdir -p build_$@
	mv $(TARGET) build_$@
	make -f Makefile.$@ clean
clean:
	make -f Makefile.origin clean
	make -f Makefile.rtm clean
	rm -rf build_origin build_rtm
