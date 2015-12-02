
/* CMPT 332 - Fall 2015
 * Assignment 4, Question 1
 *
 * Jordaen Graham - jhg257
 * Jennifer Rospad - jlr247
 *
 * File: server.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include <ifaddrs.h>
#include "server.h"


#define BACKLOG 10     /* how many pending connections queue will hold */


struct clients{
    int fd;
    char *address;
    struct clients *next;
}*recv_list;

struct message_queue{
    char* message;
    struct message_queue *next;
}*MessageQueue;


int recv_size=0, queue_size=0;
pthread_mutex_t recv_mutex, queue_mutex;
pthread_cond_t cond;


void sigchld_handler(int s) {
    /* waitpid() might overwrite errno, so we save and restore it: */
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}

/* get sockaddr, IPv4 or IPv6: */
void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

struct clients *get_connections(int server_fd) {
    int client_fd; /*return */
    socklen_t sin_size;
    char s[INET6_ADDRSTRLEN];
    struct sockaddr_storage their_addr; /* connector's address information */
    struct clients *client;

    sin_size = sizeof their_addr;
    client_fd = accept(server_fd, (struct sockaddr *)&their_addr, &sin_size);
    if (client_fd == -1) {
        perror("accept");
        return NULL;
    }

    inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), (char *) s, INET6_ADDRSTRLEN);

    printf("server: got connection from %s\n", s);
    fflush(stdout);
    client = malloc(sizeof(struct clients));
    client->fd = client_fd;
    client->address = malloc(INET6_ADDRSTRLEN* sizeof(char));
    snprintf(client->address, INET6_ADDRSTRLEN, "%s", s);
    client->next = NULL;
    return client;
}

char *recvMessage(int client_fd){
    ssize_t numbytes;
    char *buf = malloc(MAXDATASIZE * sizeof(char));

    if ((numbytes = recv(client_fd, buf, MAXDATASIZE, 0)) == -1) {
        perror("recv");
        exit(1);
    }
    else if (numbytes == 0){
        buf = "abort";
    }
    else
        buf[numbytes] = '\0';
    return buf;
}

int sendMessage(int client_fd, char *message) {
    printf("send to client: %d\n", client_fd);
    if (send(client_fd, message, strlen(message)+1, 0) == -1) {
        fprintf(stdout, "Error: sending to %d\n", client_fd);
        return -1;
    }
    printf("finished sending to client: %d\n", client_fd);
    return 0;
}

void addThreadToList(int client_fd, char *address) {
    struct clients *client;
    pthread_mutex_lock(&recv_mutex);

    client = (struct clients *)malloc(sizeof(struct clients));
    client->fd = client_fd;
    client->address = address;
    if (recv_list == NULL)
    {
        recv_list = client;
        recv_list->next=NULL;
    }
    else
    {
        client->next = recv_list;
        recv_list = client;
    }
    pthread_mutex_unlock(&recv_mutex);
}

void removeThreadFromList(int client_fd) {
    struct clients *cursor, *prev;
    pthread_mutex_lock(&recv_mutex);
    
    prev = NULL;
    cursor = recv_list;
    printf("Remove FD: %d\n", client_fd);
    while(cursor != NULL)
    {
        if(cursor->fd == client_fd)
        {
            if(cursor == recv_list)
            {
                recv_list = cursor->next;
                free(cursor);
            }
            else
            {
                prev->next = cursor->next;
                free(cursor);
            }
            break;
        }
        else
        {
            prev = cursor;
            cursor = cursor->next;
        }
    }
    close(client_fd);
    printf("Remove: \n");
    for(cursor = recv_list; cursor != NULL; cursor=cursor->next){
        printf("FD: %d\n", cursor->fd);
    }
    pthread_mutex_unlock(&recv_mutex);
}

void addToMessageQueue(char *message){
    struct message_queue *queue;
    pthread_mutex_lock(&queue_mutex);
    
    queue = (struct message_queue *)malloc(sizeof(struct message_queue));
    queue->message = message;
    if (MessageQueue == NULL)
    {
        MessageQueue = queue;
        MessageQueue->next=NULL;
    }
    else
    {
        queue->next = MessageQueue;
        MessageQueue = queue;
    }
    queue_size++;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&queue_mutex);
}

