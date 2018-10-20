CFLAGS = -g -Iinclude -std=c99 -Wall -Werror -pedantic
CC = gcc

srcs = $(wildcard src/*.c)

objs = $(srcs:.c=.o)

all:  lobo_shell.x ec.x

lobo_shell.x: $(objs)
	$(CC) $(CFLAGS) -o $@ $^

ec.x: $(objs)
	$(CC) $(CFLAGS) -o $@ $^ -D EXTRACREDIT

clean:
	rm -f *.x *.o *~

.PHONY: all clean
