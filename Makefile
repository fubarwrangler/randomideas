CC=gcc
CFLAGS=-Wall -g

all: interp rsort readline

readline: readline.c
	$(CC) $(CFLAGS) -o readline readline.c

interp: interp.c
	$(CC) $(CFLAGS) -o interp interp.c

rsort: rsort.c
	$(CC) $(CFLAGS) -o rsort rsort.c

clean:
	rm -f rsort interp readline
