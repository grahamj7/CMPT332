#include "types.h"
#include "stat.h"
#include "user.h"

int
main(void){
	int num1=5, num2=10;
	
	printf(1, "Syscall to waitstat, RV: %d\n", waitstat(&num1, &num2));
	printf(1, "Values\n Num1: %d, Num2: %d\n", num1, num2);
	exit();
}




