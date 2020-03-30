// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __CAPRI_QUIESCE_H__
#define __CAPRI_QUIESCE_H__

#include "include/sdk/base.hpp"

namespace sdk {
namespace platform {
namespace capri {

sdk_ret_t capri_quiesce_start(void);
sdk_ret_t capri_quiesce_stop(void);
sdk_ret_t capri_quiesce_init(void);

} // namespace capri
} // namespace platform
} // namespace sdk

#endif  /* __CAPRI_QUIESCE_H__ */
