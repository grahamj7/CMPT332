#include "stdio.h"
#include "stdlib.h"
#include "sys/mman.h"
#define MAGIC_NUM 1234567
int init = 0;


typedef struct __node_th{
  int size;
  int magic_num;
  struct __node_th *next;
}node_th;

typedef struct __node_tf{
  int size;
  int magic_num;
  struct __node_tf *prev;
}node_tf;

node_th *head;
node_tf *foot; 

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
  if (head == NULL) {
    fprintf(stderr, "mmap has failed!");
    return -1;
  }
  head->size = size - sizeof(node_th) - sizeof(node_tf); /* size - head - foot*/
  head->next = NULL;
  foot = (node_tf*) head + head->size;
  foot->size = head->size;
  foot->prev =  NULL;

  printf("Head: %p, next: %p, size: %d\n", head, head->next, head->size);
  printf("Foot: %p, prev: %p, size: %d\n", foot, foot->prev, foot->size);
  return 0;
}

void *alloc(node_th *header, int size){
  node_th *next = header->next;
  node_tf *footer = header->size + sizeof(header) + size;
  node_tf* prev = footer->prev;
  
  if (size == header->size)
    
  else
    split_it_up();
   return NULL;
}

void *M_ALLOC(int size){
  node_th *cursor;
  void *ptr;
  if (size < 1) {
    printf("You attempted to allocate an invalid size, %d", size);
    return NULL;
  }
  cursor = head;
  while (size > cursor->size) {
    if (cursor->next == NULL)
      return NULL;
    cursor = cursor->next;
  }
  ptr = alloc(cursor, size);
  return ptr;
}

int M_FREE(void *ptr){
  header_t *hptr = (void*)ptr - sizeof(header_t);
  if (hptr->magic != MAGIC_NUM)
    return -1;
  munmap(hptr, hptr->size+sizeof(header_t));
  
  return 0;
}

void M_DISPLAY(){
  node_th *cursor;
  printf("Allocated Chunks: [\n");
  cursor = (node_th*) head;
  while (cursor != NULL){
    printf("\tptr: %p, size: %d\n", cursor, cursor->size); 
    cursor = cursor->next;
  }
  printf("\n]\n");
}

int main(int argc, char **argv){
  M_INIT(1280);
  M_ALLOC(2050);
  M_DISPLAY();
  return 0;
}
