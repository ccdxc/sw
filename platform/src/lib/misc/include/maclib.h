/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#ifndef __MACLIB_H__
#define __MACLIB_H__

#ifdef __cplusplus
extern "C" {
#if 0
} /* close to calm emacs autoindent */
#endif
#endif

typedef struct mac_s {
    u_int8_t buf[6];
} mac_t;

char *mac_to_str(const mac_t *m);
char *mac_to_str_r(const mac_t *m, char *buf, const size_t bufsz);

int mac_from_str(mac_t *m, const char *str);

#ifdef __cplusplus
}
#endif

#endif /* __MACLIB_H__ */
