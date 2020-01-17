/*
 * Copyright (c) 2017-2018, Pensando Systems Inc.
 */

#ifndef __MISC_H__
#define __MISC_H__

#ifdef __cplusplus
extern "C" {
#if 0
} /* close to calm emacs autoindent */
#endif
#endif

/*
 * roundup_power2 - Round up to next power of 2.
 */
static inline u_int64_t
roundup_power2(u_int64_t n)
{
    while (n & (n - 1)) {
        n = (n | (n - 1)) + 1;
    }
    return n;
}

char *hex_format(char *buf, size_t buflen,
                 const void *data, const size_t datalen);
char *strncpy0(char *d, const char *s, const size_t n);
int strtoargv(char *s, char *argv[], const int maxac);
unsigned long long strtoull_ext(const char *s);

char *human_readable(u_int64_t n);
char * human_readable_buf(char *buf, size_t bufsz, u_int64_t n);

void *mapfile(const char *file, const size_t sz, const int oflags);
void *mapfile_region(const char *file,
                     const off_t offset,
                     const size_t sz,
                     const int oflags);
void unmapfile(void *va, const size_t sz);

int netif_up(const char *ifname);
int netif_down(const char *ifname);
int netif_setip(const char *ifname, const u_int32_t ip, const u_int32_t nm);

#ifdef __cplusplus
}
#endif

#endif /* __MISC_H__ */
