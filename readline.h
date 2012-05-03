#ifndef _READLINE_H
#define _READLINE_H


int init_bufsize = 78;
int shrink_thresh = 73;
int nogrow_thresh = 4;

char *readline_fp(FILE *fp, size_t *slen, char strip);

#endif
