//------------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
// 
// read-write APIs to interact with the ASIC
//------------------------------------------------------------------------------

#ifndef __SDK_LINKMGR_RW_HPP__
#define __SDK_LINKMGR_RW_HPP__

#include "include/sdk/base.hpp"

namespace sdk {
namespace linkmgr {

extern uint32_t read_reg_base (uint32_t chip, uint64_t addr);
extern void write_reg_base(uint32_t chip, uint64_t addr, uint32_t  data);

// wrapper macros for read/write operations
#define READ_REG_BASE(chip, addr, data) {                   \
    SDK_TRACE_DEBUG("PORT: chip %u addr 0x%lx",             \
                    chip, addr);                            \
    *data = read_reg_base(chip, addr);                      \
}

#define WRITE_REG_BASE(chip, addr, data) {                  \
    SDK_TRACE_DEBUG("PORT: chip %u addr 0x%lx data 0x%x",   \
                    chip, addr, data);                      \
    write_reg_base(chip, addr, data);                       \
}

}    // namespace linkmgr
}    // namespace sdk

#endif  // __SDK_LINKMGR_RW_HPP__
