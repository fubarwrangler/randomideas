#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#include "iniread.h"

/* Get number of contigous characters at the end of string all in @accept */
static int get_nend(const char *str, char *accept)
{
	int n = 0;

	while(*str != NULL)	{
		if(strchr(accept, *str++) != NULL)
			n++;
		else
			n = 0;
	}
	return n;
}

/* Strip leading whitespace, return 0 for comments or blank, 1 otherwise */
static int filter_line(char *raw, size_t len, int *removed)
{
	size_t l_white = 0;

	if(len > 1)	{
		/* How many whitespace chars start the string? */
		l_white = strspn(raw, "\t ");

		/* Move the non-whitespace part to the begenning of the string */
		if(l_white > 0)
			memmove(raw, (raw + l_white), len - l_white);
	}
	*removed = (int)l_white;
	/* Skip comments and blank lines */
	return (*raw == '#' || *raw == ';' || len < l_white + 2) ? 0 : 1;
}


/* If the line is a valid "[section]", modify *str by reterminating as
 * needed by stripping out any whitespace between the square brackets,
 * and return a pointer to the \0 terminated section name (in a now-
 * modified *str). If not return NULL and don't touch *str
 */
static char *get_section(char *str)
{
	char *p = str;
	size_t len = strlen(str);

	/* Must start w/ [, end w/ ], and have something between */
	if(*p == '[' && len > 2 && *(p + len - 1) == ']')	{
		size_t start, stop;
		p++;

		/* start is index(non-white), stop is index(last non-white) */
		start = strspn(p, " \t") + 1;
		stop = start + strcspn((p + start), "]");

		*(p + stop) = '\0';
		p += start - 1;

		/* If p is at ']' now, we didn't get any non-whitespace chars */
		return p;
	}
	return NULL;
}

/* Populate *key / *value with the copies of the right sections from the
 * line given in *str.  Return 0 on success, 1 otherwise.
 */
static int get_key_value(char *str, char **key, char **value)
{
	char *p = str;
	size_t k_len, v_len;
	
	*key = *value = NULL;
	
	/* First word (up till whitespace or seperator) is the key */
	k_len = strcspn(p, "\t =:");
	if(k_len < 1)
		return 1;

	p += k_len + strspn(p + k_len, "\t ");	/* Possible white after key */


	if(*p != '=' && *p != ':')	/* The separator better be next */
		return 1;

	p += 1 + strspn(p + 1, "\t ");	/* Skip whitespace after seperator */
	v_len = strlen(p);
	if(v_len < 1)
		return 1;

	if((*key = malloc(k_len + 1)) != NULL)	{
		memmove(*key, str, k_len);
		*(*key + k_len) = '\0';
		if((*value = malloc(v_len + 1)) != NULL)	{
			memmove(*value, p, v_len);
			*(*value + v_len) = '\0';
		} else {
			free(*key);
			*key = NULL;
			return 1;
		}
	} else {
		return 1;
	}

	return 0;
}


/* Return a pointer into *str that contins just the value: from after
 * the first word and the first occurance of '=' or ':' till the end.
 */
static char *get_val_from_string(char *str, char *key)
{
	char *p = NULL;
	size_t klen = strlen(key);

	if(strncmp(key, str, klen) == 0)	{

		p = str + klen;
		p += strspn(p, " \t");

		/* Next had better be the seperator, and we advance p */
		if(*p != '=' && *p != ':')
			return NULL;
        p++;
		p += strspn(p, " \t");
		return p;
	}
	return NULL;
}


