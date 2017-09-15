#ifndef __SYSTEM_HPP__
#define __SYSTEM_HPP__

#include <base.h>
#include <system.pb.h>

using sys::SystemResponse;

namespace hal {

hal_ret_t system_get(SystemResponse *rsp);

}    // namespace hal

#endif    // __SYSTEM_HPP__

