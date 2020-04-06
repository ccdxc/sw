//------------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//------------------------------------------------------------------------------

#ifndef __UTILS_HPP__
#define __UTILS_HPP__

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <cinttypes>
#include "nic/apollo/api/include/athena/pds_flow_session_rewrite.h"

void fill_data(pds_flow_session_rewrite_data_t *data, uint32_t index,
               pds_flow_session_rewrite_nat_type_t nat_type,
               pds_flow_session_encap_t encap_type);
void fill_data_type(pds_flow_session_rewrite_data_t *data,
                    uint32_t index);
void update_data_type(pds_flow_session_rewrite_data_t *data,
                      uint32_t index);
void fill_key(pds_flow_session_rewrite_key_t *key, uint32_t index);

#endif // __UTILS_HPP__