static char *ini_readline(FILE *fp, int *err)
{
	char line_buf[INIREAD_LINEBUF];
	char *real_line = NULL;
	size_t buflen = 0;
	int n_endslash = 0, adj = 0;

	assert(fp != NULL);

	/* Read in one line */
	if(fgets(line_buf, INIREAD_LINEBUF, fp) == NULL)
		return NULL;

	/* Strip leading whitespace, and check for blanks/comments and skip them
	 * (tail rec shouldn't grow stack).  @adj is length of leading whitespace.
	 */
	buflen = strlen(line_buf);
	if(filter_line(line_buf, buflen, &adj) == 0)
		return ini_readline(fp, err);

	/* Check for trailing slash */
	n_endslash = get_nend(line_buf, "\\\n") - 1;

	if((real_line = malloc(buflen + 4)) == NULL)	{
		fputs("Error: malloc() failed\n", stderr);
		*err = INI_NOMEM;
		return NULL;
	}

	/* Reterminate line if trailing backslashes are present */
	line_buf[buflen - (n_endslash / 2) - 1] = '\n';
	line_buf[buflen - (n_endslash / 2)] = '\0';

	memmove(real_line, line_buf, buflen - (n_endslash / 2) + 1);

	/* If number of trailing slashes is odd, it is a line continuation */
	if(n_endslash > 0 && n_endslash % 2 != 0)	{
		size_t cumlen = buflen - (n_endslash / 2) - 2 - adj;

		/* Read subsequent lines into an expanding buffer stopping when we
		 * encounter a line ending in 0 or an even number of backslashes
		 */
		while(fgets(line_buf, INIREAD_LINEBUF, fp) != NULL)	{
			char *new_ptr;
			buflen = strlen(line_buf);

			/* Count trailing slashes */
			n_endslash = get_nend(line_buf, "\\\n") - 1;

			new_ptr = realloc(real_line, cumlen + buflen + 4);
			if(new_ptr == NULL) {
				fputs("Error: realloc() failed\n", stderr);
				*err = INI_NOMEM;
				free(real_line);
				return NULL;
			}
			real_line = new_ptr;

			memmove(real_line + cumlen, line_buf, buflen);

			/* Don't count backslash and newline */
			cumlen += buflen - (n_endslash / 2) - 2;

			/* If we end with \\, treat it as escaped */
			if(n_endslash % 2 == 0)	{
				real_line[cumlen + 1] = '\0';
				break;
			}
		}
		buflen = cumlen;
	}

	*(real_line + strlen(real_line) - get_nend(real_line, " \t\n")) = '\0';

	return real_line;
}

/* Public function: takes a filename, a section, and a key, and searches
 * for the value associated with that key in that section of the .ini-
 * formatted file.  See header for more.
 */
char *ini_read_value(char *fname, char *section, char *key, int *e)
{
	FILE *fp = NULL;
	char *line_buf = NULL, *value = NULL;
	char *p, *sec;

	*e = INI_IOERROR;

	if((fp = fopen(fname, "r")) != NULL)	{
		int in_section = 0;

		/* file opened, assume no section */
		*e = INI_NOSECTION;
		while(1)	{
			/* Second+ time around we free line_buf from prev time */
			if(line_buf != NULL)
				free(line_buf);

			/* Read a line (combining ones that end in back-slash) into
			 * heap storage -- MUST FREE
			 */
			if((line_buf = ini_readline(fp, e)) == NULL)
				break;
			p = line_buf;

			/* Did we find a [section]-defining line? */
			if((sec = get_section(p)) != NULL)	{
				/* If we already entered a section, we're now leaving it
				 * without finding a matching key, so we're done
				 */
				if(in_section != 0)
					break;

				if(strcmp(section, sec) == 0)	{
					/* section found, assume no key */
					*e = INI_NOKEY;
					in_section = 1;
				}
			}
			if(in_section != 0)	{
				if((p = get_val_from_string(p, key)) != NULL)	{
					/* found it */
					value = strdup(p);
					if(value == NULL)
						*e = INI_NOMEM;
					//else
						//puts(*value);
					*e = INI_FOUND;
					break;
				}
			}
		}
		fclose(fp);
		if(line_buf != NULL)
			free(line_buf);
	} else	{
		fprintf(stderr, "Error: cannot read config file: %s\n", fname);
	}
	return value;
}



int read_inifile(FILE *fp, struct ini_file *inidata)
{
	int err, first = 1;
	char *line, *p;
	struct ini_section *sec = inidata->first, *sp;
	struct kv_pair *kvp;

	while((line=ini_readline(fp, &err)) != NULL)	{
		if((p = get_section(line)) != NULL)	{
			printf("New section %s\n", p);
			if((sp = malloc(sizeof(struct ini_section))) != NULL)	{
				sp->name = strdup(p);
				sp->items = NULL;
				sp->next = NULL;
				if(first)	{
					inidata->first = sp;
					first = 0;
				} else	{
					sec->next = sp;
				}
				sec = sp;
			} else {
				fputs("Error allocating memory", stderr);
				return 1;
			}
		} else {

			//kvp = malloc(sizeof(struct kv_pair));

			//if(get_key_value(line, &kvp->key, &kvp->value) == 0)	{

			//}

		}
		free(line);
	}
}


#define INITESTS
#ifdef INITESTS

int main(int argc, char *argv[])
{
	char *v = NULL, *p;
	char *key, *val = NULL;
	struct ini_file ini;
	struct ini_section *sp;
	
	FILE *fp;

	fp = fopen(argv[1], "r");
	read_inifile(fp, &ini);
	fclose(fp);
	sp = ini.first;
	while(sp != NULL)	{
		struct kv_pair *k;
		puts(sp->name);
		k = sp->items;
		while(k != NULL)	{
			printf("Key: %s|Value: %s\n", k->key, k->value);
			k = k->next;
		}
		sp = sp->next;
	}

	
	return 0;
}
#endif