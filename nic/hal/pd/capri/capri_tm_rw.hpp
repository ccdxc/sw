/*
 * capri_tm_rw.hpp
 * Vasanth Kumar (Pensando Systems)
 */

#ifndef __CAPRI_TM_RW_HPP__
#define __CAPRI_TM_RW_HPP__

#include <stdio.h>
#include <string>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <base.h>

typedef struct tm_pg_params_s {
    uint32_t reserved_min;
    uint32_t xon_threshold;
    uint32_t headroom;
    uint32_t low_limit;
    uint32_t alpha;
    uint32_t mtu;
} tm_pg_params_t;

typedef struct tm_pg_cos_map_s {
    uint32_t cos;
    uint32_t pg;
} tm_pg_cos_map_t;

typedef struct tm_scheduler_map_s {
    uint32_t priority;
} tm_scheduler_map_t;

hal_ret_t capri_tm_pg_params_update(uint32_t port,
                                    uint32_t pg,
                                    tm_pg_params_t *pg_params);

hal_ret_t capri_tm_pg_cos_map_update(uint32_t port,
                                     uint32_t pg,
                                     tm_pg_cos_map_t *pg_cos_map);

hal_ret_t capri_tm_scheduler_map_update(uint32_t port,
                                        tm_scheduler_map_t *scheduler_map);

/* Program the lif value on an uplink port */
hal_ret_t capri_tm_uplink_lif_set(uint32_t port,
                                  uint32_t lif);
#endif