char *readFromMessageQueue(){
    struct message_queue *temp;
    char *message;
    
    message = "Failure";
    pthread_mutex_lock(&queue_mutex);
    if (MessageQueue != NULL){
        message = MessageQueue->message;
        temp = MessageQueue;
        MessageQueue = MessageQueue->next;
        free(temp);
    }
    queue_size--;
    pthread_mutex_unlock(&queue_mutex);
    return message;
}


void sendToAllClients(char *buf) {
    struct clients *cursor;
    int i, *bad_list, bad_size;
    pthread_mutex_lock(&recv_mutex);
    

    bad_size = 0;
    bad_list = malloc(recv_size * sizeof(int));
    cursor = recv_list;
    for (; cursor != NULL; cursor = cursor->next) {
        if (sendMessage(cursor->fd, buf) == -1) {
            printf("bad client_fd: %d\n", cursor->fd);
            bad_list[bad_size] = cursor->fd;
            bad_size++;
        }
    }
    pthread_mutex_unlock(&recv_mutex);
    if (bad_size > 0) {
        if (bad_list != NULL) {
            printf("badlist not null\n");
            for (i = 0; i < bad_size; i++) {
                fprintf(stderr, "Send error in client: %d\n", bad_list[i]);
                if (bad_list[i] == -1)
                    break;
                fprintf(stderr, "Send error in client: %d\n", bad_list[i]);
                removeThreadFromList(bad_list[i]);
            }
        }
    }
}







































void *client_receive_func() {
    char *buf=NULL;

    while(1) {
        /* wait for signal from cond */
        pthread_mutex_lock(&queue_mutex);
        while(queue_size == 0)	{
	        pthread_cond_wait(&cond, &queue_mutex); 
        }
        pthread_mutex_unlock(&queue_mutex);
        buf = readFromMessageQueue();
        pthread_mutex_lock(&queue_mutex);
        if (strcmp("Failure", buf) != 0){
            sendToAllClients(buf);
        }
        pthread_mutex_unlock(&queue_mutex);
    }
    pthread_exit(NULL);
}

void *client_send_func(void *args) {
    int client_fd, size;
    char *message, *buf=NULL;
    struct clients *client;

    client = (struct clients*) args;
    client_fd = client->fd;
    
    while(1) {
        buf = recvMessage(client_fd);
        if (strcmp(buf, "/quit") == 0)
            break;
        else if (strcmp(buf, "abort") == 0){
            pthread_exit(NULL);
        }
	    size = strlen(client->address)+strlen(SENDPORT)+ strlen(buf) + 4;
        message = malloc(size);
        if (buf[0] == '/')
            ++buf;
        snprintf(message, size, "%s, %d: %s", client->address, atoi(SENDPORT), 
            buf);
        addToMessageQueue(message);    
        /* wake up waiting thread with condition variable */
    }
    close(client_fd);
    pthread_exit(NULL);
}




















void *run_server_get_receives(void *args) {
    pthread_t client_thread;
    struct clients *client;
    /* Pointer to the location of all client thread */
    int server_fd, *ptr;
    ptr = (int*) args;
    server_fd = *ptr;

    /* allocate space for the client_thread */
    client_thread = (pthread_t) malloc(sizeof(pthread_t));
    if (client_thread == 0){
        perror("create thread");
        pthread_exit(NULL);
    }

    /* Create and start thread */
    if (pthread_create(&client_thread, NULL, client_receive_func, NULL) == -1){
        perror("start pthread");
        pthread_exit(NULL);
    }

    while(1) {  /* main accept() loop */
        client = get_connections(server_fd);
        if (client->fd == -1) {
            continue;
        }

        addThreadToList(client->fd, client->address);
    }
    pthread_exit(NULL);
}

