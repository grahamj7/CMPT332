#include <jmorecfg.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include "dogwashsynch.h"
#include <pthread.h>


pthread_mutex_t DA_mutex, DB_mutex, bays_avail_mutex, bays_mutex;
int *DA_count, *DB_count, *bays_avail, *num_bays;

pthread_cond_t Cond_A, Cond_B;

void DA_start(){
    printf("A: GET DB\n");
    pthread_mutex_lock(&DB_mutex);
    printf("A: GET DA\n");
    pthread_mutex_lock(&DA_mutex);
    printf("DA_start, DB_count: %d\n", *DB_count);
    if ( 0 != *DB_count){
        printf("Waiting for DA\n");
        pthread_mutex_unlock(&DA_mutex);
        pthread_cond_wait(&Cond_B, &DB_mutex);
        pthread_mutex_lock(&DA_mutex);
    }
    *DA_count = *DA_count + 1;
    pthread_mutex_unlock(&DB_mutex);
    printf("A: REL DB\n");
        printf("Wash DA\n");
    if (*DA_count == 1)
        pthread_mutex_lock(&bays_mutex);
    pthread_mutex_unlock(&DA_mutex);
    printf("A: REL DA\n");}

void DB_start(){
    printf("B: GET DB\n");
    pthread_mutex_lock(&DB_mutex);
    printf("B: GET DA\n");
    pthread_mutex_lock(&DA_mutex);
    printf("DB_start, DA_count: %d\n", *DA_count);
    if ( 0 != *DA_count){
        printf("Waiting for DA\n");
        pthread_mutex_unlock(&DB_mutex);
        pthread_cond_wait(&Cond_A, &DA_mutex);
        pthread_mutex_lock(&DB_mutex);
    }
    *DB_count = *DB_count + 1;
    pthread_mutex_unlock(&DA_mutex);
    printf("B: REL DA\n");
    printf("Wash DB\n");
    if (*DB_count == 1)
        pthread_mutex_lock(&bays_mutex);
    pthread_mutex_unlock(&DB_mutex);
    printf("B: REL DB\n");
}

void DA_done(){
    printf("DA_DONE\n");
    pthread_mutex_lock(&DA_mutex);
    *DA_count = *DA_count - 1;
    printf("Done DA: %d\n", *DA_count);
    if (*DA_count == 0){
        pthread_cond_signal(&Cond_A);
        printf("DA signal\n");
        pthread_mutex_unlock(&bays_mutex);
    }
    pthread_mutex_unlock(&DA_mutex);
    printf("Finished DA\n");
}

void DB_done(){
    printf("DB_DONE\n");
    pthread_mutex_lock(&DB_mutex);
    *DB_count = *DB_count - 1;
    printf("Done DB: %d\n", *DB_count);
    
    if (*DB_count == 0){
        pthread_cond_signal(&Cond_B);
        printf("DB signal\n");
        pthread_mutex_unlock(&bays_mutex);
    }
    pthread_mutex_unlock(&DB_mutex);
    printf("Finished DB\n");
}



int dogwash_init(int numbays) {
    int rv;

    srand(time(NULL));
        
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
    
    
    bays_avail = (int *)malloc(sizeof(int));
    if (0 == bays_avail){
        printf("ERROR: Out of memory. Could not allocate bays_avail.\n");
        return -1;
    }
    *bays_avail = numbays;
    
    num_bays = (int *)malloc(sizeof(int));
    if (0 == num_bays){
        printf("ERROR: Out of memory. Could not allocate num_bays.\n");
        return -1;
    }
    *num_bays = numbays;
    
    rv = pthread_mutex_init(&bays_mutex, NULL);
    if (0 != rv){
        printf("ERROR: Out of memory. Could not allocate bays_mutex.\n");
        return -1;
    }
    rv = pthread_mutex_init(&bays_avail_mutex, NULL);
    if (0 != rv){
        printf("ERROR: Out of memory. Could not allocate bays_avail_mutex.\n");
        return -1;
    }
    rv = pthread_mutex_init(&DA_mutex, NULL);
    if (0 != rv){
        printf("ERROR: Out of memory. Could not allocate DA_mutex.\n");
        return -1;
    }
    rv = pthread_mutex_init(&DB_mutex, NULL);
    if (0 != rv){
        printf("ERROR: Out of memory. Could not allocate DB_mutex.\n");
        return -1;
    }

    rv = pthread_cond_init(&Cond_A, NULL);
    if (0 != rv){
        printf("ERROR: Out of memory. Could not allocate Cond_A.\n");
        return -1;
    }

    rv = pthread_cond_init(&Cond_B, NULL);
    if (0 != rv){
        printf("ERROR: Out of memory. Could not allocate Cond_B.\n");
        return -1;
    }
    pthread_cond_signal(&Cond_A);
    pthread_cond_signal(&Cond_B);
    return 0;
}

void *dog(void *arg){
    int num = newdog((dogtype) arg);
    sleep(1);
    num = dogdone((dogtype) arg);
    return NULL;
}

int newdog(dogtype dog){
    printf("Dog, %s Arrived\n", DA == dog ? "A" : DB == dog ? "B" : "O");

    if (DA == dog)
        DA_start();
    else if (DB == dog)
        DB_start();
    
    printf("Find bay for dog %s\n", DA == dog ? "A" : DB == dog ? "B" : "O");
    pthread_mutex_lock(&bays_avail_mutex);
    while (0 == *bays_avail){
        pthread_mutex_unlock(&bays_avail_mutex);
        pthread_mutex_lock(&bays_avail_mutex);
    }
    if (0 > *bays_avail)
        printf("Negative bays avail");
    *bays_avail = *bays_avail - 1;
    pthread_mutex_unlock(&bays_avail_mutex);
        
    printf("Dog, %s_%d washing. Bay Remaining: %d\n\n", DA == dog ? "A" : DB == dog ? "B" : "O", DA == dog ? *DA_count : DB == dog ? *DB_count : 0, *bays_avail);

    return 0;
}

int dogdone(dogtype dog) {
    printf("Dog: %s Done\n\n", DA == dog ? "A" : DB == dog ? "B" : "O");

    pthread_mutex_lock(&bays_avail_mutex);
    *bays_avail = *bays_avail + 1;
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
        printf("Failed to destroy bays_mutex.\n");
        return -1;
    }
    rv = pthread_mutex_destroy(&bays_avail_mutex);
    if (0 != rv){
        printf("Failed to destroy bays_avail_mutex.\n");
        return -1;
    }
    rv = pthread_mutex_destroy(&DA_mutex);
    if (0 != rv){
        printf("Failed to destroy DA_mutex.\n");
        return -1;
    }
    rv = pthread_mutex_destroy(&DB_mutex);
    if (0 != rv){
        printf("Failed to destroy DB_mutex.\n");
        return -1;
    }
    
    rv = pthread_cond_destroy(&Cond_A);
    if (0 != rv){
        printf("Failed to destroy Cond_A.\n");
        return -1;
    }
    rv = pthread_cond_destroy(&Cond_B);
    if (0 != rv){
        printf("Failed to destroy Cond_B.\n");
        return -1;
    }

    return 0;
}





int main(int argc, char **argv) {

    
    int rc, i, numBays;
    int numDogs;
    pthread_t dogs[12];
    int r;
    dogtype doggy;

    numDogs = 12;
    numBays = 5;

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
    
    
    
    
/*











*/
