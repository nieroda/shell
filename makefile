CFLAGS = -Iinclude -std=c99
CC = gcc

all:  lobo_shell.x

#pipe_demo.x

# $@ is make shorthand for the thing on the left side of the colon
#   (pipe_demo.x in this case)
# $^ stands for the dependencies, everything to right of colon (the .o files)
# $< stands for the first dependency
#pipe_demo.x: pipe_demo.o
#	$(CC) -g -o $@ $^

lobo_shell.x: main.o parsetools.o
	$(CC) -g -o $@ $^

# $< is the first item after the colon (main.c here)
main.o: main.c include/parsetools.h include/constants.h
	$(CC) $(CFLAGS) -g -c -o $@ $<

parsetools.o: src/parsetools.c include/constants.h
	$(CC) $(CFLAGS) -g -c -o $@ $<

#pipe_demo.o: example/pipe_demo.c
#	$(CC) $(CFLAGS) -g -c -o $@ $<

clean:
	rm -f *.x *.o *~

