#ifndef _READLINE_H
#define _READLINE_H


extern int init_bufsize;
extern int shrink_thresh;
extern int nogrow_thresh;


#define readline_set_init_size(n) init_bufsize = n
#define readline_set_shrink_thresh(n) shrink_thresh = n
#define readline_set_nogrow_thresh(n) nogrow_thresh = n

char *readline_fp(FILE *fp, size_t *slen, char strip);

#endif
