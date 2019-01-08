// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __CAPRI_QSTATE_HPP__
#define __CAPRI_QSTATE_HPP__

#include "platform/utils/lif_manager_base.hpp"
#include "platform/utils/program.hpp"

namespace sdk {
namespace platform {
namespace capri {

void push_qstate_to_capri(utils::LIFQState *qstate, int cos);
void clear_qstate(utils::LIFQState *qstate);
void read_lif_params_from_capri(utils::LIFQState *qstate);

int32_t read_qstate(uint64_t q_addr, uint8_t *buf, uint32_t q_size);
int32_t write_qstate(uint64_t q_addr, const uint8_t *buf, uint32_t q_size);
int32_t get_pc_offset(utils::program_info *pinfo, const char *prog_name,
                      const char *label, uint8_t *offset);

} // namespace capri
} // namespace platform
} // namespace sdk

#endif // _CAPRI_QSTATE_HPP_
