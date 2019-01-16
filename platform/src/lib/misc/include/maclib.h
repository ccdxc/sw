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

#include <stdint.h>

typedef uint64_t mac_t;

char *mac_to_str(const mac_t *m, char *str, const uint64_t strsz);
int mac_from_str(mac_t *m, const char *str);

static inline int is_zero_mac_addr(const mac_t *addr)
{
    return (*addr == 0);
}

static inline int is_multicast_mac_addr(const mac_t *addr)
{
    return (0x01 & *(const uint8_t *)addr);
}

static inline int is_broadcast_mac_addr(const mac_t *addr)
{
    return (*addr == 0xffffffffffffULL);
}

static inline int is_unicast_mac_addr(const mac_t *addr)
{
    return !is_multicast_mac_addr(addr) && !is_zero_mac_addr(addr);
}

#ifdef __cplusplus
}
#endif

#endif /* __MACLIB_H__ */
