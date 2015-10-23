CC := gcc
CCFLAGS := -Wall -Wextra -g

all: clean run

clean: 
	@rm *.o* &> /dev/null || true
	@rm *.h.gch &> /dev/null || true
	@rm *.a &> /dev/null || true
	@rm *~ &> /dev/null || true
	@rm A2_3 &> /dev/null || true
	@rm [tT]est &> /dev/null || true

run: Test
	@echo ""
	@Test

Test: A2_3.c A2_3.h test.c
	$(CC) $(CCFLAGS) -o Test A2_3.c A2_3.h test.c

A2_3.o: A2_3.c A2_3.h
	$(CC) $(CCFLAGS) -c A2_3.c A2_3.h

test.o: test.c A2_3.h
	$(CC) $(CCFLAGS) -c test.c A2_3.h

test_library: library test.o A2_3.h
	$(CC) $(CCFLAGS) test.c -o test libmem.a

library: A2_3.o
	@ar -cq libmem.a A2_3.o
	@ranlib libmem.a
	@echo "Library libmem.a created"

