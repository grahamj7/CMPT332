/* CMPT 332 - Fall 2015
 * Assignment 4, Question 1
 *
 * Jordaen Graham - jhg257
 * Jennifer Rospad - jlr247
 *
 * File: client.c 
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "server.h"

void sendMessage(int socket_fd, char *message) {
    if(send(socket_fd, message, strlen(message), 0) == -1)
        perror("sending message");
}

int run_send_client(int socket_fd){
    char *buf = malloc(MAXDATASIZE * sizeof(char)), *message = malloc(MAXDATASIZE * sizeof(char));
    size_t end;

    while(1){
        printf("prompt>> ");
        buf = fgets(buf, MAXDATASIZE-2, stdin);
        end = strlen(buf) - 1;
        if (buf[end] == '\n')
            buf[end] = '\0';
        snprintf(message, MAXDATASIZE, "/%s", buf);
        sendMessage(socket_fd, message);
       if(strcmp(buf, "quit") == 0)
            break;
    }

    close(socket_fd);
    return 0;
}

/* get sockaddr, IPv4 or IPv6: */
void *get_in_addr(struct sockaddr *sa)  {
    if (sa->sa_family == AF_INET)
        return &(((struct sockaddr_in*)sa)->sin_addr);

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[]) {
    int socket_fd = -1, rv;
    char s[INET6_ADDRSTRLEN], *host;
    struct addrinfo hints, *servinfo, *p;

    if (argc != 2)
        host = "localhost";
    else
        host = argv[1];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(host, SENDPORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    /* loop through all the results and connect to the first we can */
    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((socket_fd = socket(p->ai_family, p->ai_socktype,
                                p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(socket_fd, p->ai_addr, p->ai_addrlen) == -1) {
            close(socket_fd);
            perror("client: connect");
            continue;
        }

        break;
    }

    if (p == NULL) {
        perror("client: failed to connect");
        exit(2);
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *) p->ai_addr),
              s, sizeof s);
    printf("client: connecting to %s\n", s);

    freeaddrinfo(servinfo); /* all done with this structure */

    return run_send_client(socket_fd);
}
