#include "types.h"
#include "param.h"
#include "user.h"

int main(int argc, char *argv[]) {
    int i, k, l, m;
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

    for(m = 0; m < maxtweettotal; m++){
        i = bput(tag_arr[m], msg_arr[m]);
        if(0 != i){
            printf(1, "Error in with bput.\n");
            exit();
        }
    }

    /*j = put(tag_arr[m], msg_arr[m]);
    if(0 != j){
        printf(1, "Error in with put.\n");
        exit();
    }*/
    
    char *test_tag = "tag 4     ";
    k = bget(test_tag, buffer);
    if(0 != k){
        printf(1, "Error in with bget.\n");
        exit();
    }

    printf(1, "The tweet returned from tag %s was %s.\n", test_tag, buffer);

    l = get(test_tag, buffer);
    if(0 != l){
        printf(1, "Error in with get.\n");
        exit();
    }
    


    /* printf(1, "Inside testtweets.c: mytag = %s, message = %s, ret_tag = %s, buffer = %s\n", mytag, message, ret_tag, buffer);
    printf(1, "Returned to testtweets.c: bput: %d, put: %d, bget: %d, get: %d.\n", i, j, k, l);*/

    exit();
}
