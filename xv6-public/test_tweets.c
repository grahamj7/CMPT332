#include "types.h"
#include "param.h"
#include "user.h"

int main(int argc, char *argv[]) {
    int i, m, rc;
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



    printf(1, "\n========== Printing the current tweet table (1)  ==========\n");
    printtweettable();



    printf(1, "\n========== Forking to create multiple test threads  ==========\n");
    rc = fork(); /* break from great grandparent thread */
    if(0 > rc){
        printf(1, "Failed to fork\n");
        exit(); 
    }
    else if(0 == rc){ /* grandparent thread */
        /*rc = fork();
        if(0 > rc){
            printf(1, "Failed to fork\n");
            exit();
        }
        else if(0 == rc){*/ /* parent thread */
            /*rc = fork();
            if(0 > rc){
                printf(1, "Failed to fork\n");
                exit();
            }
            else if(0 == rc){*/ /* child thread */
                // do stuff
                /*exit();
            }
            else { *//* parent thread */
                /*char *test_tag_par = "tag 1     ";
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
                i = bget(test_tag_par, buffer);
                if(-1 != i){
                    printf(1, "Error in with get.\n");
                    exit();
                }
                printf(1, "\nThe tweet returned from tag %s was %s.\n", test_tag_par, buffer);
                wait();
                exit();
            }
        }
        else {*/ /* grandparent thread */
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

        
            /*i = bget(test_tag, buffer);
            if(0 != i){
                printf(1, "Error in with bget.\n");
                exit();
            }
            printf(1, "\nThe tweet returned from tag %s was %s.\n", test_tag, buffer);
            wait();*/
            exit();    
        //}
    }
    else { /* great grandparent thread */
        wait();
        printtweettable();
    }

    /*printf(1, "\n========== Pull latest tweet with tag 4 (using bget) ==========\n");
    char *test_tag = "tag 4     ";
    i = bget(test_tag, buffer);
    if(0 != i){
        printf(1, "Error in with bget.\n");
        exit();
    }
    printf(1, "\nThe tweet returned from tag %s was %s.\n", test_tag, buffer);



    printf(1, "\n========== Pull latest tweet with tag 4 (using get) ==========\n");
    i = get(test_tag, buffer);
    if(0 != i){
        printf(1, "Error in with get.\n");
        exit();
    }
    printf(1, "\nThe tweet returned from tag %s was %s.\n", test_tag, buffer);


    
    printf(1, "\n========== Printing the current tweet table (2)  ==========\n");
    printtweettable();



    printf(1, "\n========== Attempt to exceed maxtweetssametag with put (should "
            "have no output)  ==========\n");
    char *new_tag = "tag 5     ";
    char *new_msg = "17 pineapples!";
    i = put(new_tag, new_msg);
    if(-1 != i){
        printf(1, "Error in with put. It should have failed with a -1 code.\n");
        exit();
    }



    printf(1, "\n========== Printing the current tweet table (3) ==========\n");
    printtweettable();



    printf(1, "\n========== Attempt to bput msg 42 pineapples when maxtweetsametag"
           " is exceeded. Must wait.  ==========\n");
    char *new_tag2 = "tag 5     ";
    char *new_msg2 = "42 pineapples!";
    i = bput(new_tag2, new_msg2);
    if(0 != i){
        printf(1, "Error in with bput.\n");
        exit();
    }


    printf(1, "\n========== Printing the current tweet table (4)  ==========\n");
    printtweettable();



    printf(1, "\n========== Using bget to pull latest tag 5 tweet ==========\n");
    char *test_tag2 = "tag 5     ";
    i = bget(test_tag2, buffer);
    if(0 != i){
        printf(1, "Error in with bget.\n");
        exit();
    }
    printf(1, "The tweet returned from tag %s was %s.\n", test_tag2, buffer);
    


    printf(1, "\n========== Printing the current tweet table  ==========\n");
    printtweettable(); */


    exit();
}
