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
#include <sys/time.h>

#include <rte_spinlock.h>
#include <rte_atomic.h>
#include <rte_malloc.h>

#include "nic/apollo/api/include/athena/pds_flow_age.h"

#ifndef BITS_PER_BYTE
#define BITS_PER_BYTE           8
#endif

namespace ftl_pollers_client {

sdk_ret_t init(void);
uint32_t qcount_get(void);
sdk_ret_t poll(uint32_t qid,
               expiry_user_cb_t expiry_user_cb = nullptr,
               bool bringup_log = true);

} // namespace ftl_pollers_client

#endif // __FTL_POLLERS_CLIENT_HPP__
