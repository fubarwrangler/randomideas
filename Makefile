all: interp rsort

interp:
	gcc -Wall -o interp interp.c

rsort:
	gcc -Wall -o rsort rsort.c
