// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
/*
 * elba_mon.hpp
 */

#ifndef __ELBA_MON_HPP__
#define __ELBA_MON_HPP__

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include "include/sdk/base.hpp"
#include "asic/rw/asicrw.hpp"

namespace sdk {
namespace platform {
namespace elba {

typedef enum pen_adjust_perf_status {
    PEN_PERF_SUCCESS = 0,
    PEN_PERF_FAILED = 1,
    PEN_PERF_INVALID = 2,
} pen_adjust_perf_status_t;

typedef enum pen_adjust_perf_index {
    PEN_PERF_ID0 = 0,
    PEN_PERF_ID1 = 1,
    PEN_PERF_ID2 = 2,
    PEN_PERF_ID3 = 3,
    PEN_PERF_ID4 = 4,
} pen_adjust_index_t;

typedef enum pen_adjust_perf_type {
    PEN_PERF_UP = 0,
    PEN_PERF_DOWN = 1,
    PEN_PERF_SET = 2,
} pen_adjust_perf_type_t;

typedef struct hbmerrcause_s {
    uint32_t offset;
    std::string message;
} hbmerrcause_t;

#define MAX_CHANNEL   8
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a)       (sizeof (a) / sizeof ((a)[0]))
#endif

pen_adjust_perf_status_t elba_adjust_perf(int chip_id, int inst_id,
                                          pen_adjust_index_t &idx,
                                          pen_adjust_perf_type_t perf_type);
void elba_set_half_clock(int chip_id, int inst_id);
sdk_ret_t elba_unravel_hbm_intrs(bool *iscattrip, bool *iseccerr,
                                  bool logging);

} // namespace elba
} // namespace platform
} // namespace sdk

#endif
