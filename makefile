CC = gcc
CFLAGS = -Wall -g
LDFLAGS =

libmlpt.a: mlpt.o
	ar rcs libmlpt.a mlpt.o

main: main.o libmlpt.a
	$(CC) $(LDFLAGS) -o main main.o -L. -lmlpt

mlpt.o: ../tlb/mlpt.c ../tlb/mlpt.h config.h
	$(CC) $(CFLAGS) -c mlpt.c 

clean: 
	rm --force main *.o *.a

all: main libmlpt.a
