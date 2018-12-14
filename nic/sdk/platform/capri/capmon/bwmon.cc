//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file contains the implementation for fetching and storing bandwidth
/// info - capmon -b
//===----------------------------------------------------------------------===//

#include <stdint.h>
#include <stdio.h>

#include "cap_ms_c_hdr.h"
#include "cap_pics_c_hdr.h"
#include "cap_pp_c_hdr.h"
#include "cap_pxb_c_hdr.h"
#include "cap_top_csr_defines.h"

#include "platform/src/lib/pal/include/pal.h"
#include "capmon.hpp"

static inline void
capmon_asic_bwmon_rd_store(bwmon_t *bwmon_data, uint64_t rd_cnt,
                           uint32_t rd_latency_avg, uint32_t rd_latency_max,
                           uint32_t rd_bandwidth_avg, uint32_t rd_bandwidth_max,
                           uint32_t rd_trans, uint32_t rd_trans_no_drdy)
{
    bwmon_data->rd_cnt = rd_cnt;
    bwmon_data->rd_latency_avg = rd_latency_avg;
    bwmon_data->rd_latency_max = rd_latency_max;
    bwmon_data->rd_bandwidth_avg = rd_bandwidth_avg;
    bwmon_data->rd_bandwidth_max = rd_bandwidth_max;
    bwmon_data->rd_trans_no_drdy = rd_trans_no_drdy;
    bwmon_data->rd_trans = rd_trans;
}

static inline void
capmon_asic_bwmon_wr_store(bwmon_t *bwmon_data, uint64_t wr_cnt,
                           uint32_t wr_latency_avg, uint32_t wr_latency_max,
                           uint32_t wr_bandwidth_avg, uint32_t wr_bandwidth_max,
                           uint32_t wr_trans, uint32_t wr_trans_no_drdy)
{
    bwmon_data->wr_cnt = wr_cnt;
    bwmon_data->wr_latency_avg = wr_latency_avg;
    bwmon_data->wr_latency_max = wr_latency_max;
    bwmon_data->wr_bandwidth_avg = wr_bandwidth_avg;
    bwmon_data->wr_bandwidth_max = wr_bandwidth_max;
    bwmon_data->wr_trans_no_drdy = wr_trans_no_drdy;
    bwmon_data->wr_trans = wr_trans;
}

