//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/include/base.hpp"
#include "nic/hal/hal.hpp"
#include "nic/hal/iris/include/hal_state.hpp"
#include "gen/proto/interface.grpc.pb.h"
#include "nic/hal/src/internal/internal.hpp"
#include "nic/include/pd.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/src/utils/utils.hpp"
#include "nic/hal/src/utils/if_utils.hpp"
#include "nic/hal/plugins/proxy/proxy_plugin.hpp"
#include "nic/hal/plugins/cfg/nw/session.hpp"

using intf::Interface;
using intf::LifSpec;
using kh::LifKeyHandle;
using intf::LifRequestMsg;
using intf::LifResponse;
using intf::LifResponseMsg;
using intf::LifDeleteRequestMsg;
using intf::LifDeleteResponseMsg;
using intf::LifGetRequestMsg;
using intf::LifGetResponseMsg;
using intf::InterfaceSpec;
using intf::InterfaceStatus;
using intf::InterfaceResponse;
using kh::InterfaceKeyHandle;
using intf::InterfaceRequestMsg;
using intf::InterfaceResponseMsg;
using intf::InterfaceDeleteRequestMsg;
using intf::InterfaceDeleteResponseMsg;
using intf::InterfaceGetRequest;
using intf::InterfaceGetRequestMsg;
using intf::InterfaceGetResponse;
using intf::InterfaceGetResponseMsg;
using intf::InterfaceL2SegmentRequestMsg;
using intf::InterfaceL2SegmentSpec;
using intf::InterfaceL2SegmentResponseMsg;
using intf::InterfaceL2SegmentResponse;
using intf::GetQStateRequestMsg;
using intf::GetQStateResponseMsg;
using intf::SetQStateRequestMsg;
using intf::SetQStateResponseMsg;

namespace hal {

void getprogram_address(const internal::ProgramAddressReq &req,
                        internal::ProgramAddressResponseMsg *rsp) {
    uint64_t addr;
    internal::ProgramAddressResp *resp = rsp->add_response();
    pd::pd_func_args_t          pd_func_args = {0};
    if (req.resolve_label()) {
        pd::pd_capri_program_label_to_offset_args_t args = {0};
        args.handle = req.handle().c_str();
        args.prog_name = (char *)req.prog_name().c_str();
        args.label_name = (char *)req.label().c_str();
        args.offset = &addr;
        pd_func_args.pd_capri_program_label_to_offset = &args;
        hal_ret_t ret = pd::hal_pd_call(pd::PD_FUNC_ID_PROG_LBL_TO_OFFSET, &pd_func_args);
        if (ret != HAL_RET_OK) {
            resp->set_addr(0xFFFFFFFFFFFFFFFFULL);
        } else {
            resp->set_addr(addr);
        }
    } else {
        pd::pd_capri_program_to_base_addr_args_t base_args = {0};
        base_args.handle = req.handle().c_str();
        base_args.prog_name = (char *)req.prog_name().c_str();
        base_args.base_addr = &addr;
        pd_func_args.pd_capri_program_to_base_addr = &base_args;
        hal_ret_t ret = pd::hal_pd_call(pd::PD_FUNC_ID_PROG_TO_BASE_ADDR, &pd_func_args);
        if (ret != HAL_RET_OK) {
            resp->set_addr(0xFFFFFFFFFFFFFFFFULL);
        } else {
            resp->set_addr(addr);
        }
    }
}

void allochbm_address(const internal::HbmAddressReq &req,
                      internal::HbmAddressResp *resp) {
    mem_addr_t addr = 0ULL;
    uint32_t size_kb;
    const char *region = req.handle().c_str();
    sdk::platform::utils::mpartition *mpart = NULL;
    mpart = sdk::platform::utils::mpartition::get_instance();
    if (mpart) {
        addr = mpart->start_addr(region);
        if (addr == INVALID_MEM_ADDRESS) {
            addr = 0ULL;
        }
        size_kb = mpart->size(region) >> 10;
    }
    HAL_TRACE_DEBUG("pi-internal:{}: Allocated HBM Address {:#x} Size {}",
                    __FUNCTION__, addr, size_kb);
    if (addr == 0) {
        resp->set_addr(0xFFFFFFFFFFFFFFFFULL);
        resp->set_size(0);
    } else {
        resp->set_addr(addr);
        resp->set_size(size_kb);
    }
}

void configurelif_bdf(const internal::LifBdfReq &req,
                      internal::LifBdfResp *resp)
{
    pd::pd_capri_pxb_cfg_lif_bdf_args_t args = {0};
    pd::pd_func_args_t          pd_func_args = {0};
    args.lif = req.lif();
    args.bdf = req.bdf();
    pd_func_args.pd_capri_pxb_cfg_lif_bdf = &args;
    int ret = (int)pd::hal_pd_call(pd::PD_FUNC_ID_PXB_CFG_LIF_BDF, &pd_func_args);

    resp->set_lif(req.lif());
    resp->set_bdf(req.bdf());
    resp->set_status(ret);
}

// No HAL functionality
hal_ret_t log_flow (fwlog::FWEvent &req, internal::LogFlowResponse *rsp) {
    return HAL_RET_OK;
}

hal_ret_t quiesce_msg_snd(const types::Empty &request,
                          types::Empty* rsp) 
{
#ifdef SIM
    HAL_TRACE_DEBUG("QuiesceMsgSnd Request");
    hal::proxy::quiesce_message_tx();
#endif
    return HAL_RET_OK;
}

hal_ret_t quiesce_start(const types::Empty &request,
                        types::Empty* rsp)
{
    hal_ret_t           ret = HAL_RET_OK;
    hal::pd::pd_func_args_t args = {0};

    ret = pd::hal_pd_call(pd::PD_FUNC_ID_QUIESCE_START, &args);
    return ret;
}

hal_ret_t quiesce_stop(const types::Empty &request,
                       types::Empty* rsp)
{
    hal_ret_t           ret = HAL_RET_OK;
    hal::pd::pd_func_args_t args = {0};

    ret = pd::hal_pd_call(pd::PD_FUNC_ID_QUIESCE_STOP, &args);
    return ret;
}

hal_ret_t testsendfin_req(internal::TestSendFinRequest& req,
                          internal::TestSendFinResponse *rsp)
{
    HAL_TRACE_DEBUG("Calling session upgrade");
    return (session_handle_upgrade());
}

hal_ret_t flow_hash_get(internal::FlowHashGetRequest& req,
                        internal::FlowHashGetResponseMsg *rsp)
{
    return (session_flow_hash_get(req, rsp));
}

hal_ret_t testclocksync_req(internal::TestClockSyncRequest& req,
                          internal::TestClockSyncResponse *rsp)
{
    pd::pd_func_args_t             pd_func_args = {0};
    pd::pd_clock_trigger_sync_args_t clock_args;

    HAL_TRACE_DEBUG("Calling Clock trigger sync");
    pd_func_args.pd_clock_trigger_sync = &clock_args;
    pd::hal_pd_call(pd::PD_FUNC_ID_CLOCK_TRIGGER_SYNC, &pd_func_args);

    return HAL_RET_OK;
}

}    // namespace hal
