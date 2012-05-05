#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int _readl_init_buf = 78;
int _readl_shrink_thres = 73;
int _readl_skip_shrink = 4;
char _readl_error = 0;
char _readl_strip = 0;

static char *resize(char *buf, size_t new)
{
	char *tmp = realloc(buf, new);
	printf("---Resize to %d\n", (int)new);
	if(tmp == NULL)
		exit(2);
	return tmp;
}

char *readline_fp(FILE *fp, size_t *slen)
{
	size_t len = 0;
	static char *buf = NULL;
	static int bufsize = 0;
	static int offset = 0;
	static int n_nogrow = 0;

	if(buf == NULL)	{
		buf = malloc(_readl_init_buf);
		if(buf == NULL)
			return NULL;
		bufsize = _readl_init_buf;
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
		if( (bufsize / 2 > _readl_shrink_thres) &&
		    (n_nogrow > _readl_skip_shrink) &&
		    (len < bufsize / 2)
		  )	{

			bufsize /= 2;
			buf = resize(buf, bufsize);
			n_nogrow = 0;
		}

		n_nogrow++;
		if(_readl_strip)
			buf[len - 2] = '\0';
		*slen = len;
		return buf;
	}
	free(buf);
	return NULL;
}

char *readline(const char *fname, int *slen)
{
	static FILE *fp = NULL;

	if(fp == NULL)	{
		if((fp = fopen(fname, "r")) == NULL)	{
			_readl_error = 1;
			return NULL;
		}
	} else {
		char *storage = readline_fp(fp, slen);
		if(storage == NULL)
			fclose(fp);
		return storage;
	}
}