void
bwmon_fn(int index, uint32_t base_addr)
{
    // Use PICS defines to get relative spacing between monitor regs
    // base_addr is the address of
    // CAP_*_CSR_STA_AXI_BW_MON_RD_LATENCY_BYTE_ADDRESS =
    uint32_t rd_latency, rd_bandwidth, rd_cnt, rd_trans;
    uint32_t wr_latency, wr_bandwidth, wr_cnt, wr_trans;
    bwmon_t *bwmon_data = &asic->bwmons[index];

    pal_reg_rd32w(base_addr +
                      CAP_PICS_CSR_STA_AXI_BW_MON_RD_LATENCY_BYTE_ADDRESS -
                      CAP_PICS_CSR_STA_AXI_BW_MON_RD_LATENCY_BYTE_ADDRESS,
                  &rd_latency, 1);
    pal_reg_rd32w(base_addr +
                      CAP_PICS_CSR_STA_AXI_BW_MON_RD_BANDWIDTH_BYTE_ADDRESS -
                      CAP_PICS_CSR_STA_AXI_BW_MON_RD_LATENCY_BYTE_ADDRESS,
                  &rd_bandwidth, 1);
    pal_reg_rd32w(base_addr + CAP_PICS_CSR_CNT_AXI_BW_MON_RD_BYTE_ADDRESS -
                      CAP_PICS_CSR_STA_AXI_BW_MON_RD_LATENCY_BYTE_ADDRESS,
                  &rd_cnt, 1);
    pal_reg_rd32w(base_addr +
                      CAP_PICS_CSR_STA_AXI_BW_MON_RD_TRANSACTIONS_BYTE_ADDRESS -
                      CAP_PICS_CSR_STA_AXI_BW_MON_RD_LATENCY_BYTE_ADDRESS,
                  &rd_trans, 1);
    pal_reg_rd32w(base_addr +
                      CAP_PICS_CSR_STA_AXI_BW_MON_WR_LATENCY_BYTE_ADDRESS -
                      CAP_PICS_CSR_STA_AXI_BW_MON_RD_LATENCY_BYTE_ADDRESS,
                  &wr_latency, 1);
    pal_reg_rd32w(base_addr +
                      CAP_PICS_CSR_STA_AXI_BW_MON_WR_BANDWIDTH_BYTE_ADDRESS -
                      CAP_PICS_CSR_STA_AXI_BW_MON_RD_LATENCY_BYTE_ADDRESS,
                  &wr_bandwidth, 1);
    pal_reg_rd32w(base_addr + CAP_PICS_CSR_CNT_AXI_BW_MON_WR_BYTE_ADDRESS -
                      CAP_PICS_CSR_STA_AXI_BW_MON_RD_LATENCY_BYTE_ADDRESS,
                  &wr_cnt, 1);
    pal_reg_rd32w(base_addr +
                      CAP_PICS_CSR_STA_AXI_BW_MON_WR_TRANSACTIONS_BYTE_ADDRESS -
                      CAP_PICS_CSR_STA_AXI_BW_MON_RD_LATENCY_BYTE_ADDRESS,
                  &wr_trans, 1);

    capmon_asic_bwmon_rd_store(
        bwmon_data, rd_cnt,
        CAP_PICS_CSR_STA_AXI_BW_MON_RD_LATENCY_AVRG_GET(rd_latency),
        CAP_PICS_CSR_STA_AXI_BW_MON_RD_LATENCY_MAXV_GET(rd_latency),
        CAP_PICS_CSR_STA_AXI_BW_MON_RD_BANDWIDTH_AVRG_GET(rd_bandwidth),
        CAP_PICS_CSR_STA_AXI_BW_MON_RD_BANDWIDTH_MAXV_GET(rd_bandwidth),
        CAP_PICS_CSR_STA_AXI_BW_MON_RD_TRANSACTIONS_OUTSTANDING_GET(rd_trans),
        CAP_PICS_CSR_STA_AXI_BW_MON_RD_TRANSACTIONS_DESS_RDY_GET(rd_trans));
    capmon_asic_bwmon_wr_store(
        bwmon_data, wr_cnt,
        CAP_PICS_CSR_STA_AXI_BW_MON_WR_LATENCY_AVRG_GET(wr_latency),
        CAP_PICS_CSR_STA_AXI_BW_MON_WR_LATENCY_MAXV_GET(wr_latency),
        CAP_PICS_CSR_STA_AXI_BW_MON_WR_BANDWIDTH_AVRG_GET(wr_bandwidth),
        CAP_PICS_CSR_STA_AXI_BW_MON_WR_BANDWIDTH_MAXV_GET(wr_bandwidth),
        CAP_PICS_CSR_STA_AXI_BW_MON_WR_TRANSACTIONS_OUTSTANDING_GET(wr_trans),
        CAP_PICS_CSR_STA_AXI_BW_MON_WR_TRANSACTIONS_DESS_RDY_GET(wr_trans));
}

void
bwmon_read_counters()
{
    int index = 0;

    bwmon_fn(index++, (CAP_ADDR_BASE_PXB_PXB_OFFSET +
                       CAP_PXB_CSR_STA_AXI_BW_MON_RD_LATENCY_BYTE_ADDRESS));

    bwmon_fn(index++, (CAP_ADDR_BASE_RPC_PICS_OFFSET +
                       CAP_PICS_CSR_STA_AXI_BW_MON_RD_LATENCY_BYTE_ADDRESS));

    bwmon_fn(index++, (CAP_ADDR_BASE_TPC_PICS_OFFSET +
                       CAP_PICS_CSR_STA_AXI_BW_MON_RD_LATENCY_BYTE_ADDRESS));

    bwmon_fn(index++, (CAP_ADDR_BASE_SSI_PICS_OFFSET +
                       CAP_PICS_CSR_STA_AXI_BW_MON_RD_LATENCY_BYTE_ADDRESS));

    bwmon_fn(index++, (CAP_ADDR_BASE_SSE_PICS_OFFSET +
                       CAP_PICS_CSR_STA_AXI_BW_MON_RD_LATENCY_BYTE_ADDRESS));

    bwmon_fn(index++, (CAP_ADDR_BASE_MS_MS_OFFSET +
                       CAP_MS_CSR_STA_AXI_BW_MON_RD_LATENCY_BYTE_ADDRESS));
}

void
bwmon_reset_counters()
{
}
