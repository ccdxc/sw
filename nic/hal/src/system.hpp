#ifndef __SYSTEM_HPP__
#define __SYSTEM_HPP__

#include "nic/include/base.h"
#include "nic/proto/hal/system.pb.h"

using sys::SystemResponse;

namespace hal {

hal_ret_t system_get(SystemResponse *rsp);

}    // namespace hal

#endif    // __SYSTEM_HPP__

