#include "tweet.h"

/* hash table to hold tweets */
struct tw_table tweet_table;

/* flag and lock to check if tweet_table has been initialized
 * -- plan to move initialization to init function if time left */
int init_flag = 0;
struct spinlock init_lock;

/* turns a tag into a hash value for placement of the tweet into the table */
int hash(char *tag){
    int j, hsh;
    hsh = 0;

    for (j = 0; j < 10; j++){
        hsh = (7 * hsh) + tag[j];
    }

    hsh = hsh % (maxtweettotal);
    return hsh;
}

/* initializes tweet table; should be moved elsewhere (like init()) if time */
void init_tw_table(void){
    int i, j;
    char *lock_name = "taglock";
    
    for(i = 0; i < maxtweettotal; i++){
        tweet_table.tag_list[i].tag[0] = '\0';
        for(j = 0; j < maxtweetsametag; j++){
            tweet_table.tag_list[i].tweets[j][0] = '\0';
        }
        tweet_table.tag_list[i].numtweets = 0;
        initlock(&tweet_table.tag_list[i].taglock, lock_name);
    }

    init_flag = 1;
}

/* Copies tag string */
void cp_tag(char *source, char *dest){
    int i;
    for(i = 0; i < taglength; i++){
        dest[i] = source[i];
    }
    dest[taglength] = '\0';
}


/* Copies message string */
void cp_tweet(char *source, char *dest){
    int i;
    for(i = 0; i < tweetlength; i++){
        dest[i] = source[i];
    }
    dest[tweetlength] = '\0';
}

// "tweet" a 140 character message with a 10 character tag
// Will block until able to tweet
int t_bput(char *tag, char *message){
    /* Check that hash table is initialized */
    acquire(&init_lock);
    if(0 == init_flag){
        init_tw_table();
    }
    release(&init_lock);
    
    /* Find location and aquire lock for that hash location */
    int hash_val = hash(tag);
    acquire(&(tweet_table.tag_list[hash_val].taglock));
    int n = tweet_table.tag_list[hash_val].numtweets;
    
    /* Check if tag is already initialized */
    if(0 == n){
        cp_tag(tag, tweet_table.tag_list[hash_val].tag);
    }
    
    /* Insert tweet when possible */
    while(tweet_table.tag_list[hash_val].numtweets >= maxtweetsametag){
        sleep(&(tweet_table.tag_list[hash_val]), 
                &(tweet_table.tag_list[hash_val].taglock));
    }
    cp_tweet(message, tweet_table.tag_list[hash_val].tweets[n]);
    tweet_table.tag_list[hash_val].numtweets = n + 1;
    
    if(n == 0){
        wakeup(&(tweet_table.tag_list[hash_val]));
    }
    release(&(tweet_table.tag_list[hash_val].taglock));
    
    return 0;
}

// like bput but returns immediately if unable to tweet
int t_put(char *tag, char *message){
    /* Check that hash table is initialized */
    acquire(&init_lock);
    if(0 == init_flag){
        init_tw_table();
    }
    release(&init_lock);
    
    /* Find location and aquire lock for that hash location */
    int hash_val = hash(tag);
    acquire(&(tweet_table.tag_list[hash_val].taglock));
    int n = tweet_table.tag_list[hash_val].numtweets;
    
    /* Check if tag is already initialized */
    if(0 == n){
        cp_tag(tag, tweet_table.tag_list[hash_val].tag);
    }
    
    /* Insert tweet if possible */
    if(n < maxtweetsametag){
        cp_tweet(message, tweet_table.tag_list[hash_val].tweets[n]);
        tweet_table.tag_list[hash_val].numtweets = n + 1;
    } else {
        release(&(tweet_table.tag_list[hash_val].taglock));
        return -1;
    }
    
    if(0 == n){
        wakeup(&(tweet_table.tag_list[hash_val]));
    }
    release(&(tweet_table.tag_list[hash_val].taglock));
    
    return 0;
}

// reads a tweet with matching tag and removes it from system copying it into buf.
// Block until able to do so
int t_bget(char *tag, char *buf){
    int hsh, n;

    hsh = hash(tag);
    acquire(&tweet_table.tag_list[hsh].taglock);
    n = tweet_table.tag_list[hsh].numtweets;
    
    while(tweet_table.tag_list[hsh].numtweets == 0){
        sleep(&(tweet_table.tag_list[hsh]),
            &(tweet_table.tag_list[hsh].taglock));
    }     
    cp_tweet(tweet_table.tag_list[hsh].tweets[n - 1], buf);
    tweet_table.tag_list[hsh].numtweets = n - 1;
    
    if(n == maxtweetsametag){
        wakeup(&(tweet_table.tag_list[hsh]));
    }
    release(&tweet_table.tag_list[hsh].taglock);
    return 0;
}

// like bget but returns immediately if cannot find tweet with matching tag
int t_get(char *tag, char *buf){
    int hsh, n;

    hsh = hash(tag);
    acquire(&tweet_table.tag_list[hsh].taglock);
    n = tweet_table.tag_list[hsh].numtweets;
    if(0 == n){
        /* Case of no msg with this tag; nothing to put in buffer */
        return -1;
    } else {
        cp_tweet(tweet_table.tag_list[hsh].tweets[n - 1], buf);
        tweet_table.tag_list[hsh].numtweets = n - 1;
        if(n == maxtweetsametag){
            wakeup(&(tweet_table.tag_list[hsh]));
        }
    }
    release(&tweet_table.tag_list[hsh].taglock);
    return 0;
}

void t_print_table(void){
    int i, j, n;
    
    for(i = 0; i < maxtweettotal; i++){
        j = 0;
        n = tweet_table.tag_list[i].numtweets;
        cprintf("Row %d has %d tweets.\n", i, n);
        while(j < n){
            cprintf("Entry number %d of row %d is: tag: %s, message: %s\n", j, i, 
                    tweet_table.tag_list[i].tag,
                    tweet_table.tag_list[i].tweets[j]);
            j++;
        }
    }
}

