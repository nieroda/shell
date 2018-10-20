CFLAGS = -g -Iinclude -std=c99 -Wall -Werror -pedantic
CC = gcc

# this says go find all the files in src/ with .c as an extension and put
# them in a string separated by spaces
srcs = $(wildcard src/*.c)

# this is a shortcut to name .o files from your source files, it says find
# all the srcs with .c and replace that extension with .o and put them
# in a string separated by spaces
objs = $(srcs:.c=.o)

all:  lobo_shell.x ec.x

lobo_shell.x: $(objs)
	$(CC) $(CFLAGS) -o $@ $^

ec.x: $(objs)
	$(CC) $(CFLAGS) -o $@ $^ -D EXTRACREDIT

clean:
	rm -f *.x *.o *~

# this is for makefile optimization it basically says don't try to make
# these things into files they are just rules, it also helps with name
# resolution conflicts in case you want to name something "clean" or "all"
.PHONY: all clean

# slightly cleaner syntax than gondree's :D
