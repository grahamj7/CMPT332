CC := gcc
CCFLAGS := -Wall -pedantic

all: clean run

clean: 
	@rm *.o* &> /dev/null || true
	@rm A2_3 &> /dev/null || true

run: A2_3
	@echo ""
	@A2_3

A2_3: A2_3.c
	$(CC) $(CCFLAGS) -o A2_3 A2_3.c
