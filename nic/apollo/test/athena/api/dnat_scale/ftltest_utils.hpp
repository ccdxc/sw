//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#ifndef __UTILS_HPP__
#define __UTILS_HPP__

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <cinttypes>
#include "nic/apollo/api/include/athena/pds_dnat.h"

void fill_key(uint32_t index, pds_dnat_mapping_key_t *key);

void fill_data(uint32_t index, pds_dnat_mapping_data_t *data);

void update_data(uint32_t index, pds_dnat_mapping_data_t *data);

#endif // __UTILS_HPP__
