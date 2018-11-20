#include<stdint.h>
#include<stdio.h>
#include"htm_util.h"
int credit[100];
int debit[100];

TransactionDiagnosticInfo dia;

int main(int argc, char **argv){
	for(int i=0; i < 10000; i++){
		dia.transactionAbortCode = 0xffffffff;
		if ( tbegin(&dia) == 0){
		for (int j=0; j < 100; j++){
			credit[0]++;
			debit[0]++;
		}
		tend();
		}
		else { //fallback
			printf("Abort: %lx\n", dia.transactionAbortCode);
		}
	}
	printf("credit addr = %p\n",  credit);
	printf("debit addr = %p\n",  debit);
	return 0;
}
