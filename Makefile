
CC := gcc
CCFLAGS := -Wall -Wextra -pthread -g

all: clean run_sem

clean:
	@rm *.o* &> /dev/null || true
	@rm *~ &> /dev/null || true
	@rm [tT]est* &> /dev/null || true

run_pthread: Test_Pthread
	./Test_Pthread

run_sem: Test_Sem
	./Test_Sem

Test_Pthread: dogwashsynch_pthread.c dogwashsynch_pthread.h
	$(CC) $(CCFLAGS) -o Test_Pthread dogwashsynch_pthread.c
	
dogwashsynch_pthread.o: dogwashsynch_pthread.c dogwashsynch_pthread.h
	$(CC) $(CCFLAGS) -c dogwashsynch_pthread.c


Test_Sem: dogwashsynch_sem.c dogwashsynch_sem.h
	$(CC) $(CCFLAGS) -o Test_Sem dogwashsynch_sem.c
	
dogwashsynch_sem.o: dogwashsynch_sem.c dogwashsynch_sem.h
	$(CC) $(CCFLAGS) -c dogwashsynch_sem.c



