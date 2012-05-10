#include <stdio.h>
#include <stdlib.h>

#include "readline.h"

int main(int argc, char const *argv[])
{
	char *bf;
	size_t a;

	if(argc > 1)	{
		while((bf = readline_continue(argv[1], &a)) != NULL)	{
			printf("(%4d): %s\n", (int)a, bf);
			free(bf);
		}
		if(readline_error())	{
			puts(readline_errstr());
		}
	} else {
		readline_set_strip();
		while((bf = readline_fp(stdin, &a)) != NULL)	{
			printf("(%4d): %s\n", (int)a, bf);
		}
		if(readline_error())	{
			puts(readline_errstr());
		}
	}

	return 0;
}
