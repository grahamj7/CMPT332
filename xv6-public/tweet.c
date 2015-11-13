#include "tweet.h"

/* hash table to hold tweets */
tw_table tweet_table;

/* flag and lock to check if tweet_table has been initialized
 * -- plan to move initialization to init function if time left */
int init_flag = 0;
struct spinlock init_lock;

/* turns a tag into a hash value for placement of the tweet into the table */
int hash(char *tag){
    int j, hsh;
    hsh = 0;

    for (j = 0; j < 10; j++){
        hsh = (37 * hsh) + tag[j];
    }

    hsh = hsh % (maxtweettotal/maxtweetsametag);
    return hsh;
}

/* initializes tweet table; should be moved elsewhere (like init()) if time */
void init_tw_table(void){
    int i;
    char *lock_name = "taglock";
    
    for(i = 0; i < (maxtweettotal/maxtweetsametag); i++){
        tweet_table.tw_entries[i].num_tags_filled = 0;
        initlock(&tweet_table.tw_entries[i].taglock, lock_name);
    }
}

// "tweet" a 140 character message with a 10 character tag
// Will block until able to tweet
int t_bput(char *tag, char *message){
    acquire(&init_lock);
        if(0 == init_flag){
            init_tw_table();
        }
    release(&init_lock);

    int hash_val = hash(tag);
    acquire(&(tweet_table.tw_entries[hash_val].taglock));
    int n = tweet_table.tw_entries[hash_val].num_tags_filled;
    if(n < maxtweetsametag){
        tweet_table.tw_entries[hash_val].tw_tag_list[n].t_tag = tag;
        tweet_table.tw_entries[hash_val].tw_tag_list[n].t_message = message;
        tweet_table.tw_entries[hash_val].num_tags_filled = n + 1;
    }
    release(&(tweet_table.tw_entries[hash_val].taglock));
    //cprintf("In tweet.c, t_bget() received: %s, The message: %s\n\n", tag, message);
    return 0;
}

// like bput but returns immediately if unable to tweet
int t_put(char *tag, char *message){
    //cprintf("In tweet.c, t_bget() received: %s, The message: %s\n\n", tag, message);
    return 0;
}

// reads a tweet with matching tag and removes it from system copying it into buf.
// Block until able to do so
int t_bget(char *tag, char *buf){
    int i, j;
    //buf = "This buf was returned from t_bget.\0";
    cprintf("In tweet.c, t_bget() received: %s, The buffer: %s\n\n", tag, buf);
    
    for(i = 0; i < (maxtweettotal/maxtweetsametag); i++){
        j = 0;
        while(j < tweet_table.tw_entries[i].num_tags_filled){
            cprintf("Entry number %d of row %d is: tag: %s, message: %s\n", j, i, 
                    tweet_table.tw_entries[i].tw_tag_list[j].t_tag,
                    tweet_table.tw_entries[i].tw_tag_list[j].t_message);
            j++;
        }
    }
    return 0;
}

// like bget but returns immediately if cannot find tweet with matching tag
int t_get(char *tag, char *buf){
    //buf = "This buf was returned from t_bget.\0";
    //cprintf("In tweet.c, t_get() received: %s, The buffer: %s\n\n", tag, buf);
    return 0;
}
