#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "readline.h"

static char *resize(char *buf, size_t new)
{
	char *tmp = realloc(buf, new);
	printf("---Resize to %d\n", (int)new);
	if(tmp == NULL)
		exit(2);
	return tmp;
}
char *readline_fp(FILE *fp, size_t *slen, char strip)
{
	size_t len = 0;
	static char *buf = NULL;
	static int bufsize = 80;
	static int offset = 0;
	static int n_nogrow = 0;

	if(buf == NULL)	{
		buf = malloc(bufsize);
		if(buf == NULL)
			return NULL;
	}

	while(fgets(buf + offset, bufsize - offset - 1, fp) != NULL)	{
		len = strlen(buf);
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
		*slen = len;
		return buf;
	}
	free(buf);
	return NULL;
}
