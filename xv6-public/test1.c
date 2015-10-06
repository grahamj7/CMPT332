#include "types.h"
#include "stat.h"
#include "user.h"

int
main (int argc, char *argv[]){
  int num1=-1, num2=-1;
  int i=0, rc, N=-1;
  for (i=2; i < argc; i++)
    printf(1, "Unused argv[%d]: %s\n", i, (char*)argv[i]);
  N = atoi(argv[1]); // defaults to 0
  
  printf(1, "N: %d\n", N); 
  
  printf(1, "Original P: %d\n", getpid());
  printf(1, "Creating First fork %d\n", i);  
  rc = fork();
  if (rc < 0)
    printf(1, "Failed to Fork\n");
  else if (rc == 0){ // chlid
    printf(1, "isChild (%d)\n", getpid());
  }
  else { // parent
    printf(1, "Nums before waitstat: %d, %d\n", num1, num2);
    waitstat(&num1, &num2);
    printf(1, "Nums After: %d, %d\n", num1, num2);
    printf(1, "isParent (%d)\nWAIT\n", getpid());
  }
  
  exit();
}




