

CC := gcc
CCFLAGS := -Wall -Wextra -pthread -g

all: clean Test

clean: 
	@rm *.o* &> /dev/null || true
	@rm *~ &> /dev/null || true
	@rm [tT]est &> /dev/null || true

Test: dogwashsynch.c dogwashsynch.h
	$(CC) $(CCFLAGS) -o Test dogwashsynch.c
	./Test

dogwashsynch.o: dogwashsynch.c dogwashsynch.h
	$(CC) $(CCFLAGS) -c dogwashsynch.c

test.o: dogwashsynch.c dogwashsynch.h
	$(CC) $(CCFLAGS) -c dogwashsynch.c

