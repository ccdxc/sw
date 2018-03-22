// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

//------------------------------------------------------------------------------
// HAL DEBUG service implementation
//------------------------------------------------------------------------------

#include "nic/include/base.h"
#include "nic/include/trace.hpp"
#include "nic/hal/svc/debug_svc.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/src/debug/debug.hpp"
#include "nic/include/asic_pd.hpp"
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
DebugServiceImpl::DebugInvoke(ServerContext *context,
                              const DebugRequestMsg *req,
                              DebugResponseMsg *rsp)
{
    hal_ret_t                            ret;
    bool                                 table_access = false;
    bool                                 reg_access = false;
    hal::pd::pd_debug_cli_read_args_t    args;
    string                               data;

    HAL_TRACE_DEBUG("Rcvd Ddebug request");

    DebugSpec spec = req->request(0);
    DebugResponse *response = rsp->add_response();
    debug::DebugKeyHandle key_handle = spec.key_or_handle();

    if ((key_handle.key_or_handle_case() == debug::DebugKeyHandle::kTableId) ||
        (key_handle.key_or_handle_case() == debug::DebugKeyHandle::kTableName)) {
        HAL_TRACE_DEBUG("{}: Table: {}", __FUNCTION__, key_handle.table_id());
        table_access = true;
    } else if ((key_handle.key_or_handle_case() == debug::DebugKeyHandle::kRegId) ||
               (key_handle.key_or_handle_case() == debug::DebugKeyHandle::kRegName)) {
        HAL_TRACE_DEBUG("{}: Reg: {}", __FUNCTION__, key_handle.reg_id());
        reg_access = true;
    }
    if (spec.mem_type() == debug::DEBUG_MEM_TYPE_TABLE) {
        table_access = true;
    } else if (spec.mem_type() == debug::DEBUG_MEM_TYPE_REG) {
        reg_access = true;
    }

    HAL_TRACE_DEBUG("{}: operation: {} index: {}", __FUNCTION__,
                    spec.opn_type(), spec.index());

    if (table_access) {
        if (spec.opn_type() == debug::DEBUG_OP_TYPE_READ) {
            DebugSpec *rsp_spec = response->mutable_spec();
            args.tableid = key_handle.table_id();
            args.index = spec.index();
            args.swkey = (void *)spec.swkey().c_str();
            args.swkey_mask = (void *)spec.swkey_mask().c_str();
            args.actiondata = (void *)spec.actiondata().c_str();
            ret = hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_DEBUG_CLI_READ, (void *)&args);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_DEBUG("{}: Hardware read failure, err : {}",
                                __FUNCTION__, ret);
                response->set_api_status(types::API_STATUS_HW_READ_ERROR);
                return Status::OK;
            }

            response->set_api_status(types::API_STATUS_OK);
            rsp_spec->set_swkey(spec.swkey());
            rsp_spec->set_swkey_mask(spec.swkey_mask());
            rsp_spec->set_actiondata(spec.actiondata());
        } else {
            args.tableid = key_handle.table_id();
            args.index = spec.index();
            args.swkey = (void *)spec.swkey().c_str();
            args.swkey_mask = (void *)spec.swkey_mask().c_str();
            args.actiondata = (void *)spec.actiondata().c_str();
            ret = hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_DEBUG_CLI_WRITE, (void *)&args);
            if (ret != HAL_RET_OK) {
                HAL_TRACE_DEBUG("{}: Hardware write failure, err : {}",
                                __FUNCTION__, ret);
                response->set_api_status(types::API_STATUS_HW_READ_ERROR);
                return Status::OK;
            }
            response->set_api_status(types::API_STATUS_OK);
        }
    } else if (reg_access) {
        HAL_TRACE_DEBUG("{}: Register address: 0x{0:x}", __FUNCTION__,
                        spec.addr());
        if (spec.opn_type() == debug::DEBUG_OP_TYPE_READ) {
            if (key_handle.key_or_handle_case() == debug::DebugKeyHandle::kRegName) {

                // PD-Cleanup: Don't call capri apis
                // data = hal::pd::asic_csr_dump((char *)key_handle.reg_name().c_str());
                debug::RegisterData *reg_data = response->add_data();
                reg_data->set_reg_name(key_handle.reg_name());
                reg_data->set_value(data);

            } else if (key_handle.key_or_handle_case() == debug::DebugKeyHandle::kBlockName) {
                HAL_TRACE_DEBUG("{}: block name", (char *) key_handle.block_name().c_str());

                /*char *root_path = NULL;
                root_path = std::getenv("HAL_CONFIG_PATH");
                string reg_file = std::string(root_path) + "reg_out" + (char *)key_handle.block_name().c_str() + ".txt";
                HAL_TRACE_DEBUG("reg file {}", reg_file);
                FILE *reg_fd = fopen(reg_file.c_str(), "ab+");

                if (!reg_fd) {
                    HAL_TRACE_DEBUG("Returned no file");
                    return Status::OK;
                }*/
                if (strcmp((char *)key_handle.block_name().c_str() , "all") == 0) {
                    // PD-Cleanup: Don't call capri apis
#if 0
                    vector<string> block_vector = hal::pd::asic_csr_list_get("cap0", 1);

                    for ( auto block : block_vector) {
                        HAL_TRACE_DEBUG("Block name: {}", block);
                        vector < tuple < std::string, string, std::string>> reg_data;
                        reg_data = hal::pd::asic_csr_dump_reg((char *) (block.c_str()), 1);
                        for (auto reg : reg_data) {
                            string reg_name;
                            string offset;
                            string value;
                            std::tie(reg_name, offset, value) = reg;
                            debug::RegisterData *reg_data = response->add_data();
                            reg_data->set_reg_name(reg_name);
                            reg_data->set_value(value);
                            reg_data->set_address(offset);
                            //std::fprintf(reg_fd, "%s, offset: %d , value: %s \n",reg_name.c_str(), offset, value.c_str());
                        }
                        //fflush(reg_fd);
                    }
#endif
                    //fclose(reg_fd);
                } else {
                    string block_name = key_handle.block_name();
                    HAL_TRACE_DEBUG("Block name: {}", block_name);
                    vector < tuple < std::string, string, std::string>> reg_data;
                    // PD-Cleanup: Don't call capri apis
#if 0
                    reg_data = hal::pd::asic_csr_dump_reg((char *) (block_name.c_str()), 1);
                    for (auto reg : reg_data) {
                        string reg_name;
                        string offset;
                        string value;
                        std::tie(reg_name, offset, value) = reg;
                        debug::RegisterData *reg_data = response->add_data();
                        reg_data->set_reg_name(reg_name);
                        reg_data->set_value(value);
                        reg_data->set_address(offset);
                        //std::fprintf(reg_fd, "%s, offset: %d , value: %s \n",reg_name.c_str(), offset, value.c_str());
                    }
#endif
                    //fflush(reg_fd);
                    //fclose(reg_fd);
                }
            }  else {
                uint64_t val = hal::pd::asic_reg_read(spec.addr());
                data = std::to_string(val);
            }
        } else {
            HAL_TRACE_DEBUG("{}: Writing Data: 0x{0:x}",
                            __FUNCTION__, spec.reg_data());
            uint32_t reg_data = spec.reg_data();
            hal::pd::asic_reg_write(spec.addr(), &reg_data);
        }
        response->set_api_status(types::API_STATUS_OK);
    }

    response->set_api_status(types::API_STATUS_OK);
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

Status DebugServiceImpl::MpuTraceOpn(ServerContext *context,
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

