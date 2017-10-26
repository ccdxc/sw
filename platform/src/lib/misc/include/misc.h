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
                 const unsigned char *data, const size_t datalen);
char *strncpy0(char *d, const char *s, const size_t n);
int strtoargv(char *s, char *argv[], const int maxac);

#ifdef __cplusplus
}
#endif

#endif /* __MISC_H__ */
