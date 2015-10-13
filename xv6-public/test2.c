#include "types.h"
#include "stat.h"
#include "user.h"

int
main (int argc, char *argv[]){
  int trn_arnd_time=-1, run_time=-1, j = 20; /* Number of child processes */
  int trn_arnd_time_arr[j], sum_trn_arnd_time;
  int run_time_arr[j], sum_run_time;
  int x, rc, N, i, k, m;
  double sum_child = 0.0, sum_parent = 0.0, C = 0.0;

  /* Check that user did not enter more than one argument for N */
  for (i=2; i < argc; i++)
    printf(1, "Unused argv[%d]: %s\n", i, (char*)argv[i]);
  N = atoi(argv[1]); // defaults to 0 if non-integer
  if( 0 >= N ){
    printf(1, "Please enter an integer greater than zero. %s is not a valid N.\n", argv[1]);
    exit();
  }
    
  
  /* Create 20 forks (j holds value of 20) */
  for(x=0; x < j; x++){
    rc = fork();
    if (rc < 0){
      printf(1, "Failed to Fork\n");
      exit();
    }
    else if (rc == 0){ // child
      for (k = 1; k <= 10; k++){
        C = 1.0/k;
        sum_child = 0.0;
        for (i = 1; i<( (21 - x)*N ); i++){
          sum_child = sum_child + (1.0/(i/0.9 + C));
        }
        //TODO: printing doubles correctly
        printf(1, "The sum of the %dth sum of the %dth child is = %d\n", k, x + 1, sum_child);
      }
      exit();
    }
    else { // parent
      if(waitstat(&trn_arnd_time, &run_time) < 0){s
        printf(1, "waitstat() failed.");
        break;
      } 
      trn_arnd_time_arr[x] = trn_arnd_time;
      run_time_arr[x] = run_time;
      sum_trn_arnd_time += trn_arnd_time;
      sum_run_time += run_time;
      sum_parent = 0.0;
      for(m = 1; m < 5*N; m++){
        sum_parent = sum_parent + (1.0/((m/0.9) + (1.0/x)));
      }
      //TODO: printing doubles correctly
      printf(1, "The %dth sum of the parent is = %d\n\n", x + 1, sum_parent);
    }
  }
  /* At end of program, display all child turn 
  around and run times, then the average for each.*/
  for (x = 0; x < j; x++){
    printf(1, "For child %d: Turn Around Time: %d Run Time: %d\n", x+1, trn_arnd_time_arr[x], run_time_arr[x]);
  }
  // TODO: change averages to print doubles
  printf(1, "\nAverage Turn Around Time: %d, Average Run Time: %d\n", sum_trn_arnd_time/j, sum_run_time/j);
  exit();
}
s
