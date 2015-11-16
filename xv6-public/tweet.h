/* CMPT 322 - Fall 2015
 * Assignment 3, Question 1
 * Jennifer Rospad - jlr247
 * Jordaen Graham - jhg257
 *
 * File: tweet.h */

#include "types.h"
#include "defs.h"
#include "param.h"
#include "spinlock.h"

#define taglength 10
#define tweetlength 140

typedef char tweet[tweetlength + 1];

struct tag_struct {
    char tag[taglength + 1];
    tweet tweets[maxtweetsametag];
    int numtweets;
    struct spinlock taglock;
    int putwaiter;
    int getwaiter;
};

typedef struct tag_struct tag_struct;

struct tw_table {
    tag_struct tag_list[maxtweettotal];
};

int t_bput(char*, char*);
int t_put(char*, char*);
int t_bget(char*, char*);
int t_get(char*, char*);
void t_print_table(void);

