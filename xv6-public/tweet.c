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
        tweet_table.tag_list[i].waiter = 0;
    }
    
    tweet_table.currtotal = 0;
    lock_name = "total tweet lock";
    initlock(&tweet_table.totallock, lock_name);

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

/* Checks for matching tags */
int put_check_tag_matches_hash_table_tag(char *tag, int hash_val){
    int n, count=0;
    if (strncmp(tag, tweet_table.tag_list[hash_val].tag, taglength) != 0){
        release(&(tweet_table.tag_list[hash_val].taglock));
        
        /* find a new hash value for the tag if this one is taken */
        do {
            if (count > maxtweettotal){
                return -1;
            }
            hash_val = (hash_val + 1) % maxtweettotal;
            n = tweet_table.tag_list[hash_val].numtweets;
            count++;
        } while (n != 0 && strncmp(tag, tweet_table.tag_list[hash_val].tag, taglength) != 0); 
        acquire(&(tweet_table.tag_list[hash_val].taglock));
    }
    return hash_val;
}

int get_check_tag_matches_hash_table_tag(char *tag, int orig_hash_val){
    int hash_val = orig_hash_val, count=0;
    if (strncmp(tag, tweet_table.tag_list[hash_val].tag, taglength) != 0){
        do {
            if (count > maxtweettotal){
                return orig_hash_val;
            }
            hash_val = (hash_val + 1) % maxtweettotal;
            count++;
        } while (strncmp(tag, tweet_table.tag_list[hash_val].tag, taglength) != 0); 
    }
    return hash_val;
}

/* "tweet" a 140 character message with a 10 character tag
    Will block until able to tweet */
int t_bput(char *tag, char *message){
    /* Check that hash table is initialized */
    acquire(&init_lock);
    if(0 == init_flag){
        init_tw_table();
    }
    release(&init_lock);
    
    /* Check that tweet has correct number of characters */
    if (strlen(tag) > taglength || strlen(message) > tweetlength){
        cprintf("Your tweet is not formatted correctly and will not be added to the tweet table\n");
        return -1;
    }    

    /* Check if tweet table is full */
    acquire(&tweet_table.totallock);
    while(tweet_table.currtotal >= maxtweettotal){
        sleep(&tweet_table, &tweet_table.totallock);
    }
    tweet_table.currtotal = tweet_table.currtotal + 1;
    release(&tweet_table.totallock);

    /* Find location and aquire lock for that hash location */
    int hash_val = hash(tag);
    acquire(&(tweet_table.tag_list[hash_val].taglock));
    
    /* Check if tag is already initialized */
    if(0 == tweet_table.tag_list[hash_val].numtweets){
        cp_tag(tag, tweet_table.tag_list[hash_val].tag);
    } else {
	    hash_val = put_check_tag_matches_hash_table_tag(tag, hash_val);
	    if (hash_val == -1) {
	        cprintf("No room left in tweet table after hash collision\n");
            return -1;
        } else {
            cp_tag(tag, tweet_table.tag_list[hash_val].tag);
        }
    }
    
    /* Insert tweet when possible */
    while(tweet_table.tag_list[hash_val].numtweets >= maxtweetsametag){
        sleep(tweet_table.tag_list[hash_val].waiter, 
                &(tweet_table.tag_list[hash_val].taglock));
    }
    cp_tweet(message, 
      tweet_table.tag_list[hash_val].tweets[tweet_table.tag_list[hash_val].numtweets]);
    tweet_table.tag_list[hash_val].numtweets = tweet_table.tag_list[hash_val].numtweets+1;
    
    /* Signal waiting getters */
    if(1 == tweet_table.tag_list[hash_val].numtweets){
        wakeup(tweet_table.tag_list[hash_val].waiter);
    }
    release(&(tweet_table.tag_list[hash_val].taglock));
    
    return 0;
}

