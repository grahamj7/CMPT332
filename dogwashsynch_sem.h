#ifndef DOGWASHSYNCH_SEM_H
#define DOGWASHSYNCH_SEM_H

typedef enum {DA, DB, DO} dogtype;


int dogwash_init(int);
int newdog(dogtype);
int dogdone(dogtype);
int dogwash_done();

#endif //DOGWASHSYNCH_SEM_H
