#ifndef _READLINE_H
#define _READLINE_H


extern int _readl_init_buf;
extern int _readl_shrink_thres;
extern int _readl_skip_shrink;
extern char _readl_error;
extern char _readl_strip;


#define readline_set_init_size(n) _readl_init_buf = n
#define readline_set_shrink_thresh(n) _readl_shrink_thres = n
#define readline_set_n_skip_shrink(n) _readl_skip_shrink = n
#define readline_set_strip() _readl_strip = 1
#define readline_unset_strip() _readl_strip = 0
#define readline_error() (_readl_error != 0)

char *readline_fp(FILE *fp, size_t *slen);
char *readline(const char *fname, int *slen);


#endif
