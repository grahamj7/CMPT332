#include "stdio.h"
#include "stdlib.h"
#include "sys/mman.h"
#include "A2_3.h"

int init = 0;

node_th *head, *tail; 
void *start_addr, *end_addr;


int M_INIT(int size){

  if (init)
    return -1;
  init = 1;
  if (size < 1){
    fprintf(stdout, "You entered a size of %d, defaulted to 1024\n", size);
    size = 1024;
  }
  
  /* mmap() returns a pointer to a chunk of free space */
  int allocate_space = size + sizeof(node_th) + sizeof(node_tf);
  int rem = allocate_space % 16;
  if (0 != rem)
    allocate_space = allocate_space + rem;
  printf("allocating %d bytes of space\n", allocate_space);
  head = mmap(NULL, allocate_space,PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, -1, 0);
  if (NULL == head) {
    fprintf(stderr, "mmap has failed!");
    return -1;
  }
  start_addr = head;
  head->size = size;
  head->next = NULL;
  head->magic_num = 0;
  tail = head;
  
  node_tf *footer;
  footer = (node_tf*) head + head->size + sizeof(node_th);
  footer->size = head->size;
  footer->prev =  NULL;
  footer->magic_num = 0;
  end_addr = footer;

  printf("Head/Tail: %p, next: %p, size: %d\n", head, head->next, head->size);
  printf("Footer: %p, prev: %p, size: %d\n\n\n", footer, footer->prev, footer->size);
  return 0;
}


void *alloc(node_th *header, int size){

  node_th *next;
  node_tf *footer, *prev;
  node_th* prevhead = NULL;
  node_tf* nextfoot = NULL;
  node_th *allocated_header, *new_header;
  node_tf *allocated_footer, *new_footer;


  footer = ((node_tf*)(header) + header->size + sizeof(node_th));
  printf("Header: %p, size: %d, Footer: %p, size: \n", header, header->size, footer);
  next = header->next;
  prev = footer->prev;

  if( NULL != prev) {
    prevhead = (node_th*) prev - (prev->size) - sizeof(header);
    if( MAGIC_NUM == prevhead->magic_num ) {// Double check not alloc
      return NULL;
    }
  }
  
  if( NULL != next) {
    nextfoot = (node_tf*) next + sizeof(header) + next->size;
    if( MAGIC_NUM == nextfoot->magic_num ){ // Double check not alloc
      return NULL;
    }
  }

  printf("PrevHead: %p, NextFoot: %p\n", prevhead, nextfoot);

  if (size == header->size){
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
      
  } else{
  
    // new\free space stuff
    int footer_size = footer->size;
    new_footer = footer;
    
    new_footer->size = footer_size - size - sizeof(node_tf) - sizeof(node_th);
    new_footer->magic_num = 0;
    new_footer->prev = prev;
  
    new_header = (node_th*) (header) + sizeof(node_th) + size + sizeof(node_tf);
    new_header->magic_num = 0;
    new_header->size = new_footer->size;
    new_header->next = next;


    printf("F_Header: %p, size: %d\nF_Footer: %p, size: %d\n", new_header, new_header->size, new_footer, new_footer->size);  



    allocated_header = header;
    allocated_footer = (node_tf*) (allocated_header) + sizeof(node_th) + size;
    allocated_header->next = NULL;
    allocated_footer->prev = NULL;
    allocated_header->magic_num = MAGIC_NUM;
    allocated_footer->magic_num = MAGIC_NUM;
    allocated_header->size = size;
    allocated_footer->size = size;

    
    if (NULL == prev){
      if (head == tail)
        tail = new_header;
      head = new_header;
    }
  }
  
  // shows error in header->size
    
  printf("A_Header: %p, size: %d\nA_Footer: %p, size: %d\n", allocated_header, allocated_header->size, allocated_footer, allocated_footer->size);

  printf("F_Header: %p, size: %d\nF_Footer: %p, size: %d\n", new_header, new_header->size, new_footer, new_footer->size);


/* 
    if (head == tail){
      head = NULL;
      tail = NULL;
    }
    if (NULL != prevhead)
      prevhead->next = new_header;
    else {
      if (head == tail)
        tail = new_header;
      head = new_header;      
    }

    if (NULL != nextfoot)
      nextfoot->prev = footer;
  }
*/

  return ((void*) allocated_header + sizeof(allocated_header));

}


