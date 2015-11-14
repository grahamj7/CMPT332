# CMPT 332 - Fall 2015
# Assignment 3, Question 2
#
# Jordaen Graham - jhg257
# Jennifer Rospad - jlr247
#
# Makefile

CC := gcc
CCFLAGS := -Wall -Wextra -pthread -g

all: clean Test_Pthread

clean:
	@rm *.o* &> /dev/null || true
	@rm *~ &> /dev/null || true
	@rm Test* &> /dev/null || true

Test_Pthread: dogwashsynch.c dogwashsynch.h test_pthread.c
	$(CC) $(CCFLAGS) -o Test_Pthread dogwashsynch.c test_pthread.c
	
