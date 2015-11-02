//
// Created by Jordaen.Graham on 15-10-31.
//

#ifndef ASSIGNMENT_3_MAIN_H
#define ASSIGNMENT_3_MAIN_H

#define NUMBAYS 5
typedef enum {DA, DB, DO} dogtype;

int dogwash_init(int);
void *newdog(void*);
int dogdone(dogtype);
int dogwash_done();

#endif //ASSIGNMENT_3_MAIN_H
