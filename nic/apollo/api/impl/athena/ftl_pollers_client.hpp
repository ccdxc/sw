//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
///----------------------------------------------------------------------------
///
/// \file
/// Flow Table Library software pollers client
///
///----------------------------------------------------------------------------

#ifndef __FTL_POLLERS_CLIENT_HPP__
#define __FTL_POLLERS_CLIENT_HPP__

#include <cstdio>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cmath>
#include <endian.h>
#include <sstream>
#include <inttypes.h>
#include <sys/time.h>

#include "nic/apollo/api/include/athena/pds_flow_age.h"
#include "nic/include/ftl_dev_if.hpp"

#ifndef BITS_PER_BYTE
#define BITS_PER_BYTE           8
#endif

namespace ftl_pollers_client {

extern volatile uint8_t user_will_poll_;

sdk_ret_t init(void);
uint32_t qcount_get(void);
sdk_ret_t expiry_fn_dflt(pds_flow_expiry_fn_t *ret_fn_dflt);
sdk_ret_t poll_control(bool user_will_poll,
                       pds_flow_expiry_fn_t expiry_fn);
sdk_ret_t poll(uint32_t qid,
               void *user_ctx = nullptr);

sdk_ret_t force_session_expired_ts_set(bool force_expired_ts);
sdk_ret_t force_conntrack_expired_ts_set(bool force_expired_ts);
sdk_ret_t session_scanners_metrics_get(ftl_dev_if::lif_attr_metrics_t *metrics);
sdk_ret_t conntrack_scanners_metrics_get(ftl_dev_if::lif_attr_metrics_t *metrics);
sdk_ret_t pollers_metrics_get(ftl_dev_if::lif_attr_metrics_t *metrics);
uint32_t session_table_depth_get(void);
uint32_t conntrack_table_depth_get(void);
void expiry_log_set(bool enable_sense);

static inline bool
user_will_poll(void)
{
    return user_will_poll_;
}

} // namespace ftl_pollers_client

#endif // __FTL_POLLERS_CLIENT_HPP__