void *run_server_get_sends(void *args) {
    pthread_t client_thread;
    struct clients *client;
    /* Pointer to the location of all client thread */
    int server_fd, *ptr;
    ptr = (int*) args;
    server_fd = *ptr;

    /* allocate space for the client_thread */
    client_thread = (pthread_t) malloc(sizeof(pthread_t));
    if (client_thread == 0){
        perror("create thread");
        pthread_exit(NULL);
    }

    while(1) {  /* main accept() loop */
        client = get_connections(server_fd);
        
        if (client->fd == -1) {
            continue;
        }

    	/* Create and start thread */
        if (pthread_create(&client_thread, NULL, client_send_func, client) == -1){
            perror("start pthread");
            pthread_exit(NULL);
        }
    }
    pthread_exit(NULL);
}























int *setup(const char *port){
    struct addrinfo hints, *servinfo, *p;
    struct sigaction sa;
    
    /* listen for connection on server_fd, communicates on client_fd */
    int rv, yes = 1, *server_fd = NULL;  
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; /* use my IP */

    if ((rv = getaddrinfo(NULL, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        exit(1);
    }

    server_fd = malloc(sizeof(int));
    /* loop through all the results and bind to the first we can */
    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((*server_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(*server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(*server_fd, p->ai_addr, p->ai_addrlen) == -1) {
            close(*server_fd);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo); /* all done with this structure */

    if (p == NULL) {
        perror("server: failed to bind");
        exit(1);
    }

    if (listen(*server_fd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    sa.sa_handler = sigchld_handler; /* reap all dead processes */
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }
    return server_fd;
}

int main(void) {
   
    struct ifaddrs *addrs, *tmp;
    struct sockaddr_in *pAddr = NULL;    
    
    int *client_send_fd = NULL, *client_receive_fd = NULL;
    pthread_t client_send_thread, client_recv_thread;

    client_send_fd = setup(SENDPORT);    
    client_receive_fd = setup(RECEIVEPORT);    

    getifaddrs(&addrs);
    tmp = addrs;
    printf("Addresses: [\n");
    while (tmp)
    {
        if (tmp->ifa_addr && tmp->ifa_addr->sa_family == AF_INET)
        {
            pAddr = (struct sockaddr_in *)tmp->ifa_addr;
            printf("\t%s, %s\n", tmp->ifa_name, inet_ntoa(pAddr->sin_addr));
        }
        tmp = tmp->ifa_next;
    }
    freeifaddrs(addrs);
    printf("]\n");

    /* Server prints out all available addresses, you can access the server with: 
    ./Send_Client ip_address or ./Recv_Client ip_address
    */

	if (0 != pthread_cond_init(&cond, NULL)){
        perror("init cond");
        exit(1);
    }
    if (0 != pthread_mutex_init(&recv_mutex, NULL)) {
        perror("init recv_mutex");
        exit(1);
    }
    

    recv_list = NULL;
    MessageQueue = NULL;
    signal(SIGPIPE, SIG_IGN);

    printf("server: waiting for clientSenders on Port: %s\n", SENDPORT);
    printf("server: waiting for clientRecievers on Port: %s\n", RECEIVEPORT);
    fflush(stdout);
    
    client_send_thread = (pthread_t) malloc(sizeof(pthread_t));
    if (client_send_thread == 0){
        perror("create thread");
        return -1;
    }
    client_recv_thread = (pthread_t) malloc(sizeof(pthread_t));
    if (client_recv_thread == 0){
        perror("create thread");
        return -1;
    }

    /* Create and start client senders thread */
    if (pthread_create(&client_send_thread, NULL, run_server_get_sends, client_send_fd) == -1){
        perror("start pthread");
        return -1;
    }

    /* Create and start client receivers thread */
    if (pthread_create(&client_recv_thread, NULL, run_server_get_receives, client_receive_fd) == -1){
        perror("start pthread");
        return -1;
    }
    
    if (pthread_join(client_send_thread, NULL) == -1){
        perror("join pthread");
        return -1;
    }
    if (pthread_join(client_recv_thread, NULL) == -1){
        perror("join pthread");
        return -1;
    }

    if (pthread_cond_destroy(&cond)) {
        perror("destroy cond");
        exit(1);
    }
    if (pthread_mutex_destroy(&recv_mutex)) {
        perror("destroy recv_mutex");
        exit(1);
    }

    return 0;
}

