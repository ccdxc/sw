// {C} Copyright 2020 Pensando Systems Inc. All rights reserved

#ifndef __ELBA_QUIESCE_HPP__
#define __ELBA_QUIESCE_HPP__

#include "include/sdk/base.hpp"

namespace sdk {
namespace platform {
namespace elba {

sdk_ret_t elba_quiesce_start(void);
sdk_ret_t elba_quiesce_stop(void);
sdk_ret_t elba_quiesce_init(void);

}    // namespace elba
}    // namespace platform
}    // namespace sdk

#endif    // __ELBA_QUIESCE_HPP__
