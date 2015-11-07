#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "dogwashsynch_pthread.h"

pthread_t *dog_threads;
dogtype doggy;
int numBays, numDogs;

int init() {
    
    dog_threads = (pthread_t*) malloc(numDogs * sizeof(pthread_t));
    if (dog_threads == 0){
        fprintf(stderr, "Test failed to allocate dog_threads\n");
        return -1;
    }

    if(0 != dogwash_init(numBays)) {
        fprintf(stderr, "Error in initializing dogwash with %i bays.\n", numBays);
        return -1;
    }

    return 0;
}

int destroy(){

    free(dog_threads);
    dog_threads = NULL;
    
    if (0 != dogwash_done()){
        fprintf(stderr, "Dogwash_done failed to destroy the environment\n");
        return -1;
    }
    
    return 0;
}

void *dog_func(void *arg){
    newdog((dogtype) arg);
    sleep(1);
    dogdone((dogtype) arg);
    return NULL;
}


int test_DA_starve(){
    printf("\tWelcome to our dog wash! \nWe currently have %d bays available today\n\n", numBays);

    int i, rc;

    if (0 != init()) {
        fprintf(stderr, "Initializing test case rand_order failed\n");
        return -1;
    }
    
    for (i = 0; i < numDogs; i++) {
        doggy = DB;
        if (((i+1) % 4) == 0){doggy = DA;}
        rc = pthread_create(&dog_threads[i], NULL, dog_func, (void *)doggy);
        if (rc){
            fprintf(stderr, "ERROR; return code from pthread_create() is %d\n", rc);
            return -1;
        }
    }

    for (i = 0; i < numDogs; i++){
        rc = pthread_join(dog_threads[i], NULL);
        if (rc){
            fprintf(stderr, "ERROR; return code from pthread_join() is %d\n", rc);
            return -1;
        }
    }

    rc = destroy();
    printf("Done Washing Dogs\n");
}

int test_DB_starve(){
    printf("\tWelcome to our dog wash! \nWe currently have %d bays available today\n\n", numBays);

    int i, rc;

    if (0 != init()) {
        fprintf(stderr, "Initializing test case rand_order failed\n");
        return -1;
    }
    
    for (i = 0; i < numDogs; i++) {
        doggy = DA;
        if (((i+1) % 4) == 0){doggy = DB;}
        rc = pthread_create(&dog_threads[i], NULL, dog_func, (void *)doggy);
        if (rc){
            fprintf(stderr, "ERROR; return code from pthread_create() is %d\n", rc);
            return -1;
        }
    }

    for (i = 0; i < numDogs; i++){
        rc = pthread_join(dog_threads[i], NULL);
        if (rc){
            fprintf(stderr, "ERROR; return code from pthread_join() is %d\n", rc);
            return -1;
        }
    }

    rc = destroy();
    printf("Done Washing Dogs\n");
}

int test_rand_order(){

    printf("\tWelcome to our dog wash! \nWe currently have %d bays available today\n\n", numBays);

    int i, r, rc;

    if (0 != init(numDogs)) {
        fprintf(stderr, "Initializing test case rand_order failed\n");
        return -1;
    }

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

    for (i = 0; i < numDogs; i++){
        rc = pthread_join(dog_threads[i], NULL);
        if (rc){
            fprintf(stderr, "ERROR; return code from pthread_join() is %d\n", rc);
            return -1;
        }
    }

    rc = destroy();
    printf("Done Washing Dogs\n");

    return rc;
}

int main(int argc, char **argv){

    if (2 <= argc)                  /* If 1 or more arguments are passed in then, */
        numBays = atoi(argv[1]);    /* use the first argument as the number of bays*/
    if (0 == numBays)
        numBays = 5;                /* If numBays is 0 default it to 5 */
        
    if (3 <= argc)                  /* If 2 or more arguments are passed in, then use */
        numDogs = atoi(argv[2]);    /* the second arg for the number of dogs */
    if (5 > numDogs)
        numDogs = 20;               /* If numDogs is less than 5, default it to 20 */

    printf("\n\nTest random dogs\n");
    test_rand_order();
    printf("\n\nTest starve DA dogs\n");
    test_DA_starve();
    printf("\n\nTest starve DB dogs\n");
    test_DB_starve();

    /* Last thing that main() should do */
    return 0;
}
