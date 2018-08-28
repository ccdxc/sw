// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#include <inttypes.h>
#include "cap_top_csr_defines.h"
#include "cap_ms_c_hdr.h"
#include "sdk/pal.hpp"

#define CAP_SBUS_SBUS_RST 0x20
#define CAP_SBUS_SBUS_WR  0x21
#define CAP_SBUS_SBUS_RD  0x22

int
cap_ms_write(int chip_id, int addr, uint32_t data)
{
    sdk::lib::pal_reg_write(CAP_ADDR_BASE_MS_MS_OFFSET + addr,
                            &data,
                            1);
    return 0;
}

uint32_t
cap_ms_read(int chip_id, int addr)
{
    uint32_t data = 0;
    sdk::lib::pal_reg_read(CAP_ADDR_BASE_MS_MS_OFFSET + addr,
                           &data,
                           1);
    return data;
}

void
cap_ms_sbus_write(int chip_id, int rcvr_addr, int data_addr, int sbus_data)
{
    uint32_t data = CAP_MS_CSR_CFG_SBUS_INDIR_COMMAND_SET(CAP_SBUS_SBUS_WR) |
                    CAP_MS_CSR_CFG_SBUS_INDIR_DATA_ADDR_SET(data_addr)      |
                    CAP_MS_CSR_CFG_SBUS_INDIR_RCVR_ADDR_SET(rcvr_addr);

    cap_ms_write(chip_id, CAP_MS_CSR_CFG_SBUS_INDIR_BYTE_OFFSET, data);

    data = CAP_MS_CSR_DHS_SBUS_INDIR_ENTRY_DATA_SET(sbus_data);
    cap_ms_write(chip_id, CAP_MS_CSR_DHS_SBUS_INDIR_BYTE_ADDRESS, data);

    data = CAP_MS_CSR_CFG_SBUS_RESULT_MODE_SET(0);
    cap_ms_write(chip_id, CAP_MS_CSR_CFG_SBUS_RESULT_BYTE_OFFSET, data);
}

int
cap_ms_sbus_read(int chip_id, int rcvr_addr, int data_addr)
{
    int data = CAP_MS_CSR_CFG_SBUS_INDIR_COMMAND_SET(CAP_SBUS_SBUS_RD) |
               CAP_MS_CSR_CFG_SBUS_INDIR_DATA_ADDR_SET(data_addr)      |
               CAP_MS_CSR_CFG_SBUS_INDIR_RCVR_ADDR_SET(rcvr_addr);
    cap_ms_write(chip_id, CAP_MS_CSR_CFG_SBUS_INDIR_BYTE_OFFSET, data);

    data = CAP_MS_CSR_CFG_SBUS_RESULT_MODE_SET(0);
    cap_ms_write(chip_id, CAP_MS_CSR_CFG_SBUS_RESULT_BYTE_OFFSET, data);

    data = cap_ms_read(chip_id, CAP_MS_CSR_DHS_SBUS_INDIR_BYTE_ADDRESS);

    return CAP_MS_CSR_DHS_SBUS_INDIR_ENTRY_DATA_GET(data);
}
