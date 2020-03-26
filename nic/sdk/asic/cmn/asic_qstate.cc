// {C} Copyright 2020 Pensando Systems Inc. All rights reserved

#include "asic/rw/asicrw.hpp"
#include "asic/cmn/asic_qstate.hpp"

using namespace sdk::platform::utils;

namespace sdk  {
namespace asic {

int clear_qstate_mem (uint64_t base_addr, uint32_t size)
{
    // qstate is a multiple for 4K So it is safe to assume
    // 256 byte boundary.
    static uint8_t zeros[256] = {0};
    for (uint32_t i = 0; i < (size / sizeof(zeros)); i++) {
        sdk_ret_t rc = asic_mem_write(base_addr + (i * sizeof(zeros)),
                                                 zeros, sizeof(zeros));
        if (rc != SDK_RET_OK) {
            return -EIO;
        }
    }
    return 0;
}

int32_t read_qstate (uint64_t q_addr, uint8_t *buf, uint32_t q_size)
{
    sdk_ret_t rv = asic_mem_read(q_addr, buf, q_size);
    if (rv != SDK_RET_OK) {
        return -EIO;
    }
    return 0;
}

int32_t write_qstate (uint64_t q_addr, const uint8_t *buf, uint32_t q_size)
{
    sdk_ret_t rc = asic_mem_write(q_addr, (uint8_t *)buf, q_size);
    if (rc != SDK_RET_OK) {
        return -EIO;
    }
    return 0;
}

int32_t get_pc_offset (program_info *pinfo,
                       const char *prog_name,
                       const char *label,
                       uint8_t *offset)
{
    mem_addr_t off;

    off = pinfo->symbol_address((char *)prog_name, (char *)label);
    if (off == SDK_INVALID_HBM_ADDRESS)
        return -ENOENT;
    // 64 byte alignment check
    if ((off & 0x3F) != 0) {
        return -EIO;
    }
    // offset can be max 14 bits
    if (off > 0x3FC0) {
        return -EIO;
    }
    *offset = (uint8_t) (off >> 6);
    return 0;
}

}    // namespace asic
}    // namespace sdk
