#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "dogwashsynch_sem.h"
#include <semaphore.h>
#include <pthread.h>


sem_t DA_sem, DB_sem, Bays_sem, Bays_avail_sem, Wait_sem;
int *num_bays, *DA_count, *DB_count;



void DA_start(){
    printf("Try Wait Sem\n");
    sem_wait(&Wait_sem);
    printf("Got Wait Sem\n");
    printf("Try DA_Sem\n");
    sem_wait(&DA_sem);
    printf("Got DA_Sem\n");
    *DA_count = *DA_count + 1;
    if (*DA_count == 1){
        printf("Try Bays_Sem\n");
        sem_wait(&Bays_sem);
        printf("Got Bays_Sem\n");
    }
    printf("Post Wait\n");
    sem_post(&Wait_sem);
    printf("Post DA\n");
    sem_post(&DA_sem);
}

void DB_start(){
    printf("Try Wait Sem\n");
    sem_wait(&Wait_sem);
    printf("Got Wait Sem\n");
    printf("Try DB_Sem\n");
    sem_wait(&DB_sem);
    printf("Got DB_Sem\n");
    *DB_count = *DB_count + 1;
    if (*DB_count == 1){
        printf("Try Bays_Sem\n");
        sem_wait(&Bays_sem);
        printf("Got Bays_Sem\n");
    }
    printf("Post Wait\n");
    sem_post(&Wait_sem);
    printf("Post DB\n");
    sem_post(&DB_sem);
}

void DA_done(){
    printf("D: Try DA_Sem\n");
    sem_wait(&DA_sem);
    printf("D: Got DA_Sem\n");
    *DA_count = *DA_count - 1;
    if (*DA_count == 0){
        printf("D: Post Bays_Sem\n");
        sem_post(&Bays_sem);
    }
    printf("D: Post DA_Sem\n");
    sem_post(&DA_sem);
}

void DB_done(){
    printf("D: Try DB_Sem\n");
    sem_wait(&DB_sem);
    printf("D: Got DB_Sem\n");
    *DB_count = *DB_count - 1;
    if (*DB_count == 0){
        printf("D: Post Bays_Sem\n");
        sem_post(&Bays_sem);
    }
    printf("D: Post DB_Sem\n");
    sem_post(&DB_sem);
}



int dogwash_init(int numbays) {
    int rv;

    printf("Sem dogwash init\n");

    srand((unsigned int) time(NULL));


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

    num_bays = (int *)malloc(sizeof(int));
    if (0 == num_bays){
        fprintf(stderr, "ERROR: Out of memory. Could not allocate num_bays.\n");
        return -1;
    }
    *num_bays = numbays;


    rv = sem_init(&Bays_sem, 0, 1);
    if (0 != rv){
        fprintf(stderr, "ERROR: Out of memory. Could not allocate Bays_sem.\n");
        return -1;
    }
    rv = sem_init(&Bays_avail_sem, 0, (unsigned int) numbays);
    if (0 != rv){
        fprintf(stderr, "ERROR: Out of memory. Could not allocate Bays_avail_sem.\n");
        return -1;
    }
    rv = sem_init(&DA_sem, 0, 1);
    if (0 != rv){
        fprintf(stderr, "ERROR: Out of memory. Could not allocate DA_sem.\n");
        return -1;
    }
    rv = sem_init(&DB_sem, 0, 1);
    if (0 != rv){
        fprintf(stderr, "ERROR: Out of memory. Could not allocate DB_sem.\n");
        return -1;
    }

    rv = sem_init(&Wait_sem, 0, 1);
    if (0 != rv){
        fprintf(stderr, "ERROR: Out of memory. Could not allocate Wait_sem.\n");
        return -1;
    }

    return 0;
}

int dogwash_done() {
    int rv;

    printf("Sem dogwash done\n");

    free(DA_count);
    DA_count = NULL;

    free(DB_count);
    DB_count = NULL;

    free(num_bays);
    num_bays = NULL;

    rv = sem_destroy(&Bays_sem);
    if (0 != rv){
        fprintf(stderr, "Failed to close Bays_sem.\n");
        return -1;
    }
    rv = sem_destroy(&Bays_avail_sem);
    if (0 != rv){
        fprintf(stderr, "Failed to close Bays_avail_sem.\n");
        return -1;
    }
    rv = sem_destroy(&DA_sem);
    if (0 != rv){
        fprintf(stderr, "Failed to close DA_sem.\n");
        return -1;
    }
    rv = sem_destroy(&DB_sem);
    if (0 != rv){
        fprintf(stderr, "Failed to close DB_sem.\n");
        return -1;
    }
    rv = sem_destroy(&Wait_sem);
    if (0 != rv){
        fprintf(stderr, "Failed to close Wait_sem.\n");
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

    sem_wait(&Bays_avail_sem);
    sem_getvalue(&Bays_avail_sem, &value);
    printf("\nDog, %s washing.\nBays Remaining: %d\n\n", DA == dog ? "A" : DB == dog ? "B" : "O", value);

    return 0;
}

int dogdone(dogtype dog) {
    int value;
    sem_getvalue(&Bays_avail_sem, &value);

    sem_post(&Bays_avail_sem);
    sem_getvalue(&Bays_avail_sem, &value);
    printf("\nFinished dog %s.\nBays avail: %d\n\n", DA == dog ? "A" : DB == dog ? "B" : "O", value);

    if ( DA == dog ){
        DA_done();
    }else if ( DB == dog ){
        DB_done();
    }

    pthread_exit(NULL);
    return 0;
}



