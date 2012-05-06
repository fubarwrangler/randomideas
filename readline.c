#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int _readl_init_buf = 80;
int _readl_shrink_thres = 90;
int _readl_skip_shrink = 4;
char _readl_error = 0;
char _readl_strip = 0;

// #define _READLINE_DEBUG

static char *resize(char *buf, size_t new)
{
	char *tmp = realloc(buf, new);
#ifdef _READLINE_DEBUG
	fprintf(stderr, "---Resize to %d\n", (int)new);
#endif
	if(tmp == NULL)	{
		_readl_error = 3;
		free(buf);
	}
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

		/* Grow if we need to */
		if(buf[len - 1] != '\n' && !feof(fp))	{
			bufsize *= 2;
			if((buf = resize(buf, bufsize)) == NULL)
				return NULL;
			offset = len;
			n_nogrow = 0;
			continue;
		} else {
			offset = 0;
			/* Shrink if we should and we fit */
			if( (bufsize / 2 > _readl_shrink_thres) &&
			    (n_nogrow > _readl_skip_shrink) &&
			    (len < bufsize / 2)
			  )	{

				bufsize /= 2;
				if((buf = resize(buf, bufsize)) == NULL)
					return NULL;
				n_nogrow = 0;
			}

			n_nogrow++;
			if(_readl_strip)	{
				buf[len - 1] = '\0';
				*slen = len - 1;
			} else {
				*slen = len;
			}
			return buf;
		}
	}
	if(!feof(fp))
		_readl_error = 2;
	free(buf);
	return NULL;
}


char *readline(const char *fname, size_t *slen)
{
	static FILE *fp = NULL;
	char *storage;

	if(fp == NULL)	{
		if((fp = fopen(fname, "r")) == NULL)	{
			_readl_error = 1;
			return NULL;
		}
	}
	storage = readline_fp(fp, slen);
	if(storage == NULL)
		fclose(fp);
	return storage;
}
