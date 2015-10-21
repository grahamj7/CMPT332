#include "stdio.h"
#include "stdlib.h"
#include "sys/mman.h"
#include "A2_3.h"

int init = 0;

node_th *head, *tail; /* Pointers to head and tail of free list */
void *start_addr, *end_addr; /* Pointers to starting header and final 
                                footer of initialized memory */


int M_INIT(int size){

  /* Check that memory is not already initialized */
  if (init)
    return -1;
  init = 1;
  
  /* Set default memory size if user enters invalid int */
  if (size < 1){
    fprintf(stdout, "You entered a size of %d, defaulted to 1024\n", size);
    size = 1024;
  }
  
  /* Calculate size of memory to initialize */
  int allocate_space = size + sizeof(node_th) + sizeof(node_tf);
  int rem = allocate_space % 16;
  if (0 != rem)
    allocate_space = allocate_space + (16 - rem); /* make space to be alloc a multiple of 16 */
    
  printf("allocating %d bytes of space\n", allocate_space);
  
  /* Call mmap to map this section of memory */
  head = mmap(NULL, allocate_space,PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, -1, 0);
  if (NULL == head) {
    fprintf(stderr, "mmap has failed!");
    return -1;
  }
  
  /* Save start of top header, initialize header, set tail */
  start_addr = head;
  head->size = size;
  head->next = NULL;
  head->is_alloc = FREE;
  tail = head;
  
  /* Set initial tail for entire mapped memory,
  and save address of that final footer */
  node_tf *footer;
  footer = (void*) head + head->size + sizeof(node_th);
  footer->size = head->size;
  footer->prev =  NULL;
  footer->is_alloc = FREE;
  end_addr = footer;

  printf("Head/Tail: %p, next: %p, size: %d\n", head, head->next, head->size);
  printf("Footer: %p, prev: %p, size: %d\n\n\n", footer, footer->prev, footer->size);
  return 0;
}


void *alloc(node_th *header, int size){

  node_th *nexthead; /* Pointer to header of next chunk in free memory */
  node_tf *footer, *prevfoot;   /* Pointer to footer of current chunk of free memory, 
                                  and Pointer to footer of previous chunk in free memory */
  node_th* prevhead = NULL; /* Pointer to header of previous chunk in free memory */
  node_tf* nextfoot = NULL; /* Pointer to footer of next chunk in free memory */
  node_th *allocated_header, *free_header; /* Current header and header of free 
                                              chunk made by splitting */
  node_tf *allocated_footer, *free_footer; /* Current footer and footer of free 
                                              chunk made by splitting */


  /* Calculate footer, and save references to previous and next free memory chunks */
  footer = (void*)header + header->size + sizeof(node_th);
  printf("Header: %p, size: %d, Footer: %p, size: \n", header, header->size, footer);
  nexthead = header->next;
  prevfoot = footer->prev;

  /* Get previous free mem chunk's header */
  if( NULL != prevfoot) {
    prevhead = (void*) prevfoot - (prevfoot->size) - sizeof(node_th);
    if( ALLOCATED == prevhead->is_alloc ) {// Double check not alloc
      return NULL;
    }
  }
  
  /* Get next free mem chunk's footer */
  if( NULL != nexthead) {
    nextfoot = (void*) nexthead + sizeof(node_th) + nexthead->size;
    if( ALLOCATED == nextfoot->is_alloc ){ // Double check not alloc
      return NULL;
    }
  }

  printf("PrevHead: %p, NextFoot: %p\n", prevhead, nextfoot);

  /* If size of space to be allocated fits into free chunk perfectly: */
  if (size == header->size){
    header->is_alloc = ALLOCATED;
    footer->is_alloc = ALLOCATED;
    header->next = NULL;
    footer->prev = NULL;
    if (head == tail){ /* If filling the last remaining chunk, 
                          there is no more free list */
      head = NULL;
      tail = NULL;
    }
    /* If there is a prev or next chunk, correctly link them together */
    if (NULL != prevhead)
      prevhead->next = nexthead;
    else {
      if(head == header)
        head = nexthead;
    }
    if (NULL != nextfoot)
      nextfoot->prev = prevfoot;
      
  }
  
  /* If size of space to alloc does NOT equal the free chunk size, split */
  else {
    /* Footer of new, smaller free chunk is the old footer;
       reset the fields accordingly */
    free_footer = footer;
    
    free_footer->size = footer->size - size - sizeof(node_tf) - sizeof(node_th);
    free_footer->is_alloc = FREE;
    free_footer->prev = prevfoot;
  
    free_header = (void*) header + sizeof(node_th) + size + sizeof(node_tf);
    free_header->is_alloc = FREE;
    free_header->size = free_footer->size;
    free_header->next = nexthead;
  
    /* Allocated header is old header; adjust fields accordingly */
    allocated_header = header;
    allocated_footer = (void*) allocated_header + sizeof(node_th) + size;
    allocated_header->next = NULL;
    allocated_footer->prev = NULL;
    allocated_header->is_alloc = ALLOCATED;
    allocated_footer->is_alloc = ALLOCATED;
    allocated_header->size = size;
    allocated_footer->size = size;

    /* Update tail to newly split free chunk if we were at the tail */
    if (NULL == nexthead){
      tail = free_header;
    }
    
    /* If we were at the front of the free list, set head to new free_header */
    if (NULL == prevfoot)
      head = free_header;
  }
  
  // shows error in header->size
    
  printf("A_Header: %p, size: %d\nA_Footer: %p, size: %d\n", allocated_header, allocated_header->size, allocated_footer, allocated_footer->size);

  printf("F_Header: %p, size: %d\nF_Footer: %p, size: %d\n", free_header, free_header->size, free_footer, free_footer->size);

  return ((void*) allocated_header + sizeof(node_th));

}


