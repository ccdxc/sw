//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Initialize Stubs that drive the dataplane
//---------------------------------------------------------------

#ifndef __PDS_MS_HAL_INIT_HPP__
#define __PDS_MS_HAL_INIT_HPP__

#include <cstdint>
#include "nic/sdk/include/sdk/base.hpp"

namespace pds_ms {

bool hal_init (void);
void hal_deinit (void);
void hal_callback (sdk_ret_t status, const void *cookie);

}
#endif
