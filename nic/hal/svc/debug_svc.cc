//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//
// HAL DEBUG service implementation
//------------------------------------------------------------------------------

#include "nic/include/base.h"
#include "nic/include/trace.hpp"
#include "nic/hal/svc/debug_svc.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/src/debug/debug.hpp"
#include "nic/include/asic_pd.hpp"

// TODO: we don't seem to be using these ??
#include <vector>
#include <tuple>
using std::vector;
using std::tuple;

extern uint32_t read_reg_base(uint32_t chip, uint64_t addr);
extern void write_reg_base(uint32_t chip, uint64_t addr, uint32_t data);

struct _reg_info {
    char *reg_name;
    int  offset;
    char *value;
};
typedef struct _reg_info reg_info;

Status
DebugServiceImpl::RegisterGet(ServerContext *context,
                              const RegisterRequestMsg *req_msg,
                              RegisterResponseMsg *rsp_msg)
{
    uint64_t val  = 0x0;

    for (int i = 0; i < req_msg->request_size(); ++i) {
        debug::RegisterRequest req = req_msg->request(i);

        debug::RegisterResponse *rsp = rsp_msg->add_response();
        debug::RegisterData     *reg_data = rsp->mutable_data();

        val  = hal::pd::asic_reg_read(req.addr());

        reg_data->set_value(std::to_string(val));
        rsp->set_api_status(types::API_STATUS_OK);
    }
    return Status::OK;
}

Status
DebugServiceImpl::RegisterUpdate(ServerContext *context,
                                 const RegisterRequestMsg *req_msg,
                                 RegisterResponseMsg *rsp_msg)
{
    uint32_t data = 0x0;

    for (int i = 0; i < req_msg->request_size(); ++i) {
        debug::RegisterRequest req = req_msg->request(i);

        data = req.reg_data();

        debug::RegisterResponse *rsp = rsp_msg->add_response();

        hal::pd::asic_reg_write(req.addr(), &data);

        rsp->set_api_status(types::API_STATUS_OK);
    }

    return Status::OK;
}

Status
DebugServiceImpl::MemoryGet(ServerContext *context,
                            const MemoryRequestMsg *req_msg,
                            MemoryResponseMsg *rsp_msg)
{
    hal_ret_t                               ret          = HAL_RET_OK;
    int                                     index        = 0;
    int                                     num_indices  = 0;
    hal::pd::pd_debug_cli_read_args_t       args;
    hal::pd::pd_table_properties_get_args_t table_prop_args;
    debug::MemoryResponse                   *rsp = NULL;

    for (int i = 0; i < req_msg->request_size(); ++i) {
        debug::MemoryRequest req = req_msg->request(i);

        if (req.index() == 0xffffffff) {
            table_prop_args.tableid = req.table_id();

            ret = hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_TABLE_PROPERTIES_GET,
                                       (void *)&table_prop_args);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_DEBUG("Failed to get table properties for"
                                " table: {}, err: {}",
                                req.table_id(), ret);
                rsp = rsp_msg->add_response();
                rsp->set_api_status(types::API_STATUS_HW_READ_ERROR);
                return Status::OK;
            }
            index       = 0;
            num_indices = table_prop_args.tabledepth;
        } else {
            index       = req.index();
            num_indices = 1;
        }

        HAL_TRACE_DEBUG("index start: {}, num indices: {}",
                        index, index + num_indices);

        for (int i = index; i < index + num_indices; ++i) {
            rsp = rsp_msg->add_response();

            memset (&args, 0, sizeof(hal::pd::pd_debug_cli_read_args_t));

            args.tableid = req.table_id();
            args.index = i;
            args.swkey = (void *)req.swkey().c_str();
            args.swkey_mask = (void *)req.swkey_mask().c_str();
            args.actiondata = (void *)req.actiondata().c_str();
            ret = hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_DEBUG_CLI_READ, (void *)&args);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_DEBUG("Hardware read failure, err: {}", ret);
                rsp->set_api_status(types::API_STATUS_HW_READ_ERROR);
                return Status::OK;
            }

            rsp->set_swkey(req.swkey());
            rsp->set_swkey_mask(req.swkey_mask());
            rsp->set_actiondata(req.actiondata());
            rsp->set_api_status(types::API_STATUS_OK);
        }
    }

    return Status::OK;
}

Status
DebugServiceImpl::MemoryUpdate(ServerContext *context,
                               const MemoryRequestMsg *req_msg,
                               MemoryResponseMsg *rsp_msg)
{
    hal_ret_t                          ret = HAL_RET_OK;
    hal::pd::pd_debug_cli_read_args_t  args;

    for (int i = 0; i < req_msg->request_size(); ++i) {
        debug::MemoryRequest req = req_msg->request(i);
        debug::MemoryResponse *rsp = rsp_msg->add_response();

        memset (&args, 0, sizeof(hal::pd::pd_debug_cli_read_args_t));

        args.tableid = req.table_id();
        args.index = req.index();
        args.swkey = (void *)req.swkey().c_str();
        args.swkey_mask = (void *)req.swkey_mask().c_str();
        args.actiondata = (void *)req.actiondata().c_str();
        ret = hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_DEBUG_CLI_WRITE, (void *)&args);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_DEBUG("Hardware write failure, err: {}", ret);
            rsp->set_api_status(types::API_STATUS_HW_READ_ERROR);
            return Status::OK;
        }
        rsp->set_api_status(types::API_STATUS_OK);
    }

    return Status::OK;
}

Status
DebugServiceImpl::MemTrackGet(ServerContext *context,
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

Status
DebugServiceImpl::SlabGet(ServerContext *context,
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

Status
DebugServiceImpl::MpuTraceUpdate(ServerContext *context,
                                 const MpuTraceRequestMsg *req,
                                 MpuTraceResponseMsg *rsp)
{
    uint32_t  i     = 0;
    uint32_t  nreqs = req->request_size();

    HAL_TRACE_DEBUG("Received MPU trace request");

    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    for (i = 0; i < nreqs; i++) {
        auto request = req->request(i);
        hal::mpu_trace_enable(request, rsp);
    }

    return Status::OK;
}

Status
DebugServiceImpl::TraceUpdate(ServerContext *context,
                              const TraceRequestMsg *req,
                              TraceResponseMsg *rsp)
{
    uint32_t         i, nreqs = req->request_size();
    TraceResponse    *response;

    HAL_TRACE_DEBUG("Received trace update");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }
    for (i = 0; i < nreqs; i++) {
        response = rsp->add_response();
        auto spec = req->request(i);
        hal::trace_update(spec, response);
    }
    return Status::OK;
}

Status
DebugServiceImpl::TraceGet(ServerContext *context,
                           const Empty *req,
                           TraceResponseMsg *rsp)
{
    HAL_TRACE_DEBUG("Received trace get");
    hal::trace_get(rsp);
    return Status::OK;
}
