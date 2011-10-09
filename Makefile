all: interp rsort

interp: interp.c
	gcc -Wall -o interp interp.c

rsort: rsort.c
	gcc -Wall -o rsort rsort.c

clean:
	rm rsort interp
