// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __CAPRI_QSTATE_HPP__
#define __CAPRI_QSTATE_HPP__

#include "platform/utils/lif_manager_base.hpp"
#include "platform/utils/program.hpp"
#include "asic/pd/pd.hpp"

namespace sdk {
namespace platform {
namespace capri {

void capri_clear_qstate_map(uint32_t lif_id);
void capri_program_qstate_map(lif_qstate_t *qstate, uint8_t enable);
void capri_read_qstate_map(lif_qstate_t *qstate);
void capri_reprogram_qstate_map(uint32_t lif_id, uint8_t enable);
sdk_ret_t capri_read_qstate(uint64_t q_addr, uint8_t *buf, uint32_t q_size);
sdk_ret_t capri_write_qstate(uint64_t q_addr, const uint8_t *buf,
                             uint32_t q_size);
sdk_ret_t capri_clear_qstate(lif_qstate_t *qstate);
void capri_reset_qstate_map(uint32_t lif_id);

// ---- Deprecated ----
void push_qstate_to_capri(utils::LIFQState *qstate, int cos);
void clear_qstate(utils::LIFQState *qstate);
void read_lif_params_from_capri(utils::LIFQState *qstate);
// ---- Deprecated ----

} // namespace capri
} // namespace platform
} // namespace sdk

#endif // _CAPRI_QSTATE_HPP_
