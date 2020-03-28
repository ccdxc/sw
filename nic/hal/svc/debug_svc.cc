//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//
// HAL DEBUG service implementation
//------------------------------------------------------------------------------

#include "nic/include/base.hpp"
#include "nic/hal/hal_trace.hpp"
#include "nic/hal/svc/debug_svc.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/src/debug/debug.hpp"
#include "nic/sdk/asic/rw/asicrw.hpp"
#include "nic/hal/src/debug/snake.hpp"
#include "nic/include/asic_pd.hpp"
#include "nic/linkmgr/linkmgr_debug.hpp"
#include <malloc.h>

// TODO: we don't seem to be using these ??
#include <vector>
#include <tuple>
using grpc::ServerWriter;
using std::vector;
using std::tuple;

using sdk::asic::pd::port_queue_credit_t;
using sdk::asic::pd::queue_credit_t;

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
        if (req.id_name_or_addr_case() == debug::RegisterRequest::kAddr) {
            sdk::asic::asic_reg_read(req.addr(), &val, 1, true);
            reg_data->set_value(std::to_string(val));
            rsp->set_api_status(types::API_STATUS_OK);
        } else {
            rsp->set_api_status(types::API_STATUS_INVALID_ARG);
        }
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

        if (req.id_name_or_addr_case() == debug::RegisterRequest::kAddr) {
            sdk::asic::asic_reg_write(req.addr(), &data);
            rsp->set_api_status(types::API_STATUS_OK);
        } else if (req.id_name_or_addr_case() == debug::RegisterRequest::kRegId){
            hal::register_update(req, rsp);
            rsp->set_api_status(types::API_STATUS_OK);
        } else {
            rsp->set_api_status(types::API_STATUS_INVALID_ARG);
        }
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

