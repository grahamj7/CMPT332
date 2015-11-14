/* CMPT - Fall 2015
 * Assignment 3, Question 2
 *
 * Jordaen Graham - jhg257
 * Jennifer Rospad - jlr247
 *
 * File: dogwashsynch.h */

#ifndef DOGWASHSYNCH_PTHREAD_H
#define DOGWASHSYNCH_PTHREAD_H

typedef enum {DA, DB, DO} dogtype;

int dogwash_init(int);
int newdog(dogtype);
int dogdone(dogtype);
int dogwash_done();

#endif /*DOGWASHSYNCH_H*/

