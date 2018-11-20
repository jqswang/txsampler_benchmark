#include <omp.h>
#include <stdlib.h>
#include <iostream>
#include "tm.h"
#include "MersenneTwister.h"

//#define NUM_OF_ITERATIONS (1<<20)

unsigned long seed = 0;

int main(int argc, char **argv){
	if (argc < 2){
		std::cout <<"Invalid Input!" << std::endl;
		std::cout <<"USage: "<< argv[0]<< " [the_power_number_of_iteration_num]"<<std::endl;
		return 0;
	}
	int power_number = atoi(argv[1]);
	unsigned long num_iteration = 1 << power_number;
#pragma omp parallel
{
	MTRand * _rng;
	for(int i=0; i< num_iteration ; i++){
#ifdef ORIGIN
		TM_BEGIN();
		_rng = new MTRand(++seed);
		TM_END();
#else
                _rng = new MTRand(seed);
                TM_BEGIN(); 
		seed++;
		TM_END();
#endif
		delete _rng;
	}
}	
	return 0;
}
