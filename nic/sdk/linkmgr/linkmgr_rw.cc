// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#include "linkmgr_rw.hpp"

namespace sdk {
namespace linkmgr {

uint32_t
read_reg_base (uint32_t chip, uint64_t addr)
{
    uint32_t data = 0x0;

    if (sdk::lib::pal_reg_read(addr, &data) != sdk::lib::PAL_RET_OK) {
        SDK_TRACE_ERR("read failed");
    }

    return data;
}

void write_reg_base(uint32_t chip, uint64_t addr, uint32_t  data)
{
    if (sdk::lib::pal_reg_write(addr, &data) != sdk::lib::PAL_RET_OK) {
        SDK_TRACE_ERR("write failed");
    }

    uint32_t read_data = read_reg_base(chip, addr);
    SDK_TRACE_DEBUG("read_data after write 0x%x", read_data);
}

}    // namespace linkmgr
}    // namespace sdk
