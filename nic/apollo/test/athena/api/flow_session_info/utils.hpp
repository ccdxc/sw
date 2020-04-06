//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#ifndef __UTILS_HPP__
#define __UTILS_HPP__

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <cinttypes>
#include "nic/apollo/api/include/athena/pds_flow_session_info.h"

#define CREATE    (0)
#define UPDATE    (1)

void fill_key(pds_flow_session_key_t *key, uint32_t index,
              uint8_t direction);

void fill_data(pds_flow_session_data_t *data, uint32_t index,
               uint8_t direction, bool update);

void fill_scale_data(pds_flow_session_data_t *data, uint32_t index);

void update_scale_data(pds_flow_session_data_t *data, uint32_t index);

#endif // __UTILS_HPP__
