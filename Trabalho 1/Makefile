all: magic

magic: magic.o utils.o
	gcc -Wall magic.o utils.o -o magic -std=c11

magic.o: magic.c
	gcc -Wall -c magic.c

utils.o: utils.c utils.h 
	gcc -Wall -c utils.c
