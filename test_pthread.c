/* CMPT 332 - Fall 2015
 * Assignment 3, Question 2
 *
 * Jordaen Graham - jhg257
 * Jennifer rospad - jlr247
 *
 * File: test_pthread.c */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "dogwashsynch.h"

/* Pointer to the location of all dog threads */
pthread_t *dog_threads;

/* Dog type of the dog currently being processed */
dogtype doggy;


/* Function to initialize the dogwash test system */
int init(int numBays, int numDogs) {
    /* allocate space for the dogthreads */   
    dog_threads = (pthread_t*) malloc(numDogs * sizeof(pthread_t));
    if (dog_threads == 0){
        fprintf(stderr, "Test failed to allocate dog_threads\n");
        return -1;
    }
    
    /* create a dogwash system with the correct number of bays */
    if(0 != dogwash_init(numBays)) {
        fprintf(stderr, "Error in initializing dogwash with %i bays.\n", numBays);
        return -1;
    }

    return 0;
}

/* Function to destroy the test dogwash system */
int destroy(){
    /* free the space allocated for the dog threads */
    free(dog_threads);
    dog_threads = NULL;
    
    /* close the dogwash */
    if (0 != dogwash_done()){
        fprintf(stderr, "Dogwash_done failed to destroy the environment\n");
        return -1;
    }
    
    return 0;
}

/* Function to start, wash, and end a dog thread */
void *dog_func(void *arg){
    newdog((dogtype) arg);
    sleep(1);
    dogdone((dogtype) arg);
    return NULL;
}

/* Tests the case where there are many DB's and few DA's to ensure no starvation */
int test_DA_starve(int numBays, int numDogs){
    printf("===============================================================\n");
    printf("\tWelcome to our dog wash! \nWe currently have %d bays available "
            "today\n\n", numBays);
    printf("===============================================================\n");
    int i, rc;

    /* Initialize dogwash */
    if (0 != init(numBays, numDogs)) {
        fprintf(stderr, "Initializing test case DA_starve failed\n");
        return -1;
    }
    
    /* Create dog threads, with dogtypes mainly as DB */
    for (i = 0; i < numDogs; i++) {
        doggy = DB;
        if ((( i+1)  % 4) == 0){
            doggy = DA;
        }
        else if( ((i + 1) % 10) == 0){
            doggy = DO;
        }

        rc = pthread_create(&dog_threads[i], NULL, dog_func, (void *)doggy);
        if (rc){
            fprintf(stderr, "ERROR; return code from pthread_create() is %d\n", rc);
            return -1;
        }
    }

    /* Wait for threads to complete */
    for (i = 0; i < numDogs; i++){
        rc = pthread_join(dog_threads[i], NULL);
        if (rc){
            fprintf(stderr, "ERROR; return code from pthread_join() is %d\n", rc);
            return -1;
        }
    }
    
    /* End dogwash */
    rc = destroy();
    printf("Done Washing Dogs\n");
	return rc;
}

/* Tests the case where there are many DA's and few DB's to ensure no starvation */
int test_DB_starve(int numBays, int numDogs){
    printf("===============================================================\n");
    printf("\tWelcome to our dog wash! \nWe currently have %d bays available "
            "today\n\n", numBays);
    printf("===============================================================\n");

    int i, rc;
    
    /* Initialize dogwash */
    if (0 != init(numBays, numDogs)) {
        fprintf(stderr, "Initializing test case DB_starve failed\n");
        return -1;
    }
    
    /* Create dog threads, with dogtypes mainly as DA */
    for (i = 0; i < numDogs; i++) {
        doggy = DA;
        if (((i+1) % 4) == 0){
            doggy = DB;
        }
        else if(((i + 1) % 10) == 0){
            doggy = DO;
        }

        rc = pthread_create(&dog_threads[i], NULL, dog_func, (void *)doggy);
        if (rc){
            fprintf(stderr, "ERROR; return code from pthread_create() is %d\n", rc);
            return -1;
        }
    }

    /* Wait for threads to complete*/
    for (i = 0; i < numDogs; i++){
        rc = pthread_join(dog_threads[i], NULL);
        if (rc){
            fprintf(stderr, "ERROR; return code from pthread_join() is %d\n", rc);
            return -1;
        }
    }

    /* End dogwash */
    rc = destroy();
    printf("Done Washing Dogs\n");
	return rc;
}

/* Tests the case of randomized dogs */
int test_rand_order(int numBays, int numDogs){
    printf("===============================================================\n");
    printf("\tWelcome to our dog wash! \nWe currently have %d bays available "
            "today\n\n", numBays);
    printf("===============================================================\n");

    int i, r, rc;

    /* Initialize dog wash */
    if (0 != init(numBays, numDogs)) {
        fprintf(stderr, "Initializing test case rand_order failed\n");
        return -1;
    }
    
    /* Create dog threads with randomized dogtype */
    for (i = 0; i < numDogs; i++) {
        r = rand();
        if (0 == r % 3)
            doggy = DA;
        if (1 == r % 3)
            doggy = DB;
        if (2 == r % 3)
            doggy = DO;

        rc = pthread_create(&dog_threads[i], NULL, dog_func, (void *)doggy);
        if (rc){
            fprintf(stderr, "ERROR; return code from pthread_create() is %d\n", rc);
            return -1;
        }
    }

    /* Wait for threads to complete */
    for (i = 0; i < numDogs; i++){
        rc = pthread_join(dog_threads[i], NULL);
        if (rc){
            fprintf(stderr, "ERROR; return code from pthread_join() is %d\n", rc);
            return -1;
        }
    }
    
    /* End dogwash */
    rc = destroy();
    printf("Done Washing Dogs\n");

    return rc;
}

int main(int argc, char **argv){
    int numBays, numDogs;

    /* Check for correct usage */
    if(3 != argc){
        printf("Usage: Test_Pthread < number of bays > < number of dogs >\n");
        return -1;
    }
    
    /* Get desired number of bays and dogs for testing, and check for validity */
    numBays = atoi(argv[1]);    /* use the first arg as the number of bays*/
    numDogs = atoi(argv[2]);    /* the second arg for the number of dogs */

    if(0 >= numBays){
        printf("Invalid number of bays.\n");
        return -1;
    }
    if(0 >= numDogs){
        printf("Invalid number of dogs.\n");
        return -1;
    }
    
    /* Run tests */
    printf("\n\n******* Testing dogs of random type *******\n");
    test_rand_order(numBays, numDogs);
    
    printf("\n\n******* Testing to ensure we do not starve DA dogs *******\n");
    test_DA_starve(numBays, numDogs);
    
    printf("\n\n******* Testing to ensure we do not starve DB dogs *******\n");
    test_DB_starve(numBays, numDogs);

    return 0;
}