void *M_ALLOC(int size){
  node_th *cursor; /* Travels through list of free chunks to find 
                      one of the right size */

  /* Check that size to be allocated is a valid integer */
  if (size < 1) {
    fprintf(stderr, "You attempted to allocate an invalid size, %d", size);
    return NULL;
  }
  
  /* Ensure chunk of mem being allocated is multiple of 16 */
  int new_size = size + sizeof(node_th) + sizeof(node_tf);
  int rem = new_size % 16;
  if (0 != rem){
    size = size + (16 - rem); /* make space to alloc a multiple of 16 */
  }
  
  /* Loop through free list looking for available piece of memory of correct size */
  cursor = head;
  while (size > cursor->size) {
    /* If no free space large enough, return null */
    if (NULL == cursor->next){
      return NULL;
    }
    cursor = cursor->next;
  }  

  return alloc(cursor, size);
}


int M_FREE(void *ptr){
  node_th *header = (void*)ptr - sizeof(node_th); /* Header of chunk ptr is in */
  printf("Gets to here before failing! ptr: %p header: %p\n", ptr, header);
  //If I try to print header->size above, it fails there. For some reason, can't access header->size
  //Also, in our test case, header location SHOULD be equal to head location, but its NOT. Its off by 8.
  node_tf *footer = (void*)ptr + header->size; /* Footer of chunk prt is in */
  
  /* Check that we are not trying to free free space */
  if (FREE == header->is_alloc)
    return -1;
  
  header->is_alloc = FREE;
  footer->is_alloc = FREE;
  
  /* Add to freelist */
  /* If free list is empty, make this chunk the whole free list */
  if (NULL == head){
    head = header;
  } else {
    // Make new free chunk the tail
    tail->next = header;
    footer->prev = (void*)tail + tail->size + sizeof(node_th);
  }
  tail = header;
  tail->next = NULL;
   
  /* Coalesce: add free chunks to free list 
  node_tf *prevfoot = NULL, *nextfoot = NULL; // Footer of prev and next memory chunks 
  node_th *nexthead = NULL, *prevhead = NULL; // Header of next and prev memory chunks 
  
  if (header != start_addr)
    prevfoot = (void*)header - sizeof(node_tf);
  if (footer != end_addr)
    nexthead = (void*)footer + sizeof(node_tf);
  
  if (NULL != prevfoot){
    if (FREE == prevfoot->is_alloc){
      prevhead = (void*)prevfoot - prevfoot->size - sizeof(node_th);
      prevhead->size = prevhead->size + header->size + sizeof(node_th) + sizeof(node_tf);
      footer->size = prevhead->size;
      printf("footer-prev : %p, prev-foot-prev %p\n", footer->prev, prevfoot->prev);
      footer->prev = prevfoot->prev;
    }
  }
  if (NULL != nexthead){
    if (FREE == nexthead->is_alloc){
      nextfoot = (void*)nexthead + nexthead->size + sizeof(node_th);
      nextfoot->size = nextfoot->size + footer->size + sizeof(node_th) + sizeof(node_tf);
      header->size = nextfoot->size;
      header->next = nexthead->next;
    }
  }*/
  return 0;
}


void M_DISPLAY(){
  node_th *cursor;
  fprintf(stdout, "Free Chunks: [\n");
  cursor = (node_th*) head;
  while (NULL != cursor){
    if (FREE == cursor->is_alloc)
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
