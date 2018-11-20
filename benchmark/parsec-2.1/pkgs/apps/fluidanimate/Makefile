
TARGET=fluidanimate

all: origin rtm

origin:
	rm -rf *.o build_$@
	$(MAKE) -f Makefile.$@
	mkdir -p build_$@
	mv $(TARGET) build_$@
rtm:
	rm -rf *.o build_$@
	$(MAKE) -f Makefile.$@
	mkdir -p build_$@
	mv $(TARGET) build_$@
clean:
	rm -rf *.o build_origin build_rtm cmp 
