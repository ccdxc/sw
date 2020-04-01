//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#ifndef __UTILS_HPP__
#define __UTILS_HPP__

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <cinttypes>
#include "nic/apollo/api/include/athena/pds_flow_cache.h"

void fill_key(uint32_t index, pds_flow_key_t *key);

void fill_data(uint32_t index, pds_flow_spec_index_type_t index_type,
               pds_flow_data_t *data);

void dump_flow(pds_flow_iter_cb_arg_t *iter_cb_arg);

void dump_stats(pds_flow_stats_t *stats);

#endif // __UTILS_HPP__
