#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *resize(char *buf, size_t new)
{
	char *tmp = realloc(buf, new);
	printf("---Resize to %d\n", (int)new);
	if(tmp == NULL)
		exit(2);
	return tmp;
}

char *readline(FILE *fp, int strip)
{
	int shrink_thresh = 73;
	int nogrow_thresh = 4;
	static char *buf = NULL;
	static int bufsize = 35;
	static int offset = 0;
	static int n_nogrow = 0;

	if(buf == NULL)	{
		buf = malloc(bufsize);
		if(buf == NULL)
			return NULL;
	}

	while(fgets(buf + offset, bufsize - 1, fp) != NULL)	{

		size_t len = strlen(buf);
		if(buf[len - 1] != '\n' && !feof(fp))	{
			bufsize *= 2;
			buf = resize(buf, bufsize);
			offset = len;
			n_nogrow = 0;
			continue;
		}
		offset = 0;
		if(bufsize > shrink_thresh && n_nogrow > nogrow_thresh && len < bufsize / 2)	{
			bufsize /= 2;
			buf = resize(buf, bufsize);
			n_nogrow = 0;
		}
		
		n_nogrow++;
		if(strip)
			buf[len - 2] = '\0';
		return buf;
	}
	free(buf);
	return NULL;
}


int main(int argc, char const *argv[])
{
	char *bf;
	FILE *fp;

	if(argc > 1)
	 	fp = fopen(argv[1], "r");
	else
		fp = stdin;


	if(fp == NULL)
		return 1;

	while((bf = readline(fp, 0)) != NULL)	{
		printf("%s", bf);
	}

	fclose(fp);

	return 0;
}