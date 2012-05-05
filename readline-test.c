#include <stdio.h>

#include "readline.h"

int main(int argc, char const *argv[])
{
	char *bf;
	FILE *fp;
	size_t a;

	if(argc > 1)
		fp = fopen(argv[1], "r");
	else
		fp = stdin;

	if(fp == NULL)
		return 1;

	readline_set_init_size(12);
	readline_set_shrink_thresh(66);
	readline_set_n_skip_shrink(3);

	while((bf = readline_fp(fp, &a)) != NULL)	{
		printf("%s", bf);
	}
	if(readline_error())	{
		return 1;
	}

	if(fp != stdin)
		fclose(fp);

	return 0;
}
