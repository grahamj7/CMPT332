all: clean oddshell

clean: 
  rm *.o
  rm oddshell

oddshell: oddshell.o
  oddshell

oddshell.o: oddshell.c
  gcc -Wall -Wextra -o oddshell oddshell.c
  
gdb: oddshell.c
  gcc -Wall -Wextra -g -o gdb_oddshell oddshell.c
  gdb gdb_oddshell
  
