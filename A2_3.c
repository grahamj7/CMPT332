#include "stdio.h"
#include "stdlib.h"
#include "sys/mman.h"
#include "A2_3.h"
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
node_th *tail; 

int M_INIT(int size){

  if (init)
    return -1;
  init = 1;
  if (size < 1){
    printf("You entered a size of %d, defaulted to 4096\n", size);
    size = 4096;
  }
  
  /* mmap() returns a pointer to a chunk of free space */
  head = mmap(NULL, size + sizeof(node_th) + sizeof(node_tf),PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, -1, 0);
  if (NULL == head) {
    fprintf(stderr, "mmap has failed!");
    return -1;
  }
  head->size = size;
  head->next = NULL;
  head->magic_num = 0;
  tail = head;
  
  node_tf *footer = head + head->size + sizeof(head);
  footer->size = head->size;
  footer->prev =  NULL;
  footer->magic_num = 0;

  printf("Head: %p, next: %p, size: %d\n", head, head->next, head->size);
  printf("Tail: %p, next: %p, size: %d\n", tail, tail->next, tail->size);
  printf("Footer: %p, prev: %p, size: %d\n", footer, footer->prev, footer->size);
  return 0;
}

void *alloc(node_th *header, int size){
  node_th *next = header->next;
  node_tf *footer = header + header->size + sizeof(header);
  node_tf* prev = footer->prev;
  node_th* prevhead = NULL;
  node_tf* nextfoot = NULL;
  node_th *new_header;
  
  printf("TAIL: %p, HEAD: %p\n, Header: %p, Next: %p\n, Footer: %p, Prev: %p\n", tail, head, header, next, footer, prev);
  
  if( NULL != prev) {
    printf("hi\n");
    printf("prev ptr: %p. prev size: %d, size of header: %d\n", prev, prev->size, sizeof(header));
    prevhead = (node_th*) prev - (prev->size) - sizeof(header);
    printf("prevhead: %p\n", prevhead);
    if( MAGIC_NUM == prevhead->magic_num ) {// Double check not alloc
      printf("NULL 1!\n");
      return NULL;
    }
  }

  if( NULL != next) {
    printf("next\n");
    nextfoot = (node_tf*) next + sizeof(header) + next->size;
    if( MAGIC_NUM == nextfoot->magic_num ){ // Double check not alloc
      printf("NULL 2!\n");
      return NULL;
    }
  }
  printf("check\n");
  if (size == header->size){
    printf("HERE!\n");

    header->magic_num = MAGIC_NUM;
    footer->magic_num = MAGIC_NUM;
    header->next = NULL;
    footer->prev = NULL;
    if (head == tail){
      head = NULL;
      tail = NULL;
    }
    if (NULL != prevhead)
      prevhead->next = next;
    if (NULL != nextfoot)
      nextfoot->prev = prev;
  }  //TODO: Make 16 byte boundary rule enforced
  else{
    //split_it_up();
    printf("Split\n");
    header->magic_num = MAGIC_NUM;
    header->size = size;
    header->next = NULL;

    
    int temp = footer->size - size - sizeof(node_th*) - sizeof(node_tf*); // remaining size available
    printf("%d: %d: %d: %d\n", footer->size, size, sizeof(node_th), sizeof(node_tf));
    footer->size = temp;
    footer->prev = NULL;


    node_tf *new_footer = header + sizeof(header) + size;
    new_footer->size = size;
    new_footer->magic_num = MAGIC_NUM;
    new_footer->prev = NULL;

    
    new_header = new_footer + sizeof(node_tf);
    new_header->next = next;
    new_header->magic_num = 0;
    new_header->size = footer->size;

    
    if (NULL != prevhead)
      prevhead->next = new_header;
    else
      head = new_header;

    if (NULL != nextfoot)
      nextfoot->prev = footer;
  }
  printf("HEAD: %p, size: %d\n", head, head->size);
  printf("PTR: %p\n", ((void*)header + sizeof(header)));
  node_th *ptr = ((void*)header + sizeof(header));
  node_tf *prt_f = ptr + header->size;
  printf("header: %p, footer: %p, prev: %p\n", new_header, footer, footer->prev);
  return ptr;
}

void *M_ALLOC(int size){
  node_th *cursor;
  void *ptr = NULL;
  if (size < 1) {
    fprintf(stderr, "You attempted to allocate an invalid size, %d", size);
    return NULL;
  }
  cursor = head;
  while (size > cursor->size) {
    if (NULL == cursor->next){
      printf("CURSOR!\n");

      return NULL;
    }
    cursor = cursor->next;
  }  
  ptr = alloc(cursor, size);
  return ptr;
}

int M_FREE(void *ptr){
  printf("1\n");
  node_th *header = ptr - sizeof(node_th*);
  printf("2\n");
  node_tf *footer = (node_tf*) ptr + header->size;
  printf("3\n");
  
  if (MAGIC_NUM != header->magic_num)
    return -1;
  
  header->magic_num = 0;
  footer->magic_num = 0;
  munmap(ptr, header->size);
  
  if (NULL == head){
    head = header;
    tail = header;
  } else {
    tail->next = header;
    footer->prev = (node_tf*) tail + tail->size;

    tail = header;
    tail->next = NULL;
  }
  //add to freelist
  
  //coalesce
  
  return 0;
}

void M_DISPLAY(){
  node_th *cursor;
  printf("Free Chunks: [\n");
  cursor = (node_th*) head;
  while (NULL != cursor){
    if (MAGIC_NUM != cursor->magic_num)
      printf("\tptr: %p, size: %d\n", cursor, cursor->size);
 
    cursor = cursor->next;
  }
  printf("\n]\n");
}

int main(int argc, char **argv){
  M_INIT(1024);
  printf("MALLOC!\n");
  void *ptr1 = M_ALLOC(128);
  printf("MALLOC!\n");
  void *ptr2 = M_ALLOC(64);
  printf("FREE!\n");
  M_FREE(ptr1);
  printf("MALLOC!\n");
  void *ptr3 = M_ALLOC(32);
  printf("DISPLAY!\n");
  M_DISPLAY();
  printf("FREE!\n");
  M_FREE(ptr2);
  printf("DISPLAY!\n");
  M_DISPLAY();
  printf("FREE!\n");
  M_FREE(ptr3);
  printf("DISPLAY!\n");
  M_DISPLAY();
  return 0;
}
