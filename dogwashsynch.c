/* CMPT 332 - Fall 2015
 * Assignment 3, Question 2
 *
 * Jordaen Graham - jhg257
 * Jennifer Rospad - jlr247
 *
 * File: dogwashsynch.c */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "dogwashsynch.h"
#include <pthread.h>

/* cond: signals when a bay has opened up for a waiting dog */
pthread_cond_t cond;

/* DA_mutex: locks DA_count
 * DB_mutex: locks DB_count
 * bays_avail_mutex: locks bays_avail
 * bays_mutex: ensures that only DA's **OR** DB's are washing
 * wait_mutex: waits for next chance for DA or DB dogs to wash */
pthread_mutex_t DA_mutex, DB_mutex, bays_avail_mutex, bays_mutex, wait_mutex;

/*
 * DA_count: number of DA dogs currently being washed
 * DB_count: number of DB dogs currently being washed
 * bays_avail: number of bays currently free */
int *DA_count, *DB_count, *bays_avail;

/* Start routine for a DA dog thread:
 *  increments the DA count then waits until it can get
 *  the bays_mutex (ensuring DA/DB exclusion) */
void DA_start(){
	pthread_mutex_lock(&wait_mutex);
    pthread_mutex_lock(&DA_mutex);
    *DA_count = *DA_count + 1;
    if (*DA_count == 1){
        pthread_mutex_lock(&bays_mutex);
        printf("======= DA dogs in the bay -- no DB dogs may enter =======\n");
    }
    pthread_mutex_unlock(&wait_mutex);
    pthread_mutex_unlock(&DA_mutex);
}

/* Start routine for a DB dog thread:
 *  increments the DB count then waits until it can get
 *  the bays_mutex (ensuring DA/DB exclusion) */
void DB_start(){
    pthread_mutex_lock(&wait_mutex);
    pthread_mutex_lock(&DB_mutex);
    *DB_count = *DB_count + 1;
    if (*DB_count == 1){
        pthread_mutex_lock(&bays_mutex);
        printf("======= DB dogs in the bay -- no DA dogs may enter =======\n");
    }
    pthread_mutex_unlock(&wait_mutex);
    pthread_mutex_unlock(&DB_mutex);
}

/* Finisihing routine for a DA dog thread:
 *  decrements the number of DA dogs currently being washed, and then if it is 
 *  the last DA in a bay, it releases the bays_mutex to allow DB dogs to get washed. */
void DA_done(){
    pthread_mutex_lock(&DA_mutex);
    *DA_count = *DA_count - 1;
    if (*DA_count == 0){
        pthread_mutex_unlock(&bays_mutex);
        printf("======= DA dogs done -- DB dogs may now enter =======\n");
    }
    pthread_mutex_unlock(&DA_mutex);
}

/* Finishing routine for a DB dog thread:
 *  decrements the number of DB dogs currently being washed, and then if it is 
 *  the last DB in a bay, it releases the bays_mutex to allow DA dogs to get washed. */
void DB_done(){
    pthread_mutex_lock(&DB_mutex);
    *DB_count = *DB_count - 1;
    if (*DB_count == 0){
        pthread_mutex_unlock(&bays_mutex);
        printf("======= DB dogs done -- DA dogs may now enter =======\n");
    }
    pthread_mutex_unlock(&DB_mutex);
}