/* like bput but returns immediately if unable to tweet */
int t_put(char *tag, char *message){
    /* Check that hash table is initialized */
    acquire(&init_lock);
    if(0 == init_flag){
        init_tw_table();
    }
    release(&init_lock);
    
    /* Check that tweet has correct number of characters */
    if (strlen(tag) > taglength || strlen(message) > tweetlength){
        cprintf("Your tweet is not formatted correctly and will not be added to the tweet table\n");
        return -1;
    }    

    /* Check if tweet table is full */
    acquire(&tweet_table.totallock);
    if(tweet_table.currtotal >= maxtweettotal){
        release(&tweet_table.totallock);
        return -1;
    }
    release(&tweet_table.totallock);
    
    /* Find location and aquire lock for that hash location */
    int hash_val = hash(tag);
    acquire(&tweet_table.tag_list[hash_val].taglock);

    /* Check if tag is already initialized */
    if(0 == tweet_table.tag_list[hash_val].numtweets){
        cp_tag(tag, tweet_table.tag_list[hash_val].tag);
    } else {
	    hash_val = put_check_tag_matches_hash_table_tag(tag, hash_val);
	    if (hash_val == -1) {
	        cprintf("No room left in tweet table after hash collision\n");
            return -1;
        } else {
            cp_tag(tag, tweet_table.tag_list[hash_val].tag);
        }
    }

    /* Insert tweet if possible */
    if(tweet_table.tag_list[hash_val].numtweets < maxtweetsametag){
        cp_tweet(message, tweet_table.tag_list[hash_val].tweets[tweet_table.tag_list[hash_val].numtweets]);
        tweet_table.tag_list[hash_val].numtweets = tweet_table.tag_list[hash_val].numtweets+1;
    } else {
        release(&(tweet_table.tag_list[hash_val].taglock));
        return -1;
    }
    
    /* Notify waiting getters */
    if(1 == tweet_table.tag_list[hash_val].numtweets){
        wakeup(tweet_table.tag_list[hash_val].waiter);
    }
    release(&(tweet_table.tag_list[hash_val].taglock));
    
    return 0;
}

// reads a tweet with matching tag and removes it from system copying it into buf.
// Block until able to do so
int t_bget(char *tag, char *buf){
    int hsh;
    
    /* Check that tag is valid */
    if (strlen(tag) > taglength){
        cprintf("Your tag is not formatted correctly and will not be added to the tweet table\n");
        return -1;
    }    

    hsh = hash(tag);
    hsh = get_check_tag_matches_hash_table_tag(tag, hsh);

    /* Get a tweet from that hash location when able */
    acquire(&tweet_table.tag_list[hsh].taglock);    
    while(tweet_table.tag_list[hsh].numtweets <= 0){            
        sleep(tweet_table.tag_list[hsh].waiter, &(tweet_table.tag_list[hsh].taglock));
    } 
    
    /* Copy tweet to buffer */    
    cp_tweet(tweet_table.tag_list[hsh].tweets[tweet_table.tag_list[hsh].numtweets - 1], buf);
    tweet_table.tag_list[hsh].numtweets = tweet_table.tag_list[hsh].numtweets - 1;
    
    /* Wake up sleeping putters */
    if(tweet_table.tag_list[hsh].numtweets == maxtweetsametag - 1){
        wakeup(tweet_table.tag_list[hsh].waiter);
    }

    release(&tweet_table.tag_list[hsh].taglock);
    
    /* If table was full, signal */
    acquire(&tweet_table.totallock);
    tweet_table.currtotal = tweet_table.currtotal - 1;
    if(tweet_table.currtotal == maxtweettotal -1){
        wakeup(&tweet_table);
    }    
    release(&tweet_table.totallock);
    
    return 0;
}

// like bget but returns immediately if cannot find tweet with matching tag
int t_get(char *tag, char *buf){
    int hsh;
    
    /* Check that tag is valid */
    if (strlen(tag) > taglength){
        cprintf("Your tag is not formatted correctly and will not be added to the tweet table\n");
        return -1;
    }    

    hsh = hash(tag);
    hsh = get_check_tag_matches_hash_table_tag(tag, hsh);

    acquire(&tweet_table.tag_list[hsh].taglock);
    if(0 == tweet_table.tag_list[hsh].numtweets){
        /* Case of no msg with this tag; nothing to put in buffer */
        release(&tweet_table.tag_list[hsh].taglock);
        return -1;
    } else {
        /* Get tweet if there is one */
        cp_tweet(tweet_table.tag_list[hsh].tweets[tweet_table.tag_list[hsh].numtweets - 1], buf);
        tweet_table.tag_list[hsh].numtweets = tweet_table.tag_list[hsh].numtweets - 1;
        /* Wake up sleeping putters */
        if(tweet_table.tag_list[hsh].numtweets == maxtweetsametag - 1){
            wakeup(tweet_table.tag_list[hsh].waiter);
        }
    }
    release(&tweet_table.tag_list[hsh].taglock);
    
    /* If table was full, signal */
    acquire(&tweet_table.totallock);
    tweet_table.currtotal = tweet_table.currtotal - 1;
    if(tweet_table.currtotal == maxtweettotal -1){
        wakeup(&tweet_table);
    }    
    release(&tweet_table.totallock);
    
    return 0;
}

void t_print_table(void){
    int i, j, n;
     /* Loop through tweet table and print contents */
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

