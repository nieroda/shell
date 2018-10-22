# lobo-shell
[![Build Status](https://travis-ci.com/ezquire/Simple-Shell.svg?token=xPzuzD2CR5yXW6oamq47&branch=master)](https://travis-ci.com/ezquire/Simple-Shell)

A simple shell written in C that supports redirection and piping

## Group Members
Nathan Kamm, Tyler Gearing, Ezio Ballarin


Usage
-----

To build the shell, run
```
make
```

Then you can run the shell from the current directory using the two executables
```
./lobo-shell.x - runs the shell with basic functionality
./ec.x - runs the shell with extended functionality
```

lobo-shell.x
-----

lobo-shell.x supports some basic commands like
```
ps -u yourusername
```
```
who | wc -l
```
```
ls /usr/bin | head -10 | tail -5
```
```
wc -l < [inputfile]
who > [outputfile]
wc -l < [inputfile] >> [outputfile]
```
```
tr "A-Z" "a-z" < [inputfile] | tr -cs "a-z" '\012' | sort | uniq -c | sort -nr | head -1 > [outputfile]
```

ec.x
-----

ec.x supports commands in this form, as well as commands in all the forms listed under lobo-shell.x
```
who 1>[filename]
```
```
who 2>[filename]
```
```
who 2>[filename] 1>&2
```
```
who > [filename] 2>&1
```

Testing
-----

To run provided unit tests run
```
make
./test
```

Misc
-----

To clear the output files and executables use
```
make clean
```