/* Initializes the dogwash system */
int dogwash_init(int numbays) {
    int rv;
    
    /* Check numBays */
    if(0 >= numbays){
        printf("The number of bays must be a positive integer.\n");
        return -1;
    }

    /* Initialize condition signaller */
	rv = pthread_cond_init(&cond, NULL);
	if (0 != rv){
        fprintf(stderr, "ERROR: Out of memory. Could not allocate cond.\n");
        return -1;
    }
	
    /* Initialize counters */
    DA_count = (int *)malloc(sizeof(int));
    if (0 == DA_count){
        fprintf(stderr, "ERROR: Out of memory. Could not allocate DA_count.\n");
        return -1;
    }
    *DA_count = 0;

    DB_count = (int *)malloc(sizeof(int));
    if (0 == DB_count){
        fprintf(stderr, "ERROR: Out of memory. Could not allocate DB_count.\n");
        return -1;
    }
    *DB_count = 0;
        
    bays_avail = (int *)malloc(sizeof(int));
    if (0 == bays_avail){
        fprintf(stderr, "ERROR: Out of memory. Could not allocate bays_avail.\n");
        return -1;
    }
    *bays_avail = numbays;
    
    /* Initialize locks */
    rv = pthread_mutex_init(&bays_mutex, NULL);
    if (0 != rv){
        fprintf(stderr, "ERROR: Out of memory. Could not allocate bays_mutex.\n");
        return -1;
    }

    rv = pthread_mutex_init(&bays_avail_mutex, NULL);
    if (0 != rv){
        fprintf(stderr, "ERROR: Out of memory. Could not allocate bays_avail_mutex.\n");
        return -1;
    }

    rv = pthread_mutex_init(&DA_mutex, NULL);
    if (0 != rv){
        fprintf(stderr, "ERROR: Out of memory. Could not allocate DA_mutex.\n");
        return -1;
    }

    rv = pthread_mutex_init(&DB_mutex, NULL);
    if (0 != rv){
        fprintf(stderr, "ERROR: Out of memory. Could not allocate DB_mutex.\n");
        return -1;
    }

    rv = pthread_mutex_init(&wait_mutex, NULL);
    if (0 != rv){
        fprintf(stderr, "ERROR: Out of memory. Could not allocate wait_mutex.\n");
        return -1;
    }

    return 0;
}

/* Function to initialize a new dog thread */
int newdog(dogtype dog){
    printf("Dog, %s Arrived\n", DA == dog ? "A" : DB == dog ? "B" : "O");

    /* Run special start routine if a DA or DB dog */
    if (DA == dog)
        DA_start();
    else if (DB == dog)
        DB_start();

    /* Add dog to bay to be washed when bay becomes available */
    pthread_mutex_lock(&bays_avail_mutex);
	while(0 == *bays_avail)	{
		pthread_cond_wait(&cond, &bays_avail_mutex); /* wait for signal from cond */
	}
    *bays_avail = *bays_avail - 1;
    printf("\nDog, %s washing.\nBays Remaining: %d\n\n", 
            DA == dog ? "A" : DB == dog ? "B" : "O", *bays_avail);
    pthread_mutex_unlock(&bays_avail_mutex);

    return 0;
}


/* Function to end a dog thread */
int dogdone(dogtype dog) {
    /* increment the number of bays available */
    pthread_mutex_lock(&bays_avail_mutex);
    *bays_avail = *bays_avail + 1;
    printf("\nFinished dog %s.\nBays avail: %d\n\n", 
            DA == dog ? "A" : DB == dog ? "B" : "O", *bays_avail);
	pthread_cond_signal(&cond);  //wake up waiting thread with condition variable
    pthread_mutex_unlock(&bays_avail_mutex);    

    /* do the appropriate end-tasks for threads of DA or DB type */
    if ( DA == dog ){
      DA_done();
    }else if ( DB == dog ){
      DB_done();
    }

    /* end the thread */
    pthread_exit(NULL);
    return 0;
}

/* Function to close the dogwash system */
int dogwash_done() {
    int rv;
    printf("Pthread dogwash done\n");

    /* destroy/free dynamically allocated variables in system */
    rv = pthread_cond_destroy(&cond);
    if (0 != rv){
        fprintf(stderr, "Failed to destroy cond.\n");
        return -1;
    }

    free(DA_count);
    DA_count = NULL;
    
    free(DB_count);
    DB_count = NULL;
    
    free(bays_avail);
    bays_avail = NULL;

    rv = pthread_mutex_destroy(&bays_mutex);
    if (0 != rv){
        fprintf(stderr, "Failed to destroy bays_mutex.\n");
        return -1;
    }

    rv = pthread_mutex_destroy(&bays_avail_mutex);
    if (0 != rv){
        fprintf(stderr, "Failed to destroy bays_avail_mutex.\n");
        return -1;
    }

    rv = pthread_mutex_destroy(&DA_mutex);
    if (0 != rv){
        fprintf(stderr, "Failed to destroy DA_mutex.\n");
        return -1;
    }

    rv = pthread_mutex_destroy(&DB_mutex);
    if (0 != rv){
        fprintf(stderr, "Failed to destroy DB_mutex.\n");
        return -1;
    }

    rv = pthread_mutex_destroy(&wait_mutex);
    if (0 != rv){
        fprintf(stderr, "Failed to destroy wait_mutex.\n");
        return -1;
    }

    return 0;
}

