#ifndef __DEBUG_HPP__
#define __DEBUG_HPP__

#include "nic/include/base.h"
#include "nic/gen/proto/hal/debug.pb.h"

namespace hal {

hal_ret_t mtrack_get(debug::MemTrackGetRequest& req,
                     debug::MemTrackGetResponseMsg *rsp);

}    // namespace hal

#endif    // __DEBUG_HPP__

