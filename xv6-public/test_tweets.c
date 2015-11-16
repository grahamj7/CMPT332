#include "types.h"
#include "param.h"
#include "user.h"

int main(int argc, char *argv[]) {
    int i, m, rc, rc2, rc3;
    int tweetlength = 140;

    char buffer[tweetlength + 1];
    buffer[0] = '\0';

    char *tag_arr[maxtweettotal] = {"tag 3     ","tag 1     ","tag 4     ","tag 2     ",
        "tag 1     ", "tag 5     ", "tag 5     ", "tag 1     ", "tag 2     ",
        "tag 4     ", "tag 3     ", "tag 5     ", "tag 2     ", "tag 4     ",
        "tag 3     "};
    char *msg_arr[maxtweettotal] = {"cat 1", "banana 1", "dog 1", "apple 1",
        "banana 2", "starfish 1", "starfish 2", "banana 3", "apple 2", "dog 2",
        "cat 2", "starfish 3", "apple 3", "dog 3", "cat 3"};

    printf(1, "\n========== Putting initial data in tweet structure ==========\n");
    for(m = 0; m < maxtweettotal; m++){
        i = bput(tag_arr[m], msg_arr[m]);
        if(0 != i){
            printf(1, "Error in with bput.\n");
            exit();
        }
    }



    printf(1, "\n========== Printing the current tweet table  ==========\n");
    printtweettable();



    printf(1, "\n========== Forking to create multiple test threads  ==========\n");
    rc = fork(); /* break from great grandparent thread */
    if(0 > rc){
        printf(1, "Failed to fork\n");
        exit(); 
    }
    else if(0 == rc){ /* grandparent thread */
        rc2 = fork();
        if(0 > rc2){
            printf(1, "Failed to fork\n");
            exit();
        }
        else if(0 == rc2){ /* parent thread */
            rc3 = fork();
            if(0 > rc3){
                printf(1, "Failed to fork\n");
                exit();
            }
            else if(0 == rc3){ /* child thread */
                printf(1, "Child thread!\n");
            }
            else { /* parent thread */
                char *test_tag_par = "tag 1     ";
                i = bget(test_tag_par, buffer);
                if(0 != i){
                printf(1, "Error in with get.\n");
                    exit();
                }
                printf(1, "\nThe tweet returned from tag %s was %s.\n", test_tag_par, buffer);


                i = bget(test_tag_par, buffer);
                 if(0 != i){
                    printf(1, "Error in with get.\n");
                    exit();
                }
                printf(1, "\nThe tweet returned from tag %s was %s.\n", test_tag_par, buffer);


                i = bget(test_tag_par, buffer);
                if(0 != i){
                    printf(1, "Error in with get.\n");
                    exit();
                }
                printf(1, "\nThe tweet returned from tag %s was %s.\n", test_tag_par, buffer);

                test_tag_par = "tag 4     ";
                char *test_msg_par = "Pineapples 704";
                i = bput(test_tag_par, test_msg_par);
                if(0 != i){
                    printf(1, "Error in with put.\n");
                    exit();
                }
                wait();
            }
        }
        else { /* grandparent thread */
            char *test_tag = "tag 4     ";
            i = bget(test_tag, buffer);
            if(0 != i){
            printf(1, "Error in with bget.\n");
                exit();
            }
            printf(1, "\nThe tweet returned from tag %s was %s.\n", test_tag, buffer);


            i = bget(test_tag, buffer);
            if(0 != i){
                printf(1, "Error in with bget.\n");
                exit();
            }
            printf(1, "\nThe tweet returned from tag %s was %s.\n", test_tag, buffer);


            i = bget(test_tag, buffer);
            if(0 != i){
                printf(1, "Error in with bget.\n");
                exit();
            }
            printf(1, "\nThe tweet returned from tag %s was %s.\n", test_tag, buffer);

        
            i = bget(test_tag, buffer);
            if(0 != i){
                printf(1, "Error in with bget.\n");
                exit();
            }
            printf(1, "\nThe tweet returned from tag %s was %s.\n", test_tag, buffer);
            wait();
        }
    }
    else { /* great grandparent thread */
        wait();
        printtweettable();
    }

    exit();
}
