.PHONY: all clean

#compiler c
CC = gcc
CFLAGS = -D_POSIX_SOURCE -Wall -Werror -pedantic -std=c99 -D_GNU_SOURCE -pthread

###################################

#c files
SOURCE = $(wildcard *.c) #all .c files
HEADER = $(SOURCE:.c=.h)
OBJS = $(SOURCE:.c=.o)

#standard rules

all: serverSingle serverThreaded client

serverSingle: $(CURDIR)/Server/server.c $(CURDIR)/Server/filehandling.c
	$(CC) $(CFLAGS) -o serverSingle $(CURDIR)/Server/server.c $(CURDIR)/Server/filehandling.c

serverThreaded: $(CURDIR)/Server/server.c $(CURDIR)/Server/filehandling.c
	$(CC) $(CFLAGS) -o serverThreaded $(CURDIR)/Server/server.c $(CURDIR)/Server/filehandling.c

client: $(CURDIR)/Client/client.c $(CURDIR)/Client/read.c $(CURDIR)/Client/socket.c
	$(CC) $(CFLAGS) -o client $(CURDIR)/Client/client.c $(CURDIR)/Client/read.c $(CURDIR)/Client/socket.c

clean:

###################################

#overwrite suffix rules to enforce our rules
.SUFFIXES:

%.o: %.c
	$(CC) -c $(CFLAGS) $<

#EOF

.PHONY: all test clean