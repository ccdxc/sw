/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#ifndef __MISC_H__
#define __MISC_H__

#ifdef __cplusplus
extern "C" {
#if 0
} /* close to calm emacs autoindent */
#endif
#endif

char *hex_format(char *buf, size_t buflen,
                 const void *data, const size_t datalen);
char *strncpy0(char *d, const char *s, const size_t n);
int strtoargv(char *s, char *argv[], const int maxac);

char *human_readable(u_int64_t n);
char * human_readable_buf(char *buf, size_t bufsz, u_int64_t n);

void *mapfile(const char *file, const size_t sz, const int oflags);
void *mapfile_region(const char *file, 
                     const off_t offset, 
                     const size_t sz,
                     const int oflags);
void unmapfile(void *va, const size_t sz);

#ifdef __cplusplus
}
#endif

#endif /* __MISC_H__ */
