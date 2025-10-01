CC = gcc
CFLAGS = -Wall -Werror - ansi -lm

# Compile 

/* locker.out: main.o locker.o storage.o crypto.o compress.o util.o
	$(CC) $(CFLAGS) -o locker.out main.o locker.o storage.o crypto.o compress.o util.o */

  main.o main.c locker.h
    $(CC) $(CFLAGS) -c main.c
  
  locker.o: locker.c locker.h storage.h crypto.h compress.h util.h
    $(CC) $(CFLAGS) -c safe.c

  storage.o: storage.c storage.h locker.h crypto.h
    $(CC) $(CFLAGS) -c storage.c
    
  crypto.o: crypto.c crypto.h
    $(CC) $(CFLAGS) -c crypto.c

compress.o: compress.c compress.h
    $(CC) $(CFLAGS) -c compress.c

util.o: util.c util.h
    $(CC) $(CFLAGS) -c util.c

# Clean
clean:
  rm main.o locker.o storage.o crypto.o compress.o util.o /* add in whatever is the out file i.e. locker.out*/

# debug
 /* debug CFLAGS += -G DDBUG */ 
 debug: clean all

/*********************************************************************************
Example:
#makefile for generating L08a.out

CC = gcc
CFLAGS = -Wall -Werror -ansi -lm

L08a.out: L08a_dev0.o L08planet_dev1.o L08planet_dev2.o
	$(CC) $(CFLAGS) -o L08a.out L08a_dev0.o L08planet_dev1.o L08planet_dev2.o 

L08a_dev0.o: L08a_dev0.c L08planet.h 
	$(CC) $(CFLAGS) -c  -o L08a_dev0.o L08a_dev0.c

L08planet_dev1.o: L08planet_dev1.c L08planet.h
	$(CC) $(CFLAGS) -c  -o L08planet_dev1.o L08planet_dev1.c

L08planet_dev2.o: L08planet_dev2.c L08planet.h
	$(CC) $(CFLAGS) -c  -o L08planet_dev2.o L08planet_dev2.c

clean:
	rm L08a_dev0.o L08planet_dev1.o L08planet_dev2.o L08a.out
*********************************************************************************//
