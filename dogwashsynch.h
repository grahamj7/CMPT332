#ifndef ASSIGNMENT_3_MAIN_H
#define ASSIGNMENT_3_MAIN_H

typedef enum {DA, DB, DO} dogtype;


int dogwash_init(int);
int newdog(dogtype);
int dogdone(dogtype);
int dogwash_done();

#endif /*ASSIGNMENT_3_MAIN_H*/
