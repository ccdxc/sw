//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __INTERNAL_HPP__
#define __INTERNAL_HPP__

#include "nic/include/base.hpp"
#include "nic/include/eth.hpp"
#include "nic/include/ip.hpp"
#include "sdk/list.hpp"
#include "sdk/ht.hpp"
#include "nic/include/bitmap.hpp"
#include "nic/gen/proto/hal/internal.pb.h"
#include "nic/hal/src/internal/internal.hpp"
#include "nic/gen/proto/hal/fwlog.pb.h"

namespace hal {

void getprogram_address(const internal::ProgramAddressReq& req,
                        internal::ProgramAddressResponseMsg *rsp);

void allochbm_address(const internal::HbmAddressReq &req,
                      internal::HbmAddressResp *resp);

void configurelif_bdf(const internal::LifBdfReq &req,
                      internal::LifBdfResp *resp);

hal_ret_t software_phv_get (internal::SoftwarePhvGetRequest &req, internal::SoftwarePhvGetResponseMsg *rsp);
hal_ret_t software_phv_inject (internal::SoftwarePhvInject &req, internal::SoftwarePhvResponse *rsp);
hal_ret_t log_flow (fwlog::FWEvent &req, internal::LogFlowResponse *rsp); 

}    // namespace hal

#endif    // __INTERNAL_HPP__

