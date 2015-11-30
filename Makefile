# CMPT 332 - Fall 2015
# Assignment 3, Question 3
#
# Jordaen Graham - jhg257
# Jennifer Rospad - jlr247
#
# File: Makefile

CC := gcc
CCFLAGS := -Wall -Wextra -pedantic -pthread -g

all: clean Server Recv_Client Send_Client

clean:
	@rm *.o* &> /dev/null || true
	@rm *~ &> /dev/null || true
	@rm *Client &> /dev/null || true
	@rm Server &> /dev/null || true

run_server: Server
	@./Server

run_recv_client: Recv_Client
	@./Recv_Client "localhost" "30491"

run_send_client: Send_Client
	@./Send_Client localhost 30490

Server: server.c server.h
	$(CC) $(CCFLAGS) -o Server server.c

Client: Recv_Client Send_Client

Recv_Client: recv_client.c server.h
	$(CC) $(CCFLAGS) -o Recv_Client recv_client.c

Send_Client: send_client.c server.h
	$(CC) $(CCFLAGS) -o Send_Client send_client.c


