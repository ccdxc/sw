// {C} Copyright 2020 Pensando Systems Inc. All rights reserved

#ifndef __ELBA_QSTATE_HPP__
#define __ELBA_QSTATE_HPP__

#include "platform/utils/lif_manager_base.hpp"
#include "platform/utils/program.hpp"
#include "nic/sdk/asic/cmn/asic_qstate.hpp"

namespace sdk {
namespace platform {
namespace elba {

void elba_clear_qstate_map(uint32_t lif_id);
void elba_program_qstate_map(lif_qstate_t *qstate, uint8_t enable);
void elba_read_qstate_map(lif_qstate_t *qstate);
void elba_reprogram_qstate_map(uint32_t lif_id, uint8_t enable);
sdk_ret_t elba_clear_qstate(lif_qstate_t *qstate);
void elba_reset_qstate_map(uint32_t lif_id);
void push_qstate_to_elba(utils::LIFQState *qstate, int cos);
void clear_qstate(utils::LIFQState *qstate);
void read_lif_params_from_elba(utils::LIFQState *qstate);

}    // namespace elba
}    // namespace platform
}    // namespace sdk

#endif    // _ELBA_QSTATE_HPP_
