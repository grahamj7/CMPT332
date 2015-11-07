#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "dogwashsynch_pthread.h"
#include <pthread.h>


pthread_mutex_t DA_mutex, DB_mutex, bays_avail_mutex, bays_mutex, wait_mutex;
int *DA_count, *DB_count, *bays_avail, *num_bays;



void DA_start(){
	pthread_mutex_lock(&wait_mutex);
    pthread_mutex_lock(&DA_mutex);
    *DA_count = *DA_count + 1;
    if (*DA_count == 1)
        pthread_mutex_lock(&bays_mutex);
    pthread_mutex_unlock(&wait_mutex);
    pthread_mutex_unlock(&DA_mutex);
}

void DB_start(){
    pthread_mutex_lock(&wait_mutex);
    pthread_mutex_lock(&DB_mutex);
    *DB_count = *DB_count + 1;
    if (*DB_count == 1)
        pthread_mutex_lock(&bays_mutex);
    pthread_mutex_unlock(&wait_mutex);
    pthread_mutex_unlock(&DB_mutex);
}

void DA_done(){
    pthread_mutex_lock(&DA_mutex);
    *DA_count = *DA_count - 1;
    if (*DA_count == 0)
        pthread_mutex_unlock(&bays_mutex);
    pthread_mutex_unlock(&DA_mutex);
}

void DB_done(){
    pthread_mutex_lock(&DB_mutex);
    *DB_count = *DB_count - 1;
    if (*DB_count == 0)
        pthread_mutex_unlock(&bays_mutex);
    pthread_mutex_unlock(&DB_mutex);
}



int dogwash_init(int numbays) {
    int rv;

    printf("Pthread dogwash init\n");

    srand(time(NULL));
        
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
    
    num_bays = (int *)malloc(sizeof(int));
    if (0 == num_bays){
        fprintf(stderr, "ERROR: Out of memory. Could not allocate num_bays.\n");
        return -1;
    }
    *num_bays = numbays;
    
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

    return 0;
}


int newdog(dogtype dog){
    printf("Dog, %s Arrived\n", DA == dog ? "A" : DB == dog ? "B" : "O");

    if (DA == dog)
        DA_start();
    else if (DB == dog)
        DB_start();

    pthread_mutex_lock(&bays_avail_mutex);
    while (0 == *bays_avail){
        pthread_mutex_unlock(&bays_avail_mutex);
        pthread_mutex_lock(&bays_avail_mutex);
    }
    if (0 > *bays_avail)
        fprintf(stderr, "Negative bays avail");
    *bays_avail = *bays_avail - 1;
    printf("\nDog, %s washing.\nBays Remaining: %d\n\n", DA == dog ? "A" : DB == dog ? "B" : "O", *bays_avail);
    pthread_mutex_unlock(&bays_avail_mutex);
        

    return 0;
}



int dogdone(dogtype dog) {
    pthread_mutex_lock(&bays_avail_mutex);
    *bays_avail = *bays_avail + 1;
    printf("\nFinished dog %s.\nBays avail: %d\n\n", DA == dog ? "A" : DB == dog ? "B" : "O", *bays_avail);
    pthread_mutex_unlock(&bays_avail_mutex);    

    if ( DA == dog ){
      DA_done();
    }else if ( DB == dog ){
      DB_done();
    }

    pthread_exit(NULL);
    return 0;
}

int dogwash_done() {
    int rv;
    printf("Pthread dogwash done\n");

    free(DA_count);
    DA_count = NULL;
    
    free(DB_count);
    DB_count = NULL;
    
    free(bays_avail);
    bays_avail = NULL;

    free(num_bays);
    num_bays = NULL;

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
    
    return 0;
}



