/*
 * Copyright (c) 2020, Pensando Systems Inc.
 */

#ifndef __SERDES_H__
#define __SERDES_H__

#ifdef __cplusplus
extern "C" {
#if 0
} /* close to calm emacs autoindent */
#endif
#endif

typedef union laneinfo_u {
    uint16_t lane[16];
    uint32_t w[8];
} laneinfo_t;

uint16_t pciesd_lanes_ready(const uint16_t lanemask);
void pciesd_core_interrupt(const uint16_t lanemask,
                           const uint16_t code,
                           const uint16_t data,
                           laneinfo_t *dataout);

#ifdef __cplusplus
}
#endif

#endif /* __SERDES_H__ */
