#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include "dogwashsynch_sem.h"
#include <semaphore.h>
#include <pthread.h>


sem_t DA_sem, DB_sem, Bays_sem, Bays_avail_sem;
int *num_bays, *DA_count, *DB_count;



void DA_start(){
    printf("DA start: %d\n", *DA_count);
    sem_wait(&DA_sem);
    printf("got DA_sem\n");
    *DA_count = *DA_count + 1;
    if (*DA_count == 1)
        sem_wait(&Bays_sem);
    printf("Rel DA_sem\n");
    sem_post(&DA_sem);
}

void DB_start(){
    printf("DB start: %d\n", *DB_count);
    sem_wait(&DB_sem);
    printf("Got DB_sem\n");
    *DB_count = *DB_count + 1;
    if (*DB_count == 1)
        sem_wait(&Bays_sem);
    printf("Rel DB_sem\n");
    sem_post(&DB_sem);
}

void DA_done(){
    printf("DA end: %d\n", *DB_count);
    sem_wait(&DA_sem);
    *DA_count = *DA_count - 1;
    if (*DA_count == 0)
        sem_post(&Bays_sem);
    sem_post(&DA_sem);
}

void DB_done(){
    printf("DB end: %d\n", *DB_count);
    sem_wait(&DB_sem);
    *DB_count = *DB_count - 1;
    if (*DB_count == 0)
        sem_post(&Bays_sem);
    sem_post(&DB_sem);
}



int dogwash_init(int numbays) {
    int rv;

    srand((unsigned int) time(NULL));


    DA_count = (int *)malloc(sizeof(int));
    if (0 == DA_count){
        printf("ERROR: Out of memory. Could not allocate DA_count.\n");
        return -1;
    }
    *DA_count = 0;


    DB_count = (int *)malloc(sizeof(int));
    if (0 == DB_count){
        printf("ERROR: Out of memory. Could not allocate DB_count.\n");
        return -1;
    }
    *DB_count = 0;

    num_bays = (int *)malloc(sizeof(int));
    if (0 == num_bays){
        printf("ERROR: Out of memory. Could not allocate num_bays.\n");
        return -1;
    }
    *num_bays = numbays;


    rv = sem_init(&Bays_sem, 0, 1);
    if (0 != rv){
        printf("ERROR: Out of memory. Could not allocate Bays_sem.\n");
        return -1;
    }
    rv = sem_init(&Bays_avail_sem, 0, (unsigned int) numbays);
    if (0 != rv){
        printf("ERROR: Out of memory. Could not allocate Bays_avail_sem.\n");
        return -1;
    }
    rv = sem_init(&DA_sem, 0, 1);
    if (0 != rv){
        printf("ERROR: Out of memory. Could not allocate DA_sem.\n");
        return -1;
    }
    rv = sem_init(&DB_sem, 0, 1);
    if (0 != rv){
        printf("ERROR: Out of memory. Could not allocate DB_sem.\n");
        return -1;
    }


    return 0;
}

int dogwash_done() {
    int rv;

    free(DA_count);
    DA_count = NULL;

    free(DB_count);
    DB_count = NULL;

    free(num_bays);
    num_bays = NULL;

    rv = sem_destroy(&Bays_sem);
    if (0 != rv){
        printf("Failed to close Bays_sem.\n");
        return -1;
    }
    rv = sem_destroy(&Bays_avail_sem);
    if (0 != rv){
        printf("Failed to close Bays_avail_sem.\n");
        return -1;
    }
    rv = sem_destroy(&DA_sem);
    if (0 != rv){
        printf("Failed to close DA_sem.\n");
        return -1;
    }
    rv = sem_destroy(&DB_sem);
    if (0 != rv){
        printf("Failed to close DB_sem.\n");
        return -1;
    }


    return 0;
}



int newdog(dogtype dog){
    int value;
    printf("Dog, %s Arrived\n", DA == dog ? "A" : DB == dog ? "B" : "O");

    if (DA == dog) {
        DA_start();
    }
    else if (DB == dog) {
        DB_start();
    }

    sem_getvalue(&Bays_avail_sem, &value);
    printf("Wait for avail bay %d\n", value);

    sem_wait(&Bays_avail_sem);

    printf("Found avail bay\n");
    sem_getvalue(&Bays_avail_sem, &value);
    printf("Dog, %s_%d washing. Bay Remaining: %d\n\n", DA == dog ? "A" : DB == dog ? "B" : "O", 0, value);

    return 0;
}

int dogdone(dogtype dog) {
    int value;
    sem_getvalue(&Bays_avail_sem, &value);
    printf("Dog: %s Done\nBays avail: %d\n\n", DA == dog ? "A" : DB == dog ? "B" : "O", value);

    sem_post(&Bays_avail_sem);

    if ( DA == dog ){
        DA_done();
    }else if ( DB == dog ){
        DB_done();
    }

    sem_getvalue(&Bays_avail_sem, &value);
    printf("Finished dog %s\nBays avail: %d\n\n", DA == dog ? "A" : DB == dog ? "B" : "O", value);
    pthread_exit(NULL);
    return 0;
}



void *dog(void *arg){
    int num = newdog((dogtype) arg);
    sleep(1);
    num = dogdone((dogtype) arg);
    return NULL;
}



int main(int argc, char **argv) {

    int rc, i, r, numBays=0, numDogs=0;
    pthread_t dogs[12];
    dogtype doggy = DO;



    if (2 <= argc)
        numBays = atoi(argv[1]);
    else if (3 <= argc)
        numDogs = atoi(argv[2]);

    if (0 == numBays)
        numBays = 5;
    if (0 == numDogs)
        numDogs = 12;


    printf("\tWelcome to our dog wash! \nWe currently have %d bays available today\n\n", numBays);

    if(0 != dogwash_init(numBays)){
        printf("Error in initializing dogwash with %i bays.\n", numBays);
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

        rc = pthread_create(&dogs[i], NULL, dog, (void *)doggy);
        if (rc){
            printf("ERROR; return code from pthread_create() is %d\n", rc);
            return -1;
        }
    }
    for (i = 0; i < numDogs; i++){
        rc = pthread_join(dogs[i], NULL);
        if (rc){
            printf("ERROR; return code from pthread_join() is %d\n", rc);
            return -1;
        }
    }

    printf("Done Washing Dogs\n");


    /* Last thing that main() should do */
    return dogwash_done();
}


