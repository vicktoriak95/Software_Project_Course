FLAGS = -ansi -Wall -Wextra -Werror -pedantic-errors
LIBS = -lm

all: ex2

clean:
	rm -rf *.o ex2

ex2: spmat.o main.o
	gcc spmat.o main.o -o ex2 $(LIBS)
	
main.o: main.c
	gcc $(FLAGS) -c main.c

spmat.o: spmat.c
	gcc $(FLAGS) -c spmat.c
