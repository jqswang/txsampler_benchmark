CC  = g++ -fopenmp  -g
CFLAGS  =  -O3 -I/usr/include/c++ -I$(TSX_ROOT)/lib/rtm #-fbuiltin
OBJS	= ListItemset.o
OBJS2   = rdtsc.o Database.o Itemset.o HashTree.o
OPTS	= -DBALT
LIBS =  -lm -L$(TSX_ROOT)/lib/rtm -lrtm
LD = ld

TARGET  = no_output_apriori

default:	$(TARGET)

clean:
	rm -rf *~ *.o $(TARGET)

all-clean: clean
	unlink inputs

no_output_apriori: no_output_parapr.o $(OBJS) $(OBJS2)
	$(CC) $(CFLAGS) $(OPTS) -o no_output_apriori no_output_parapr.o $(OBJS) $(OBJS2) $(LIBS)

no_output_parapr.o: no_output_parapr.cpp
	$(CC) $(CFLAGS) -c $^ -o $@	

rdtsc.o: rdtsc.cpp
	$(CC) $(CFLAGS) -c rdtsc.cpp -o $@

Database.o: Database.cpp Database.h
	$(CC) $(CFLAGS) $(OPTS) -c Database.cpp -o $@

Itemset.o: Itemset.cpp Itemset.h
	$(CC) $(CFLAGS) $(OPTS) -c Itemset.cpp -o $@

ListItemset.o: ListItemset.cpp ListItemset.h
	$(CC) $(CFLAGS) -c ListItemset.cpp -o $@

HashTree.o: HashTree.cpp HashTree.h
	$(CC) $(CFLAGS) $(OPTS) -c HashTree.cpp -o $@

dec_shlib.o: dec_shlib.C dec_shlib.h
	$(CC) $(CFLAGS) $(OPTS) -c dec_shlib.C -o $@

dec_llsc.o: dec_llsc.s llsc.h
	$(CC)  -c dec_llsc.s -o $@
