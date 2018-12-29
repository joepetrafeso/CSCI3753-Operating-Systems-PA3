#Makefile
CC = gcc
INCLUDE = /usr/lib
LIBS = -lpthread
OBJS =

all: multi-lookup

multi-lookup: multi-lookup.c
	$(CC) -o multi-lookup multi-lookup.c queue.c util.c $(CFLAGS) $(LIBS)

clean:
	rm -f multi-lookup
