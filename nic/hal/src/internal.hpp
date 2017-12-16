#ifndef __INTERNAL_HPP__
#define __INTERNAL_HPP__

#include "nic/include/base.h"
#include "nic/include/eth.h"
#include "nic/include/ip.h"
#include "nic/include/list.hpp"
#include "sdk/ht.hpp"
#include "nic/include/bitmap.hpp"
#include "nic/gen/proto/hal/internal.pb.h"
#include "nic/hal/src/internal.hpp"

namespace hal {

void GetProgramAddress(const internal::ProgramAddressReq& req,
                       internal::ProgramAddressResp *resp);

void AllocHbmAddress(const internal::HbmAddressReq &req,
                       internal::HbmAddressResp *resp);

void ConfigureLifBdf(const internal::LifBdfReq &req,
                     internal::LifBdfResp *resp);

}    // namespace hal

#endif    // __INTERNAL_HPP__

