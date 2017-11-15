// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

//------------------------------------------------------------------------------
// HAL DEBUG service implementation
//------------------------------------------------------------------------------

#include "nic/include/base.h"
#include "nic/include/trace.hpp"
#include "nic/hal/svc/debug_svc.hpp"
#include "nic/gen/iris/include/p4pd.h"
#include "nic/hal/pd/p4pd_api.hpp"

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
    int chip = 0;
    p4pd_error_t pd_err = P4PD_SUCCESS;

    DebugSpec spec = req->request(0);
    DebugResponse *response = rsp->add_response();

    debug::DebugKeyHandle key_handle = spec.key_or_handle();

    if (key_handle.key_or_handle_case() ==
                debug::DebugKeyHandle::kTableId) {
        table_access = 1;
        HAL_TRACE_DEBUG("Table: {}",
                        key_handle.table_id());
    }

    if (key_handle.key_or_handle_case() ==
                debug::DebugKeyHandle::kRegId) {
        reg_access = 1;
        HAL_TRACE_DEBUG("Reg: {}",
                        key_handle.reg_id());
    }

    HAL_TRACE_DEBUG("operation: {}", spec.opn_type(), spec.index());

    if (table_access) {
        HAL_TRACE_DEBUG("operation: {} index: {}",
                        spec.opn_type(), spec.index());

        if (spec.opn_type() == debug::DEBUG_OP_TYPE_READ) {
            pd_err = p4pd_global_entry_read(
                                        key_handle.table_id(),
                                        spec.index(),
                                        (void *) spec.swkey().c_str(),
                                        (void *) spec.swkey_mask().c_str(),
                                        (void *) spec.actiondata().c_str());

            if (pd_err != P4PD_SUCCESS) {
                HAL_TRACE_DEBUG("Hardware read fail");
                return Status::OK;
            }
            DebugSpec *rsp_spec = response->mutable_rsp_data();
            rsp_spec->set_swkey(spec.swkey());
            rsp_spec->set_swkey_mask(spec.swkey_mask());
            rsp_spec->set_actiondata(spec.actiondata());
        } else {
            uint32_t hwkey_len        = 0;
            uint32_t hwkeymask_len    = 0;
            uint32_t hwactiondata_len = 0;

            p4pd_hwentry_query(key_handle.table_id(), &hwkey_len,
                               &hwkeymask_len, &hwactiondata_len);

            void *hwkey      = NULL;
            void *hwkeymask  = NULL;

            HAL_TRACE_DEBUG("hwkeylen: {}, hwkeymask_len: {}",
                            hwkey_len, hwkeymask_len);

            // build hw key & mask
            hwkey      = HAL_MALLOC(HAL_MEM_ALLOC_DEBUG_CLI,
                                    (hwkey_len     + 7)/8);

            hwkeymask  = HAL_MALLOC(HAL_MEM_ALLOC_DEBUG_CLI,
                                    (hwkeymask_len + 7)/8);

            memset(hwkey,     0, (hwkey_len     + 7)/8);
            memset(hwkeymask, 0, (hwkeymask_len + 7)/8);

            pd_err = p4pd_hwkey_hwmask_build(
                                key_handle.table_id(),
                                (void *) spec.swkey().c_str(),
                                (void *) spec.swkey_mask().c_str(),
                                (uint8_t *)hwkey,
                                (uint8_t *)hwkeymask);

            pd_err = p4pd_entry_write(
                                key_handle.table_id(),
                                spec.index(),
                                (uint8_t *)hwkey,
                                (uint8_t *)hwkeymask,
                                (void *) spec.actiondata().c_str());

            if (pd_err != P4PD_SUCCESS) {
                HAL_TRACE_DEBUG("Hardware write fail");
                return Status::OK;
            } else {
                HAL_TRACE_DEBUG("Hardware write PASS");
            }

            if (hwkey)     HAL_FREE(HAL_MEM_ALLOC_DEBUG_CLI, hwkey);
            if (hwkeymask) HAL_FREE(HAL_MEM_ALLOC_DEBUG_CLI, hwkeymask);
        }
    }

    if (reg_access) {
        HAL_TRACE_DEBUG("Address: 0x{0:x}", spec.addr());

        if (spec.opn_type() == debug::DEBUG_OP_TYPE_READ) {
            data = read_reg_base(chip, spec.addr());

            HAL_TRACE_DEBUG("Read Data: 0x{0:x}", data);

            response->set_ret_data(data);
        } else {
            HAL_TRACE_DEBUG("Writing Data: 0x{0:x}", spec.reg_data());
            write_reg_base(chip, spec.addr(), spec.reg_data());
        }

        response->set_debug_status(types::API_STATUS_OK);
    }

    response->set_debug_status(types::API_STATUS_OK);

    return Status::OK;
}
