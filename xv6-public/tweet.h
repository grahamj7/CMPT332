#include "types.h"
#include "defs.h"
#include "param.h"
#include "spinlock.h"
typedef struct tweet {
    char *t_tag;
    char *t_message;
} tweet;

typedef struct tweet_list {
    int num_tags_filled;
    tweet tw_tag_list[maxtweetsametag];
    struct spinlock taglock;
} tweet_list;

typedef struct tw_table {
    tweet_list tw_entries[maxtweettotal/maxtweetsametag];
}tw_table;

int t_bput(char*, char*);
int t_put(char*, char*);
int t_bget(char*, char*);
int t_get(char*, char*);
void print_tweets(void);

