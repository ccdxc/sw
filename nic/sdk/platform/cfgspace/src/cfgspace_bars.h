/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#ifndef __CFGSPACE_BARS_H__
#define __CFGSPACE_BARS_H__

void cfgspace_set_bars(cfgspace_t *cs,
                       const cfgspace_bar_t *bars,
                       const int nbars);

void cfgspace_set_rombar(cfgspace_t *cs, const cfgspace_bar_t *rombar);

void cfgspace_set_sriov_bars(cfgspace_t *cs,
                             const u_int16_t cfgbase,
                             const cfgspace_bar_t *bars,
                             const int nbars);

#endif /* __CFGSPACE_BARS_H__ */
