CC=gcc
CFLAGS=-Wall -O2

all: interp rsort

interp: interp.c
	$(CC) $(CFLAGS) -o interp interp.c

rsort: rsort.c
	$(CC) $(CFLAGS) -o rsort rsort.c

clean:
	rm -f rsort interp
