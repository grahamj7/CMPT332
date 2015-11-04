#include <jmorecfg.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "dogwashsynch.h"
#include <pthread.h>


pthread_mutex_t DA_mutex, DB_mutex, bays_mutex, new_dog_mutex;
/* Should we have a mutex where if there is an A dog in a bay, A holds
the "can add" lock? So B can only be in there if the lock isnt held */
int *DA_count, *DB_count, *bays_occupied, *num_bays;

void *thread_enter(void * arg){
  return NULL;
}

void increment_bays(){
    pthread_mutex_lock(&bays_mutex);
    bays_occupied++;
    printf("\tIncrement Bays: %d\n", *bays_occupied);
    pthread_mutex_unlock(&bays_mutex);
}
void decrement_bays(){
    pthread_mutex_lock(&bays_mutex);
    bays_occupied--;
    printf("\tDecrement Bays: %d\n", *bays_occupied);
    pthread_mutex_unlock(&bays_mutex);
}
int get_bays_occupied(){
    int val;
    pthread_mutex_lock(&bays_mutex);
    val = *bays_occupied;
    pthread_mutex_unlock(&bays_mutex);
    return val;
}
boolean check_bays(){
    boolean bool;
    pthread_mutex_lock(&bays_mutex);
/*    printf("Bays Occupied: %d\n", bays_occupied);*/
    bool = (*bays_occupied >= *num_bays);
    pthread_mutex_unlock(&bays_mutex);
    return bool;
}

void increment_DA(){
    pthread_mutex_lock(&DA_mutex);
    printf("\tIncrement DA\n");
    *DA_count++;
    pthread_mutex_unlock(&DA_mutex);
}
void decrement_DA(){
    pthread_mutex_lock(&DA_mutex);
    printf("\tDecrement DA\n");
    *DA_count--;
    pthread_mutex_unlock(&DA_mutex);
}
int get_DA_count(){
    int val;
    pthread_mutex_lock(&DA_mutex);
    val = *DA_count;
    pthread_mutex_unlock(&DA_mutex);
    return val;
}
boolean check_DA(){
    boolean boolVar;
    pthread_mutex_lock(&DB_mutex);
    boolVar = (0==*DA_count);
    pthread_mutex_unlock(&DB_mutex);
    return boolVar;
}

void increment_DB(){
    pthread_mutex_lock(&DB_mutex);
    printf("\tIncrement DB\n");
    *DB_count++;
    pthread_mutex_unlock(&DB_mutex);
}
void decrement_DB(){
    pthread_mutex_lock(&DB_mutex);
    printf("\tDecrement DB\n");
    *DB_count--;
    pthread_mutex_unlock(&DB_mutex);
}
int get_DB_count(){
    int val;
    pthread_mutex_lock(&DB_mutex);
    val = *DB_count;
    pthread_mutex_unlock(&DB_mutex);
    return val;
}
boolean check_DB(){
    boolean boolVar;
    pthread_mutex_lock(&DB_mutex);
    boolVar = (0==DB_count);
    pthread_mutex_unlock(&DB_mutex);
    return boolVar;
}







int dogwash_init(int numbays) {
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
    
    
    bays_occupied = (int *)malloc(sizeof(int));
    if (0 == bays_occupied){
        printf("ERROR: Out of memory. Could not allocate bays_occupied.\n");
        return -1;
    }
    *bays_occupied = 0;
    
    num_bays = (int *)malloc(sizeof(int));
    if (0 == DB_count){
        printf("ERROR: Out of memory. Could not allocate num_bays.\n");
        return -1;
    }
    *num_bays = numbays;
    
    return 0;
}



void *newdog_init(void *arg){
    newdog((dogtype) *arg);



int newdog(dogtype dog){
    pthread_mutex_lock(&new_dog_mutex);
    printf("Dog, %s Arrived\n", DA == dog ? "A" : DB == dog ? "B" : "O");
    if (DA == dog) {
        while (!check_DB()) {printf("While DB: %d\n", get_DB_count());}
        increment_DA();
    }
    if (DB == dog) {
        while (!check_DA()) {printf("While DA: %d\n", get_DA_count());}
        increment_DB();
    }

    while (check_bays()) ;
    increment_bays();
    printf("Bays Occupied: %d\n", get_bays_occupied());

    printf("Dog, %s_%d Washing in bay %d\n", DA == dog ? "A" : DB == dog ? "B" : "O", DA == dog ? get_DA_count() : DB == dog ? get_DB_count() : 0, get_bays_occupied());

    pthread_mutex_unlock(&new_dog_mutex);
}






int dogdone(dogtype dog) {
    sleep(1);
    printf("Dog: %s Done\n", DA == dog ? "A" : DB == dog ? "B" : "O");
    if ( DA == dog ){
      decrement_DA();
    }else if ( DB == dog ){
      decrement_DB();
    }

    printf("DA: %d, DB: %d\n", get_DA_count(), get_DB_count());
    decrement_bays();
    pthread_exit(NULL);
    return 0;
}






int dogwash_done() {
    free(DA_count);
    DA_count = NULL;
    
    free(DB_count);
    DB_count = NULL;
    
    free(bays_occupied);
    bays_occupied = NULL;

    free(num_bays);
    num_bays = NULL;
    
    return 0;
}





int main(int argc, char **argv) {

    
    int rc, i, numBays;
    const int numDogs = 12;
    numBays = 5;
    pthread_t dogs[12];
    dogtype *array;

    printf("Hello, World!\n");
    printf("Numbays: %d\n", numBays);
    
    if(0 != dogwash_init(numBays)){
      printf("Error in initializing dogwash with %i bays.\n", numBays);
      return -1;
    }

    
    array = malloc(numDogs*sizeof(dogtype));
    array[0] = DB;
    array[1] = DB;
    array[2] = DB;
    array[3] = DB;
    array[4] = DB;
    array[5] = DB;
    array[6] = DB;

    array[7] = DA;
    array[8] = DA;
    array[9] = DA;
    array[10] = DA;
    array[11] = DA;

    pthread_mutex_init(&bays_mutex, NULL);
    pthread_mutex_init(&new_dog_mutex, NULL);
    pthread_mutex_init(&DA_mutex, NULL);
    pthread_mutex_init(&DB_mutex, NULL);

    for (i = 0; i < numDogs; i++) {
        rc = pthread_create(&dogs[i], NULL, newdog, (void *)array[i]);
        if (rc){
            printf("ERROR; return code from pthread_create() is %d\n", rc);
            return -1;
        }
        printf("completed pthread_create #%i\n\n", i);
    }
    for (i = 0; i < numDogs; i++){
        rc = pthread_join(dogs[i], NULL);
        printf("Did we get to pthread_join?\n");
        if (rc){
            printf("ERROR; return code from pthread_join() is %d\n", rc);
            return -1;
        }
        dogdone(array[i]);
    }

    pthread_mutex_destroy(&bays_mutex);
    pthread_mutex_destroy(&new_dog_mutex);
    pthread_mutex_destroy(&DA_mutex);
    pthread_mutex_destroy(&DB_mutex);

    printf("Done creating threads\n");
    /* Last thing that main() should do */
    pthread_exit(NULL);
}
