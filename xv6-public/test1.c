#include "types.h"
#include "stat.h"
#include "user.h"

int
main (int argc, char *argv[]){
  int trn_arnd_time=-1, run_time=-1;
  int i=0, rc, N=-1;

  for (i=2; i < argc; i++)
    printf(1, "Unused argv[%d]: %s\n", i, (char*)argv[i]);
  N = atoi(argv[1]); // defaults to 0

  printf(1, "N: %d\n", N); 

  printf(1, "Original P: %d\n", getpid());
  printf(1, "Creating First fork %d\n", i);  
  for(i=0; i < 3; i++){
    rc = fork();
    if (rc < 0)
      printf(1, "Failed to Fork\n");
    else if (rc == 0){ // child
      sleep(100);
      printf(1, "isChild (%d)\n", getpid());
    }
    else { // parent
      printf(1, "isParent (%d)\nWAIT\n", getpid());
      printf(1, "Nums before waitstat: %d, %d\n", trn_arnd_time, run_time);
      waitstat(&trn_arnd_time, &run_time);
      printf(1, "Nums After: %d, %d\n", trn_arnd_time, run_time);
      break;
    }
  }
  exit();
}



// $ test1 5 // 1 fork
// N: 5
// Original P: 3
// Creating First fork 2
// isParent (3)
// WAIT
// Nums before waitstat: -1, -1
// isChild (4)
// PID: 0 time created: 895, time ended 997.
// Nums After: 102, 0

// $ test1 5 // 2 forks
// N: 5
// Original P: 3
// Creating First fork 2
// isParent (3)
// WAIT
// Nums before waitstat: -1, -1
// isChild (4)
// isParent (4)
// WAIT
// Nums before waitstat: -1, -1
// isChild (5)
// PID: 0 time created: 5441, time ended 5588.
// Nums After: 147, 0
// PID: 0 time created: 5153, time ended 5588.
// Nums After: 435, 35
//
// $ test1 5 // 3 forks
// N: 5
// Original P: 3
// Creating First fork 2
// isParent (3)
// WAIT
// Nums before waitstat: -1, -1
// isChild (4)
// isParent (4)
// WAIT
// Nums before waitstat: -1, -1
// isChild (5)
// isParent (5)
// WAIT
// Nums before waitstat: -1, -1
// isChild (6)
// PID: 0 time created: 992, time ended 1093.
// Nums After: 101, 0
// PID: 0 time created: 891, time ended 1093.
// Nums After: 202, 2
// PID: 0 time created: 787, time ended 1093.
// Nums After: 306, 2
//
