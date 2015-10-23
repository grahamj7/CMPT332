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
    fprintf(stdout, "You entered an invalid memory size of %d\n", size);
    return -1;
  }
  
  /* Calculate size of memory to initialize */

  int rem = (size + sizeof(node_th) + sizeof(node_tf)) % 16;
  /* make space to be alloc a multiple of 16 */
  if (0 != rem)
    size = size + (16 - rem);
  
  int allocate_space = size + sizeof(node_th) + sizeof(node_tf);
  /* Call mmap to map this section of memory */
  head = mmap(NULL, allocate_space,PROT_READ|PROT_WRITE,MAP_ANON|MAP_PRIVATE, -1,0);
  if (NULL == head) {
    fprintf(stderr, "mmap has failed!");
    return -1;
  }
  
  /* Save start of top header, initialize header, set tail */
  start_addr = head;
  head->size = size;
  head->next = NULL;
  head->is_alloc = FREE;
  head->is_node = TRUE_NODE;
  tail = head;
  
  /* Set initial tail for entire mapped memory,
  and save address of that final footer */
  node_tf *footer;
  footer = (void*) head + head->size + sizeof(node_th);
  footer->size = head->size;
  footer->prev =  NULL;
  footer->is_alloc = FREE;
  footer->is_node = 0;
  end_addr = footer;

  /* Uncomment to test */
  //printf("Overall Mem Head/Tail: %p, Head->next: %p, Head->size: %d\n", head, head->next, head->size);
  //printf("Overall Mem Footer: %p, Footer->prev: %p, Footer->size: %d\n", footer, footer->prev, footer->size);
  
  return 0;
}



void *alloc(node_th *header, int size){
  /* Pointer to header of next chunk in free memory */
  node_th *nexthead;
  
  /* Pointer to footer of current chunk of free memory, 
      and Pointer to footer of previous chunk in free memory */      
  node_tf *footer, *prevfoot;
  
  /* Pointer to header of previous chunk in free memory */
  node_th* prevhead = NULL; 
  
  /* Pointer to footer of next chunk in free memory */
  node_tf* nextfoot = NULL; 
  
  /* Current header and header of free chunk made by splitting */
  node_th *allocated_header, *free_header; 
  
  /* Current footer and footer of free chunk made by splitting */
  node_tf *allocated_footer, *free_footer; 


  /* Calculate footer, and save references to previous and next free memory chunks*/
  footer = (void*)header + header->size + sizeof(node_th);
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
    free_header->is_node = TRUE_NODE;
  
    /* Allocated header is old header; adjust fields accordingly */
    allocated_header = header;
    allocated_footer = (void*) allocated_header + sizeof(node_th) + size;
    allocated_header->next = NULL;
    allocated_footer->prev = NULL;
    allocated_header->is_alloc = ALLOCATED;
    allocated_footer->is_alloc = ALLOCATED;
    allocated_header->size = size;
    allocated_footer->size = size;
    allocated_footer->is_node = 0;

    /* Update tail to newly split free chunk if we were at the tail */
    if (NULL == nexthead){
      tail = free_header;
    }
    
    /* If we were at the front of the free list, set head to new free_header */
    if (NULL == prevfoot)
      head = free_header;
  }
  
  // shows error in header->size
  
  /* Uncomment following for extra testing checks: */ 
  //printf("Alocated chunk's header: %p, size: %d\n", allocated_header, allocated_header->size);
  //printf("Allocated chunk's footer: %p, size: %d\n",  allocated_footer, allocated_footer->size);

  //printf("Leftover free chunk's header: %p, size: %d\n", free_header, free_header->size);
  //printf("Leftover free chunk's footer: %p, size: %d\n", free_footer, free_footer->size);

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
      printf("Not enough free space for allocation request.\n");
      return NULL;
    }
    cursor = cursor->next;
  }  

  return alloc(cursor, size);
}


int M_FREE(void *ptr){
	/* Header of chunk ptr is in */
  node_th *header = (void*)ptr - sizeof(node_th); 
  
  /* Check this is a valid address to free */
  if (((void*)header) < start_addr || ((void*)header) > end_addr || TRUE_NODE != header->is_node){
    fprintf(stderr, "Invalid address\n");
    return -1;
  }

  /* Footer of chunk ptr is in */
  node_tf *footer = (void*)ptr + header->size; 
  
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
   
  /* Coalesce: add free chunks to free list */
  node_tf *prevfoot = NULL, *nextfoot = NULL; // Footer of prev and next memory chunks 
  node_th *nexthead = NULL, *prevhead = NULL; // Header of next and prev memory chunks 
  
  if (header != start_addr) // if header is at the beginning of our address space, there is no prev footer
    prevfoot = (void*)header - sizeof(node_tf);
    
  if (footer != end_addr)  // if footer is at the end of our address space, there is no next header
    nexthead = (void*)footer + sizeof(node_tf);
  
  if (NULL != prevfoot){
    if (FREE == prevfoot->is_alloc){
      prevhead = (void*)prevfoot - prevfoot->size - sizeof(node_th);
      prevhead->size = prevhead->size + header->size + sizeof(node_th) + sizeof(node_tf);
      footer->size = prevhead->size;
      footer->prev = prevfoot->prev;
      tail = prevhead;
      tail->next = NULL;
      header->is_node = 0;
      header = prevhead;
    }
  }
  if (NULL != nexthead){
    if (FREE == nexthead->is_alloc){
      nextfoot = (void*)nexthead + nexthead->size + sizeof(node_th);
      nextfoot->size = nextfoot->size + footer->size + sizeof(node_th) + sizeof(node_tf);
      header->size = nextfoot->size;
      header->next = NULL;
      nexthead->is_node = 0;
      if (head == nexthead){
        head = header;
      }
    }
  }
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
  fprintf(stdout, "]\n");
}
