//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#ifndef __INTERNAL_HPP__
#define __INTERNAL_HPP__

#include "nic/include/base.hpp"
#include "nic/sdk/include/sdk/eth.hpp"
#include "nic/sdk/include/sdk/ip.hpp"
#include "nic/sdk/include/sdk/list.hpp"
#include "nic/sdk/include/sdk/ht.hpp"
#include "nic/include/bitmap.hpp"
#include "gen/proto/internal.pb.h"
#include "nic/hal/src/internal/internal.hpp"
#include "gen/proto/fwlog.pb.h"
#include "gen/proto/types.pb.h"

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
hal_ret_t quiesce_msg_snd(const types::Empty &request, types::Empty* rsp);
hal_ret_t quiesce_start(const types::Empty &request, types::Empty* rsp);
hal_ret_t quiesce_stop(const types::Empty &request, types::Empty* rsp);
hal_ret_t ipseccb_create(internal::IpsecCbSpec& spec,
                       internal::IpsecCbResponse *rsp);

hal_ret_t ipseccb_update(internal::IpsecCbSpec& spec,
                       internal::IpsecCbResponse *rsp);

hal_ret_t ipseccb_delete(internal::IpsecCbDeleteRequest& req,
                       internal::IpsecCbDeleteResponseMsg *rsp);

hal_ret_t ipseccb_get(internal::IpsecCbGetRequest& req,
                    internal::IpsecCbGetResponseMsg *rsp);

}    // namespace hal

#endif    // __INTERNAL_HPP__

