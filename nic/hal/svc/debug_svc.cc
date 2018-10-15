//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//
// HAL DEBUG service implementation
//------------------------------------------------------------------------------

#include "nic/include/base.hpp"
#include "nic/include/trace.hpp"
#include "nic/hal/svc/debug_svc.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/src/debug/debug.hpp"
#include "nic/include/asic_pd.hpp"
#include "nic/linkmgr/linkmgr_debug.hpp"

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
    uint32_t val = 0x0;

    for (int i = 0; i < req_msg->request_size(); ++i) {
        debug::RegisterRequest req = req_msg->request(i);

        debug::RegisterResponse *rsp = rsp_msg->add_response();
        debug::RegisterData     *reg_data = rsp->mutable_data();
        hal::pd::asic_reg_read(req.addr(), &val, 1, true);
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
    hal::pd::pd_func_args_t                 pd_func_args = {0};
    debug::MemoryResponse                   *rsp = NULL;

    for (int i = 0; i < req_msg->request_size(); ++i) {
        debug::MemoryRequest req = req_msg->request(i);

        if (req.index() == 0xffffffff) {
            table_prop_args.tableid = req.table_id();

            pd_func_args.pd_table_properties_get = &table_prop_args;
            ret = hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_TABLE_PROPERTIES_GET, &pd_func_args);
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
            pd_func_args.pd_debug_cli_read = &args;
            ret = hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_DEBUG_CLI_READ, &pd_func_args);
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
    hal::pd::pd_func_args_t            pd_func_args = {0};

    for (int i = 0; i < req_msg->request_size(); ++i) {
        debug::MemoryRequest req = req_msg->request(i);
        debug::MemoryResponse *rsp = rsp_msg->add_response();

        memset (&args, 0, sizeof(hal::pd::pd_debug_cli_read_args_t));

        args.tableid = req.table_id();
        args.index = req.index();
        args.swkey = (void *)req.swkey().c_str();
        args.swkey_mask = (void *)req.swkey_mask().c_str();
        args.actiondata = (void *)req.actiondata().c_str();
        pd_func_args.pd_debug_cli_read = &args;
        ret = hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_DEBUG_CLI_WRITE, &pd_func_args);
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
DebugServiceImpl::MemoryRawGet(ServerContext *context,
                            const MemoryRawRequestMsg *req_msg,
                            MemoryRawResponseMsg *rsp_msg)
{
    hal_ret_t                               ret          = HAL_RET_OK;
#define RAW_MEM_READ_MAX        1024
    uint8_t                                 mem[RAW_MEM_READ_MAX];

    for (int i = 0; i < req_msg->request_size(); ++i) {
        debug::MemoryRawRequest req = req_msg->request(i);
        debug::MemoryRawResponse *rsp = rsp_msg->add_response();

        if (req.len() > RAW_MEM_READ_MAX) {
            HAL_TRACE_DEBUG("Raw read request too large: {}", req.len());
            rsp->set_api_status(types::API_STATUS_INVALID_ARG);
            continue;
        }

        ret = hal::pd::asic_mem_read(req.address(), mem, req.len());
        if (ret != HAL_RET_OK) {
            HAL_TRACE_DEBUG("Raw read request (Addr:0x{:x}, Len:{}) failed",
                    req.address(), req.len());
            rsp->set_api_status(types::API_STATUS_HW_READ_ERROR);
            continue;
        }

        rsp->set_actiondata((void*)mem, req.len());
        rsp->set_api_status(types::API_STATUS_OK);
    }

    return Status::OK;
}

Status
DebugServiceImpl::MemoryRawUpdate(ServerContext *context,
                               const MemoryRawUpdateRequestMsg *req_msg,
                               MemoryRawUpdateResponseMsg *rsp_msg)
{
    hal_ret_t                          ret = HAL_RET_OK;

    for (int i = 0; i < req_msg->request_size(); ++i) {
        debug::MemoryRawUpdateRequest req = req_msg->request(i);
        debug::MemoryRawUpdateResponse *rsp = rsp_msg->add_response();

        ret = hal::pd::asic_mem_write(req.address(), (uint8_t*)req.actiondata().c_str(), req.len(), true);
        if (ret != HAL_RET_OK) {
            HAL_TRACE_DEBUG("Raw write request (Addr:0x{:x}, Len:{}) failed",
                    req.address(), req.len());
            rsp->set_api_status(types::API_STATUS_HW_WRITE_ERROR);
            continue;
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
        HAL_TRACE_DEBUG("REQ {}: wrap- {} reset- {} enable- {} instr- {} kd- {} traceenable- {}",
                         i, request.spec().wrap(), request.spec().reset(), request.spec().enable(),
                         request.spec().instructions(), request.spec().table_key(), request.spec().trace_enable());
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

Status
DebugServiceImpl::GenericOpn(ServerContext *context,
                             const GenericOpnRequestMsg *req_msg,
                             GenericOpnResponseMsg *rsp_msg)
{
    for (int i = 0; i < req_msg->request_size(); ++i) {
        debug::GenericOpnRequest  req  = req_msg->request(i);
        debug::GenericOpnResponse *rsp = rsp_msg->add_response();
        linkmgr::linkmgr_generic_debug_opn(req, rsp);
        rsp->set_api_status(types::API_STATUS_OK);
    }

    return Status::OK;
}

Status
DebugServiceImpl::FlushLogs(ServerContext *context,
                            const Empty *req,
                            FlushLogsResponseMsg *rsp)
{
    FlushLogsResponse    *response;

    HAL_TRACE_DEBUG("Received flush logs");
    response = rsp->add_response();
    hal::flush_logs(response);
    return Status::OK;
}

Status
DebugServiceImpl::ThreadGet(ServerContext *context,
                            const Empty *req,
                            ThreadResponseMsg *rsp)
{
    HAL_TRACE_DEBUG("Received thread get");
    hal::thread_get(rsp);
    return Status::OK;
}

Status
DebugServiceImpl::FteSpanGet(ServerContext *context,
                             const Empty *req,
                             FteSpanResponseMsg *rsp_msg)
{
    HAL_TRACE_DEBUG("Received FTE span get");

    hal::hal_cfg_db_open(hal::CFG_OP_READ);
    // debug::FteSpanResponse *rsp = rsp_msg->add_response();
    hal::fte_span_get(rsp_msg);
    hal::hal_cfg_db_close();
    return Status::OK;
}

Status
DebugServiceImpl::FteSpanUpdate(ServerContext *context,
                                const FteSpanRequestMsg *req_msg,
                                FteSpanResponseMsg *rsp_msg)
{
    HAL_TRACE_DEBUG("Received FTE span update: req_size: {}", req_msg->request_size());

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    for (int i = 0; i < req_msg->request_size(); ++i) {
        debug::FteSpanRequest req = req_msg->request(i);
        debug::FteSpanResponse *rsp = rsp_msg->add_response();
        hal::fte_span_update(req, rsp);
    }

    hal::hal_cfg_db_close();
    return Status::OK;
}

Status
DebugServiceImpl::ClockGet(ServerContext *context,
                           const Empty *req,
                           ClockResponse *rsp)
{
    HAL_TRACE_DEBUG("Received clock get");
    hal::clock_get(rsp);
    return Status::OK;
}

#if 0
Status
DebugServiceImpl::HbmBwGet(ServerContext *context,
                           const Empty *request,
                           HbmBwGetResponse *rsp)
{
    HAL_TRACE_DEBUG("Rcvd HBM BW Get Request");

    void *resp = NULL;

    hal::hal_cfg_db_open(hal::CFG_OP_READ);
    hal::hbm_bw_get(resp);
    hal::hal_cfg_db_close();

    return Status::OK;
}
#endif
