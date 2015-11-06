
CC := gcc
CCFLAGS := -Wall -Wextra -pthread -g

NUMBAYS :=
NUMDOGS :=

all: clean pthread space sem

clean:
	@rm *.o* &> /dev/null || true
	@rm *~ &> /dev/null || true
	@rm Test* &> /dev/null || true

space:
	@echo ""
	@echo ""
	@echo ""

pthread: Test_Pthread
	./Test_Pthread $(NUMBAYS) $(NUMDOGS)

sem: Test_Sem
	./Test_Sem $(NUMBAYS) $(NUMDOGS)

Test_Pthread: dogwashsynch_pthread.c dogwashsynch_pthread.h test_pthread.c
	$(CC) $(CCFLAGS) -o Test_Pthread dogwashsynch_pthread.c test_pthread.c
	
Test_Sem: dogwashsynch_sem.c dogwashsynch_sem.h test_sem.c
	$(CC) $(CCFLAGS) -o Test_Sem dogwashsynch_sem.c test_sem.c
	
