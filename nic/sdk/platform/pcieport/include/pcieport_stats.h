/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#ifndef __PCIEPORT_STATS_H__
#define __PCIEPORT_STATS_H__

#ifdef __cplusplus
extern "C" {
#if 0
} /* close to calm emacs autoindent */
#endif
#endif

typedef union pcieport_stats {
    struct {

#define PCIEPORT_STATS_DEF(S) \
        uint64_t S;
#include "pcieport_stats_defs.h"

    };
    /* pad to 64 entries, room to grow */
    uint64_t _pad[64];

} pcieport_stats_t;

#ifdef __cplusplus
}
#endif

#endif /* __PCIEPORT_STATS_H__ */
