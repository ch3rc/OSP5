CC = gcc
CFLAGS = -I. -g -Wall
OBJS = setShare.o resources.o
.SUFFIXES: .c .o

all: oss userP

oss: oss.o $(OBJS)
	$(CC) $(CFLAGS) -o $@ oss.o $(OBJS) -pthread

userP: userP.o $(OBJS)
	$(CC) $(CFLAGS) -o $@ userP.o $(OBJS) -pthread

.c.o:
	$(CC) $(CFLAGS) -c $<

.PHONY: clean
clean:
	rm *.o *.txt oss userP
