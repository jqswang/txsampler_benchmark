#include "tm.h"
#include<omp.h>
#include<stdio.h>

struct Account {
	long debit;
	long credit;
} myAccount;

long iterations_per_thread;

void fn1(){
	int i=0;
	for (i=0; i < 50; i++){	
		myAccount.debit++;
		myAccount.credit++;
	}
}
void fn1_p(){
        int i=0;
        for (i=0; i < 50; i++){
                myAccount.debit++;
                myAccount.credit++;
        }


}


void fn2(){
	fn1();
}

void fn3(){
	fn2();
	fn1_p();
}


void *thr_fn(void *arg){
	int i,j=0;
	TM_THREAD_ENTER();
        for(i=0; i < iterations_per_thread; i++){
			TM_BEGIN();
			fn3();
			TM_END();
		}
	TM_THREAD_EXIT();
	//printf("Finished!\n");
	return NULL;
}

int main(int argc, char **argv)
{
	if (argc < 3){
		printf("Usage: %s [num_of_threads] [iteration_per_thread]\n", argv[0]);
		return 0;
	}
	int threads_num = atoi(argv[1]);
	iterations_per_thread = atoi(argv[2]);
	myAccount.debit=0;
	myAccount.credit=0;

	omp_set_num_threads(threads_num);
	TM_STARTUP(threads_num);
#pragma omp parallel
{
        thr_fn(NULL);
}
		
	TM_SHUTDOWN();
	printf("debit = %ld\n", myAccount.debit);
	printf("credit = %ld\n", myAccount.credit);
	return 0;
}
