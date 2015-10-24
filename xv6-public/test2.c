#include "types.h"
#include "stat.h"
#include "user.h"

int
main (int argc, char *argv[]){
  int sum_trn_arnd_time, sum_run_time, trn_arnd_time=-1, run_time=-1;
  int x, rc, N = 0, i, k, max = 10, j = 20; /* Number of child processes */
  int run_time_arr[j], trn_arnd_time_arr[j];
  char* num_str_in;
  double sum_child = 0.0, C = 0.0;
  int int_part, first_dig;

  /* Check that user did not enter more than one argument for N */
  if(argc > 1){
    printf(1, "Useage: test2 <no parameters>\n");
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
      while(1){
        printf(1, "Give me a single positive N (of 10 digits or less)!: ");
        num_str_in = gets(num_str_in, max);
        N = atoi(num_str_in); // defaults to 0 if non-integer
        if( 0 >= N )
          printf(1, "Enter valid integer > 0.\n");
        else
          break;     
      }
      /* Do some calculation to use time */ 
      for (k = 1; k <= 10; k++){
        C = 1.0/k;
        sum_child = 0.0;
        for (i = 1; i<( (21 - x)*N ); i++){
          sum_child = sum_child + (1.0/(i/0.9 + C));
        }
      }
      exit();
    }
    else { // parent
      if(waitstat(&trn_arnd_time, &run_time) < 0){
        printf(1, "waitstat() failed.");
        break;
      } 
      trn_arnd_time_arr[x] = trn_arnd_time;
      run_time_arr[x] = run_time;
      sum_trn_arnd_time += trn_arnd_time;
      sum_run_time += run_time;
      printf(1, "Parent has stored the reult for child #%d.\n", x);
    }
  }
  /* At end of program, display all child turn 
  around and run times, then the average for each.*/
  for (x = 0; x < j; x++){
    printf(1, "For child %d: Turn Around Time: %d Run Time: %d\n", x+1, trn_arnd_time_arr[x], run_time_arr[x]);
  }

  double avg_trn_time = sum_trn_arnd_time/j, avg_run_time = sum_run_time/j;
  int_part = (unsigned int) avg_trn_time;
  first_dig = (unsigned int) (avg_trn_time*10) - (10*int_part);
  printf(1, "\nAverage Turn Around Time: %d.%d, ", int_part, first_dig);
  
  int_part = (unsigned int) avg_run_time;
  first_dig = (unsigned int) (avg_run_time*10) - (10*int_part);
  printf(1, "Average Run Time: %d.%d\n", int_part, first_dig);
  
  exit();
}


