
#ifndef A2_3_HEADER
#define A2_3_HEADER

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


int M_INIT(int);
void *M_ALLOC(int);
int M_FREE(void *);
void M_DISPLAY();


#endif
