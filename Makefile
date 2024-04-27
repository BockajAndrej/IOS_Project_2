CFLAGS=-std=gnu11 -Wall -Wextra  -g 

all: proj2 main skuska

proj2: proj2.c
	gcc $(CFLAGS) proj2.c -o proj2 -lm

main: main.c
	gcc $(CFLAGS) main.c -o main -lm

skuska: skuska.c
	gcc $(CFLAGS) skuska.c -o skuska -lm
clean:
	rm -rf *.o main proj2 skuska