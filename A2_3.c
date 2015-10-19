#include "stdio.h"
#include "stdlib.h"
#include "sys/mman.h"
#define MAGIC_NUM 1234567
int init = 0;

typedef struct __header_t{
  int size;
  int magic;
}header_t;

typedef struct __node_t{
  int size;
  struct __node_t *next;
}node_t;

node_t *head;

int M_INIT(int size){

  if (init)
    return -1;
  init = 1;
  if (size < 1){
    printf("You entered a size of %d, defaulted to 4096\n", size);
    size = 4096;
  }
  /* mmap() returns a pointer to a chunk of free space */
  head = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, -1, 0);
  head->size = size - sizeof(node_t);
  head->next = NULL;

  return 0;
}

void *M_ALLOC(int size){
  header_t *new_header;
  if (size < 1) {
    printf("You attempted to allocate an invalid size, %d", size);
    return NULL;
  }
  if (size > head->size) {
    printf("There's not enough memory left to give you,  %d bytes", size);
    return NULL;
  }
  new_header->size = size;
  new_header->magic = MAGIC_NUM;
  
  
}

int M_FREE(void *ptr){
  header_t *hptr = (void*)ptr-sizeof(header_t);
/*  assert(hptr->magic == MAGIC_NUM); */
  free(hptr); /*??*/
  
  return 0;
}

void M_DISPLAY(){
  node_t *cursor;
  printf("Allocated Chunks: ");
  cursor = (node_t*) head;
  while (cursor != NULL){
/*    printf("%s, ", cursor); */
    cursor = cursor->next;
  }
  printf("\n");
}

int main(int argc, char **argv){
  printf("M_INIT: %d\n", M_INIT(1024));
  M_DISPLAY();
  return 0;
}
