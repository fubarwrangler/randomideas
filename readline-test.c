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

	while((bf = readline_fp(fp, &a, 0)) != NULL)	{
		printf("%s", bf);
	}

	if(fp != stdin)
		fclose(fp);

	return 0;
} 
