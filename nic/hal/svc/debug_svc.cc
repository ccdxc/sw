// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

//------------------------------------------------------------------------------
// HAL DEBUG service implementation
//------------------------------------------------------------------------------

#include "nic/include/base.h"
#include "nic/include/trace.hpp"
#include "nic/hal/svc/debug_svc.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/src/debug.hpp"
#include "nic/include/asic_pd.hpp"

extern uint32_t read_reg_base(uint32_t chip, uint64_t addr);
extern void write_reg_base(uint32_t chip, uint64_t addr, uint32_t data);

Status
DebugServiceImpl::DebugInvoke(ServerContext *context,
                              const DebugRequestMsg *req,
                              DebugResponseMsg *rsp)
{
    HAL_TRACE_DEBUG("Rcvd DEBUG Invoke Request");
    int table_access = 0;
    int reg_access = 0;
    uint32_t data = 0x0;
    hal_ret_t ret = HAL_RET_OK;

    DebugSpec spec = req->request(0);
    DebugResponse *response = rsp->add_response();

    debug::DebugKeyHandle key_handle = spec.key_or_handle();

    if (key_handle.key_or_handle_case() ==
                debug::DebugKeyHandle::kTableId) {
        table_access = 1;
        HAL_TRACE_DEBUG("{}: Table: {}",
                        __FUNCTION__, key_handle.table_id());
    }

    if (key_handle.key_or_handle_case() ==
                debug::DebugKeyHandle::kRegId) {
        reg_access = 1;
        HAL_TRACE_DEBUG("{}: Reg: {}",
                        __FUNCTION__, key_handle.reg_id());
    }

    HAL_TRACE_DEBUG("{}: operation: {}",
                    __FUNCTION__, spec.opn_type(), spec.index());

    if (table_access) {
        HAL_TRACE_DEBUG("{}: operation: {} index: {}",
                        __FUNCTION__, spec.opn_type(), spec.index());

        if (spec.opn_type() == debug::DEBUG_OP_TYPE_READ) {
            ret = hal::pd::pd_debug_cli_read(
                                    key_handle.table_id(),
                                    spec.index(),
                                    (void *) spec.swkey().c_str(),
                                    (void *) spec.swkey_mask().c_str(),
                                    (void *) spec.actiondata().c_str());
            if (ret != HAL_RET_OK) {
                HAL_TRACE_DEBUG("{}: Hardware read fail", __FUNCTION__);
                return Status::OK;
            }

            DebugSpec *rsp_spec = response->mutable_rsp_data();
            rsp_spec->set_swkey(spec.swkey());
            rsp_spec->set_swkey_mask(spec.swkey_mask());
            rsp_spec->set_actiondata(spec.actiondata());
        } else {
            ret = hal::pd::pd_debug_cli_write(
                                    key_handle.table_id(),
                                    spec.index(),
                                    (void *) spec.swkey().c_str(),
                                    (void *) spec.swkey_mask().c_str(),
                                    (void *) spec.actiondata().c_str());
            if (ret != HAL_RET_OK) {
                HAL_TRACE_DEBUG("{}: Hardware write fail", __FUNCTION__);
                return Status::OK;
            }
        }
    }

    if (reg_access) {
        HAL_TRACE_DEBUG("{}: Address: 0x{0:x}", __FUNCTION__, spec.addr());

        if (spec.opn_type() == debug::DEBUG_OP_TYPE_READ) {
            data = hal::pd::asic_reg_read(spec.addr());

            HAL_TRACE_DEBUG("{}: Read Data: 0x{0:x}", __FUNCTION__, data);

            response->set_ret_data(data);
        } else {
            HAL_TRACE_DEBUG("{}: Writing Data: 0x{0:x}",
                            __FUNCTION__, spec.reg_data());
            hal::pd::asic_reg_write(spec.addr(), spec.reg_data());
        }

        response->set_debug_status(types::API_STATUS_OK);
    }

    response->set_debug_status(types::API_STATUS_OK);

    return Status::OK;
}

Status DebugServiceImpl::MemTrackGet(ServerContext *context,
                                     const MemTrackGetRequestMsg *req,
                                     MemTrackGetResponseMsg *rsp)
{
    uint32_t               i, nreqs = req->request_size();

    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    for (i = 0; i < nreqs; i++) {
        auto request = req->request(i);
        hal::mtrack_get(request, rsp);
    }
    return Status::OK;
}

Status DebugServiceImpl::SlabGet(ServerContext *context,
                                 const SlabGetRequestMsg *req,
                                 SlabGetResponseMsg *rsp)
{
    uint32_t    i, nreqs = req->request_size();

    HAL_TRACE_DEBUG("Rcvd Slab Get Request");

    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    for (i = 0; i < nreqs; i++) {
        auto request = req->request(i);
        hal::slab_get_from_req(request, rsp);
    }

    return Status::OK;
}
