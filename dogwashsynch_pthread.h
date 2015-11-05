#ifndef DOGWASHSYNCH_PTHREAD_H
#define DOGWASHSYNCH_PTHREAD_H

typedef enum {DA, DB, DO} dogtype;


int dogwash_init(int);
int newdog(dogtype);
int dogdone(dogtype);
int dogwash_done();

#endif /*DOGWASHSYNCH_PTHREAD_H*/
