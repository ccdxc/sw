// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

//------------------------------------------------------------------------------
// HAL DEBUG service implementation
//------------------------------------------------------------------------------

#include "nic/include/base.h"
#include "nic/include/trace.hpp"
#include "nic/hal/svc/debug_svc.hpp"
#include "nic/gen/iris/include/p4pd.h"

extern uint32_t read_reg_base(uint32_t chip, uint64_t addr);

extern void write_reg_base(uint32_t chip, uint64_t addr, uint32_t data);

Status
DebugServiceImpl::DebugInvoke(ServerContext *context,
                              const DebugRequestMsg *req,
                              DebugResponseMsg *rsp)
{
    HAL_TRACE_DEBUG("Rcvd DEBUG Invoke Request");
    int table_access = 0;
    int reg_access   = 0;
    uint32_t data = 0x0;
    int chip = 0;

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

    HAL_TRACE_DEBUG("operation: {}", spec.opn_type());

    if (table_access) {
        egress_policer_actiondata *ad = (egress_policer_actiondata *)spec.str_data().c_str();

        HAL_TRACE_DEBUG("actionid: {}", ad->actionid);

        HAL_TRACE_DEBUG("entry_valid: {}",
                        ad->egress_policer_action_u.egress_policer_execute_egress_policer.entry_valid);
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

    return Status::OK;
}
