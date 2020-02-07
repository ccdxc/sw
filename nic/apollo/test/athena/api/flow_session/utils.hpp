//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#ifndef __UTILS_HPP__
#define __UTILS_HPP__

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <cinttypes>
#include "nic/apollo/api/include/athena/pds_flow_session.h"

#define CREATE            (0)
#define COMMON_UPDATE     (1)
#define REWRITE_UPDATE    (2)
#define ALL_UPDATE        (3)

void fill_key(pds_flow_session_key_t *key, uint32_t index, uint8_t direction);

void fill_data(pds_flow_session_data_t *data, uint32_t index,
               uint8_t direction, uint8_t update_mode);

#endif // __UTILS_HPP__
