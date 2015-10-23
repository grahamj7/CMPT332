#include "stdio.h"
#include <stdlib.h>
#include "A2_3.h"

int main(){
  /* Pointers used for test multiple memory allocations/frees */
  void *ptr1, *ptr2, *ptr3;
  
  printf("\n\nStarting Main (initalized with 1024B):\n\n");
  
  
  if(-1 == M_INIT(1024)){
    printf("M_INIT failed.\n");
    return EXIT_FAILURE;
  }


  printf("\nMALLOC 200B\n");
  ptr1 = M_ALLOC(200);
  if(NULL == ptr1){
    printf("MALLOC(200) failed.\n");
    return EXIT_FAILURE;
  }


  printf("\nMALLOC 370B\n");
  ptr2 = M_ALLOC(370);
  if(NULL == ptr2){
    printf("MALLOC(370) failed.\n");
    return EXIT_FAILURE;
  }


  printf("\nFREE the 370B chunk\n");
  if(-1 == M_FREE(ptr2)){
    printf("Freeing 370B failed.\n");
    return EXIT_FAILURE;
  }
    
    
  printf("\nMALLOC 15B\n");
  ptr3 = M_ALLOC(15);
  if(NULL == ptr3){
    printf("MALLOC(15) failed.\n");
    return EXIT_FAILURE;
  }

  printf("\nDISPLAY free chunks\n");
  M_DISPLAY();


  printf("\nFREE the 200B chunk\n");
  if(-1 == M_FREE(ptr1)){
    printf("Freeing 200B failed.\n");
    return EXIT_FAILURE;
  }


  printf("\nDISPLAY free chunks\n");
  M_DISPLAY();


  printf("\nFREE the 15B chunk\n");
  if(-1 == M_FREE(ptr3)){
    printf("Freeing 15B failed.\n");
    return EXIT_FAILURE;
  }


  printf("\nDISPLAY free chunks\n");
  M_DISPLAY();

  return EXIT_SUCCESS;
}

