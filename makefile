CC = gcc
CFLAGS = -Wall -Wextra -pedantic -ansi
DEBUG ?= 0

ifeq ($(DEBUG),1)
  CFLAGS += -DDEBUG
endif

OBJS = main.o locker.o compress.o crypto.o util.o storage.o

locker: $(OBJS)
	$(CC) $(CFLAGS) -o locker $(OBJS)

main.o: main.c locker.h
	$(CC) $(CFLAGS) -c main.c

locker.o: locker.c locker.h
	$(CC) $(CFLAGS) -c locker.c

compress.o: compress.c compress.h
	$(CC) $(CFLAGS) -c compress.c

crypto.o: crypto.c crypto.h
	$(CC) $(CFLAGS) -c crypto.c

util.o: util.c util.h
	$(CC) $(CFLAGS) -c util.c

storage.o: storage.c storage.h locker.h
	$(CC) $(CFLAGS) -c storage.c

.PHONY: clean debug

clean:
	rm -f *.o locker

debug:
	$(MAKE) DEBUG=1
