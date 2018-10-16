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

Then you can run the shell from the current directory
```
./lobo-shell.x
```

The shell supports some basic commands like
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
wc -l < inputfile
who > outputfile
wc -l < inputfile >> outputfile
```
```
tr "A-Z" "a-z" < wcExample.cpp | tr -cs "a-z" '\012' | sort | uniq -c | sort -nr | head -1 > output.txt
```

To clear the output files and executables use
```
make clean
```