#define RAW_MEM_READ_MAX        1024
Status
DebugServiceImpl::MemoryRawGet(ServerContext *context,
                            const MemoryRawRequestMsg *req_msg,
                            MemoryRawResponseMsg *rsp_msg)
{
    sdk_ret_t    ret = SDK_RET_OK;
    uint8_t      mem[RAW_MEM_READ_MAX];

    for (int i = 0; i < req_msg->request_size(); ++i) {
        debug::MemoryRawRequest req = req_msg->request(i);
        debug::MemoryRawResponse *rsp = rsp_msg->add_response();

        if (req.len() > RAW_MEM_READ_MAX) {
            HAL_TRACE_DEBUG("Raw read request too large: {}", req.len());
            rsp->set_api_status(types::API_STATUS_INVALID_ARG);
            continue;
        }

        ret = sdk::asic::asic_mem_read(req.address(), mem, req.len());
        if (ret != SDK_RET_OK) {
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
    sdk_ret_t    ret = SDK_RET_OK;

    for (int i = 0; i < req_msg->request_size(); ++i) {
        debug::MemoryRawUpdateRequest req = req_msg->request(i);
        debug::MemoryRawUpdateResponse *rsp = rsp_msg->add_response();

        ret = sdk::asic::asic_mem_write(req.address(),
                                        (uint8_t *)req.actiondata().c_str(),
                                        req.len(),
                                        ASIC_WRITE_MODE_BLOCKING);
        if (ret != SDK_RET_OK) {
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
    uint32_t    i, nreqs = req->request_size();

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
DebugServiceImpl::SessionCtrlUpdate(ServerContext *context,
                              const SessionCtrlRequestMsg *req,
                              Empty  *rsp)
{
    uint32_t         i, nreqs = req->spec_size();
    

    HAL_TRACE_DEBUG("Received session ctrl update");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }
    for (i = 0; i < nreqs; i++) {
        auto spec = req->spec(i);
        hal::session_ctrl_update(spec);
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

Status
DebugServiceImpl::HbmBwGet(ServerContext *context,
                           const HbmBwGetRequestMsg *req,
                           HbmBwGetResponseMsg *rsp)
{
    HAL_TRACE_DEBUG("Rcvd HBM BW Get Request");

    HbmBwGetRequest request = req->request(0);

    hal::hal_cfg_db_open(hal::CFG_OP_READ);
    hal::hbm_bw_get(&request, rsp);
    hal::hal_cfg_db_close();

    return Status::OK;
}

Status
DebugServiceImpl::LlcClear(ServerContext *context,
                           const Empty *request,
                           Empty *rsp)
{
    HAL_TRACE_DEBUG("Rcvd LLC Clear Request");

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    hal::llc_clear();
    hal::hal_cfg_db_close();

    return Status::OK;
}

Status
DebugServiceImpl::LlcSetup(ServerContext *context,
                           const LlcSetupRequestMsg *req,
                           LlcSetupResponseMsg *rsp)
{
    HAL_TRACE_DEBUG("Rcvd LLC Setup Request");

    LlcSetupRequest request = req->request(0);

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    hal::llc_setup(&request, rsp->add_response());
    hal::hal_cfg_db_close();

    return Status::OK;
}

Status
DebugServiceImpl::LlcGet(ServerContext *context,
                         const Empty *req,
                         LlcGetResponseMsg *rsp)
{
    LlcGetResponse    *response;

    HAL_TRACE_DEBUG("Received Llc Get Request");

    response = rsp->add_response();

    hal::hal_cfg_db_open(hal::CFG_OP_READ);
    hal::llc_get(response);
    hal::hal_cfg_db_close();

    return Status::OK;
}

Status
DebugServiceImpl::HbmCacheSetup(ServerContext *context,
                                const HbmCacheRequestMsg *req,
                                HbmCacheResponseMsg *rsp)
{
    HAL_TRACE_DEBUG("Rcvd HBM Cache Setup Request");

    HbmCacheRequest request = req->request(0);
    hal::hal_cfg_db_open(hal::CFG_OP_READ);
    hal::hbm_cache_setup(&request, rsp->add_response());
    hal::hal_cfg_db_close();

    return Status::OK;
}

Status
DebugServiceImpl::SchedulerStatsGet(ServerContext *context,
                                    const Empty *req,
                                    SchedulerStatsResponseMsg *rsp)
{
    SchedulerStatsResponse *response;

    HAL_TRACE_DEBUG("Received Scheduler Stats Get Request");

    response = rsp->add_response();

    hal::hal_cfg_db_open(hal::CFG_OP_READ);
    hal::scheduler_stats_get(response);
    hal::hal_cfg_db_close();

    return Status::OK;
}

Status
DebugServiceImpl::PacketBufferUpdate(ServerContext *context,
                                     const PacketBufferRequestMsg *req,
                                     PacketBufferResponseMsg *rsp)
{
    HAL_TRACE_DEBUG("Rcvd Packet Buffer Update Request");

    PacketBufferRequest request = req->request(0);
    const PacketBufferSpec &spec = request.spec();

    if (spec.has_pause()) {
        hal::packet_buffer_update(&request, rsp->add_response());
    }

    if (spec.has_span()) {
        hal::span_threshold_update(spec.span().span_threshold());
    }
    return Status::OK;
}

Status
DebugServiceImpl::XcvrValidCheckEnable(ServerContext *context,
                                       const Empty *req,
                                       Empty *rsp)
{
    HAL_TRACE_DEBUG("Received transceiver valid check enable Request");
    hal::xcvr_valid_check_enable(true);
    return Status::OK;
}

Status
DebugServiceImpl::XcvrValidCheckDisable(ServerContext *context,
                                        const Empty *req,
                                        Empty *rsp)
{
    HAL_TRACE_DEBUG("Received transceiver valid check disable Request");
    hal::xcvr_valid_check_enable(false);
    return Status::OK;
}

Status
DebugServiceImpl::SnakeTestCreate(ServerContext *context,
                                  const SnakeTestRequestMsg *req_msg,
                                  SnakeTestResponseMsg *rsp_msg)
{
    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    for (int i = 0; i < req_msg->request_size(); ++i) {
        debug::SnakeTestRequest req = req_msg->request(i);
        debug::SnakeTestResponse *rsp = rsp_msg->add_response();
        hal::snake_test_create(req, rsp);
    }

    hal::hal_cfg_db_close();
    return Status::OK;
}

Status
DebugServiceImpl::SnakeTestDelete(ServerContext *context,
                                  const SnakeTestDeleteRequestMsg *req_msg,
                                  SnakeTestDeleteResponseMsg *rsp_msg)
{
    uint32_t     i, nreqs = req_msg->request_size();
    SnakeTestDeleteResponse     *response;

    HAL_TRACE_DEBUG("Received snake test delete");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    hal::hal_cfg_db_open(hal::CFG_OP_WRITE);
    for (i = 0; i < nreqs; i++) {
        response = rsp_msg->add_response();
        auto spec = req_msg->request(i);
        hal::snake_test_delete(spec, response);
    }
    hal::hal_cfg_db_close();
    return Status::OK;
}

Status
DebugServiceImpl::SnakeTestGet(ServerContext *context,
                               const Empty *req,
                               SnakeTestResponseMsg *rsp_msg)
{
    HAL_TRACE_DEBUG("Received snake test get");

    hal::hal_cfg_db_open(hal::CFG_OP_READ);
    hal::snake_test_get(rsp_msg);
    hal::hal_cfg_db_close();
    return Status::OK;
}

Status
DebugServiceImpl::MemoryTrim(ServerContext *context,
                             const Empty *req,
                             Empty *rsp_msg)
{
    HAL_TRACE_DEBUG("Received memory trim");

    malloc_trim(0);
    return Status::OK;
}

void
queue_credits_proto_fill(uint32_t port_num,
                         port_queue_credit_t *credit,
                         void *ctxt)
{
    QueueCreditsGetResponse *proto = (QueueCreditsGetResponse *)ctxt;
    auto port_credit = proto->add_portqueuecredit();
    port_credit->set_port(port_num);

    for (uint32_t j = 0; j < credit->num_queues; j++) {
        auto queue_credit = port_credit->add_queuecredit();
        queue_credit->set_queue(credit->queues[j].oq);
        queue_credit->set_credit(credit->queues[j].credit);
    }
}

Status
DebugServiceImpl::QueueCreditsGet(ServerContext *context,
                                  const Empty *req,
                                  QueueCreditsGetResponse *rsp_msg)
{
    HAL_TRACE_DEBUG("Received queue credits get");
    sdk::asic::pd::queue_credits_get(queue_credits_proto_fill, rsp_msg);
    rsp_msg->set_apistatus(types::API_STATUS_OK);
    return Status::OK;
}

Status
DebugServiceImpl::OifListGet(ServerContext *context,
                             const OifListGetRequestMsg *req,
                             ServerWriter<OifListGetResponseMsg> *writer)
{
     uint32_t    i, nreqs = req->request_size();

     hal::hal_cfg_db_open(hal::CFG_OP_READ);
     HAL_TRACE_DEBUG("Rcvd oifl Get");
     if (nreqs == 0) {
         HAL_TRACE_DEBUG("Rcvd oifl Get All Request");
         hal::oiflist_get_all_stream(writer);
         return Status::OK;
     }

     for (i = 0; i < nreqs; i++) {
         auto request = req->request(i);
         hal::oiflist_get_stream(request, writer);
     }
     hal::hal_cfg_db_close();
     return Status::OK;
}
