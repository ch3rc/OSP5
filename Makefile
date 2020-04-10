CC = gcc
CFLAGS = -I. -g -Wall
OBJS = setShare.o
.SUFFIXES: .c .o

all: oss userP

oss: oss.o $(OBJS)
	$(CC) $(CFLAGS) -o $@ oss.o $(OBJS) -lpthread

userP: userP.o $(OBJS)
	$(CC) $(CFLAGS) -o $@ userP.o $(OBJS) -lpthread

.c.o:
	$(CC) $(CFLAGS) -c $<

.PHONY: clean
clean:
	rm *.o oss userP
