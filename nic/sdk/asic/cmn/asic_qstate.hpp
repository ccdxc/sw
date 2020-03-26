// {C} Copyright 2020 Pensando Systems Inc. All rights reserved

#ifndef __ASIC_QSTATE_HPP__
#define __ASIC_QSTATE_HPP__

#include "include/sdk/base.hpp"
#include "include/sdk/types.hpp"
#include "platform/utils/lif_manager_base.hpp"
#include "platform/utils/program.hpp"
#include "asic/asic.hpp"

namespace sdk  {
namespace asic {

sdk_ret_t asic_clear_qstate(lif_qstate_t *qstate);
int32_t read_qstate(uint64_t q_addr, uint8_t *buf, uint32_t q_size);
int32_t write_qstate(uint64_t q_addr, const uint8_t *buf, uint32_t q_size);
int32_t get_pc_offset(program_info *pinfo, const char *prog_name,
                      const char *label, uint8_t *offset);

}    // asic
}    // sdk

#endif    // __ASIC_QSTATE_HPP__