void *M_ALLOC(int size){
  node_th *cursor;
  void *ptr = NULL;
  if (size < 1) {
    fprintf(stderr, "You attempted to allocate an invalid size, %d", size);
    return NULL;
  }
  int new_size = size + sizeof(node_th) + sizeof(node_tf);
  int rem = new_size % 16;
  if (0 != rem){
    size = size + rem;
  }
  cursor = head;
  while (size > cursor->size) {
    if (NULL == cursor->next){
      return NULL;
    }
    cursor = cursor->next;
  }  
  ptr = alloc(cursor, size);
  return ptr;
}


int M_FREE(void *ptr){
  node_th *header = ptr - sizeof(node_th);
  node_tf *footer = (node_tf*) ptr + header->size;
  
  if (MAGIC_NUM != header->magic_num)
    return -1;
  
  header->magic_num = 0;
  footer->magic_num = 0;
  munmap(ptr, header->size);
  
  //add to freelist
  if (NULL == head){
    head = header;
    tail = header;
  } else {
    tail->next = header;
    footer->prev = (node_tf*) tail + tail->size;

    tail = header;
    tail->next = NULL;
  }
  
  //coalesce
  node_tf *prevfoot = NULL, *nextfoot = NULL;
  node_th *nexthead = NULL, *prevhead = NULL;
  
  if (header != start_addr)
    prevfoot = header - sizeof(node_tf);
  if (footer != end_addr)
    nexthead = footer + sizeof(node_tf);
  
  if (NULL == nexthead || NULL == prevfoot){
    fprintf(stderr, "Nexthead and prevfoot cannot be NULL!");
    return -1;
  }

  if (MAGIC_NUM != prevfoot->magic_num){
    prevhead = prevfoot - prevfoot->size - sizeof(node_th);
    prevhead->size = prevhead->size + header->size + sizeof(node_th) + sizeof(node_tf);
    footer->size = prevhead->size;
    footer->prev = prevfoot->prev;
  }
  if (MAGIC_NUM != nexthead->magic_num){
    nextfoot = nexthead + nexthead->size + sizeof(node_th);
    nextfoot->size = nextfoot->size + footer->size + sizeof(node_th) + sizeof(node_tf);
    header->size = nextfoot->size;
    header->next = nexthead->next;
  }
  return 0;
}


void M_DISPLAY(){
  node_th *cursor;
  fprintf(stdout, "Free Chunks: [\n");
  cursor = (node_th*) head;
  while (NULL != cursor){
    if (MAGIC_NUM != cursor->magic_num)
      fprintf(stdout, "\tptr: %p, size: %d\n", cursor, cursor->size);
 
    cursor = cursor->next;
  }
  fprintf(stdout, "\n]\n");
}


int main(int argc, char **argv){
  printf("\n\nStarting!\n\n\n\n");

  M_INIT(1024);
  
  printf("MALLOC!\n");
  void *ptr1 = M_ALLOC(128);
  
  printf("MALLOC!\n");
  void *ptr2 = M_ALLOC(64);

//  printf("FREE!\n");
//  M_FREE(ptr1);

  printf("MALLOC!\n");
  void *ptr3 = M_ALLOC(32);

//  printf("DISPLAY!\n");
//  M_DISPLAY();

//  printf("FREE!\n");
//  M_FREE(ptr2);

//  printf("DISPLAY!\n");
//  M_DISPLAY();

//  printf("FREE!\n");
//  M_FREE(ptr3);

  printf("DISPLAY!\n");
  M_DISPLAY();

  return 0;
}
