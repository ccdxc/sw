//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __INTERNAL_HPP__
#define __INTERNAL_HPP__

#include "nic/include/base.h"
#include "nic/include/eth.h"
#include "nic/include/ip.h"
#include "sdk/list.hpp"
#include "sdk/ht.hpp"
#include "nic/include/bitmap.hpp"
#include "nic/gen/proto/hal/internal.pb.h"
#include "nic/hal/src/internal/internal.hpp"

namespace hal {

void getprogram_address(const internal::ProgramAddressReq& req,
                        internal::ProgramAddressResponseMsg *rsp);

void allochbm_address(const internal::HbmAddressReq &req,
                      internal::HbmAddressResp *resp);

void configurelif_bdf(const internal::LifBdfReq &req,
                      internal::LifBdfResp *resp);

hal_ret_t software_phv_get (internal::SoftwarePhvGetRequest &req, internal::SoftwarePhvGetResponseMsg *rsp);
hal_ret_t software_phv_inject (internal::SoftwarePhvInject &req, internal::SoftwarePhvResponse *rsp);

}    // namespace hal

#endif    // __INTERNAL_HPP__

