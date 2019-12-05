//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Initialize Stubs that drive the dataplane
//---------------------------------------------------------------

#ifndef __PDSA_HAL_INIT_HPP__
#define __PDSA_HAL_INIT_HPP__

#include <cstdint>

namespace pdsa_stub {

bool hal_init (void);
void hal_deinit (void);
void hal_callback (bool status, uint64_t cookie);

}
#endif
