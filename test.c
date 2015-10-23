#include "stdio.h"
#include "A2_3.h"

int main(){
  void *ptr1, *ptr2, *ptr3;
  printf("\n\nStarting!\n\n\n\n");
  M_INIT(1024);

  printf("MALLOC!\n");
  ptr1 = M_ALLOC(128);

  printf("MALLOC!\n");
  ptr2 = M_ALLOC(64);

  printf("FREE!\n");
  M_FREE(ptr2);

  printf("MALLOC!\n");
  ptr3 = M_ALLOC(32);

  printf("DISPLAY!\n");
  M_DISPLAY();

  printf("FREE!\n");
  M_FREE(ptr1);

  printf("DISPLAY!\n");
  M_DISPLAY();

  printf("FREE!\n");
  M_FREE(ptr3);

  printf("DISPLAY!\n");
  M_DISPLAY();

  return 0;
}

