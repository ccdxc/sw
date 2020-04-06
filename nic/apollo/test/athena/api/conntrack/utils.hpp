//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#ifndef __UTILS_HPP__
#define __UTILS_HPP__

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <cinttypes>
#include "nic/apollo/api/include/athena/pds_conntrack.h"

#define MHTEST_CHECK_RETURN(_exp, _ret) if (!(_exp)) return (_ret)

void fill_key(pds_conntrack_key_t *key, uint32_t index);

void fill_data(pds_conntrack_data_t *data, uint32_t index);

void update_data(pds_conntrack_data_t *data, uint32_t index);

#endif // __UTILS_HPP__
