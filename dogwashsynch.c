#include <jmorecfg.h>
#include "stdio.h"
#include "unistd.h"
#include "stdlib.h"
#include "dogwashsynch.h"
#include "pthread.h"


pthread_mutex_t DA_mutex, DB_mutex, bays_mutex, new_mutex;
int DA_count=0, DB_count=0, bays_occupied=0;

void *thread_enter(void * arg){

}

void increment_bays(){
    pthread_mutex_lock(&bays_mutex);
    bays_occupied++;
    printf("\tIncrement Bays: %d\n", bays_occupied);
    pthread_mutex_unlock(&bays_mutex);
}
void decrement_bays(){
    pthread_mutex_lock(&bays_mutex);
    bays_occupied--;
    printf("\tDecrement Bays: %d\n", bays_occupied);
    pthread_mutex_unlock(&bays_mutex);
}
int get_bays_occupied(){
    int val;
    pthread_mutex_lock(&bays_mutex);
    val = bays_occupied;
    pthread_mutex_unlock(&bays_mutex);
    return val;
}
boolean check_bays(){
    boolean bool;
    pthread_mutex_lock(&bays_mutex);
//    printf("Bays Occupied: %d\n", bays_occupied);
    bool = (bays_occupied >= NUMBAYS);
    pthread_mutex_unlock(&bays_mutex);
    return bool;
}

void increment_DA(){
    pthread_mutex_lock(&DA_mutex);
    printf("\tIncrement DA\n");
    DA_count++;
    pthread_mutex_unlock(&DA_mutex);
}
void decrement_DA(){
    pthread_mutex_lock(&DA_mutex);
    printf("\tDecrement DA\n");
    DA_count--;
    pthread_mutex_unlock(&DA_mutex);
}
int get_DA_count(){
    int val;
    pthread_mutex_lock(&DA_mutex);
    val = DA_count;
    pthread_mutex_unlock(&DA_mutex);
    return val;
}
boolean check_DA(){
    boolean bool;
    pthread_mutex_lock(&DB_mutex);
    bool = (0==DA_count);
    pthread_mutex_unlock(&DB_mutex);
    return bool;
}

void increment_DB(){
    pthread_mutex_lock(&DB_mutex);
    printf("\tIncrement DB\n");
    DB_count++;
    pthread_mutex_unlock(&DB_mutex);
}
void decrement_DB(){
    pthread_mutex_lock(&DB_mutex);
    printf("\tDecrement DB\n");
    DB_count--;
    pthread_mutex_unlock(&DB_mutex);
}
int get_DB_count(){
    int val;
    pthread_mutex_lock(&DB_mutex);
    val = DB_count;
    pthread_mutex_unlock(&DB_mutex);
    return val;
}
boolean check_DB(){
    boolean bool;
    pthread_mutex_lock(&DB_mutex);
    bool = (0==DB_count);
    pthread_mutex_unlock(&DB_mutex);
    return bool;
}

int dogwash_init(int numbays) {
    return 0;
}

void *newdog(void *arg){
    pthread_mutex_lock(&new_mutex);
    dogtype dog = (dogtype) arg;
    printf("Dog, %s Arrived\n", DA == dog ? "A" : DB == dog ? "B" : "O");
//    if (DA == dog) {
//        while (!check_DB()) {printf("While DB: %d\n", get_DB_count());}
//        increment_DA();
//    }
//    if (DB == dog) {
//        while (!check_DA()) {printf("While DA: %d\n", get_DA_count());}
//        increment_DB();
//    }

    while (check_bays()) ;
    increment_bays();
    printf("Bays Occupied: %d\n", get_bays_occupied());

    printf("Dog, %s_%d Washing in bay %d\n", DA == dog ? "A" : DB == dog ? "B" : "O", DA == dog ? get_DA_count() : DB == dog ? get_DB_count() : 0, get_bays_occupied());

    pthread_mutex_unlock(&new_mutex);
    pthread_exit(NULL);
}

int dogdone(dogtype dog) {
//    sleep(1);
    printf("Dog: %s Done\n", DA == dog ? "A" : DB == dog ? "B" : "O");
    if ( DA == dog ) decrement_DA();
    else if ( DB == dog ) decrement_DB();

    printf("DA: %d, DB: %d\n", get_DA_count(), get_DB_count());
    return dogwash_done();
}

int dogwash_done() {
    decrement_bays();
    return 0;
}

int main(int argc, char **argv) {

    pthread_t dogs[NUMBAYS];
    int rc;

    printf("Hello, World!\n");
    printf("Numbays: %d\n", NUMBAYS);

    int size = 12;
    dogtype *array = malloc(size*sizeof(dogtype));
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
    pthread_mutex_init(&new_mutex, NULL);
    pthread_mutex_init(&DA_mutex, NULL);
    pthread_mutex_init(&DB_mutex, NULL);

    for (int i = 0; i < size; ++i) {
        rc = pthread_create(&dogs[i], NULL, newdog, (void *)array[i]);
        if (rc){
            printf("ERROR; return code from pthread_create() is %d\n", rc);
            return -1;
        }
    }
    for (int i = 0; i < size; i++){
        rc = pthread_join(dogs[i], NULL);
        if (rc){
            printf("ERROR; return code from pthread_join() is %d\n", rc);
            return -1;
        }
        dogdone(array[i]);
    }

    pthread_mutex_destroy(&bays_mutex);
    pthread_mutex_destroy(&new_mutex);
    pthread_mutex_destroy(&DA_mutex);
    pthread_mutex_destroy(&DB_mutex);

    printf("Done creating threads\n");
    /* Last thing that main() should do */
    pthread_exit(NULL);
}