CC := gcc
CCFLAGS := -Wall -Wextra

all: clean run

clean: 
	@rm *.o* &> /dev/null || true
	@rm *.a &> /dev/null || true
	@rm A2_3 &> /dev/null || true

run: A2_3
	@echo ""
	@A2_3

A2_3: A2_3.c
	$(CC) $(CCFLAGS) -g -o A2_3 A2_3.c

A2_3.o: A2_3.c
	$(CC) $(CCFLAGS) -g -c A2_3.c

library: A2_3.o
	@ar -cq mem.a *.o
	@echo "Library mem.a created"

