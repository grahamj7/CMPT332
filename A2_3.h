
#ifndef A2_3_HEADER
#define A2_3_HEADER

#define ALLOCATED 1
#define FREE 0

typedef struct __node_th{
  int size;
  int is_alloc;
  struct __node_th *next;
}node_th;

typedef struct __node_tf{
  int size;
  int is_alloc;
  struct __node_tf *prev;
}node_tf;


int M_INIT(int);
void *M_ALLOC(int);
int M_FREE(void *);
void M_DISPLAY();


#endif
