CFLAGS=-std=gnu99 -Wall -Wextra -Werror -pedantic

all: proj2 

proj2: proj2.c
	gcc $(CFLAGS) proj2.c -o proj2 -lm

clean:
	rm -rf *.o proj2