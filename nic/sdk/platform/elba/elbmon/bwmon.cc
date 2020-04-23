//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file contains the implementation for fetching and storing bandwidth
/// info - elbmon -b
//===----------------------------------------------------------------------===//

#include <stdint.h>
#include <stdio.h>

#include "elb_ms_c_hdr.h"
#include "elb_pics_c_hdr.h"
#include "elb_pp_c_hdr.h"
#include "elb_pxb_c_hdr.h"
#include "elb_top_csr_defines.h"
#include "elb_ns_ap_m_mini_c_hdr.h"
#include "elb_gl0_c_hdr.h"
#include "elb_gl1_c_hdr.h"
#include "elb_gl2_c_hdr.h"
#include "elb_gl3_c_hdr.h"
#include "elb_gl4_c_hdr.h"
#include "elb_gl5_c_hdr.h"
#include "elb_gl6_c_hdr.h"
#include "elb_gl7_c_hdr.h"
#include "platform/pal/include/pal.h"
#include "elbmon.hpp"

static inline void
elbmon_asic_bwmon_rd_store(bwmon_t *bwmon_data, uint64_t rd_cnt,
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
elbmon_asic_bwmon_wr_store(bwmon_t *bwmon_data, uint64_t wr_cnt,
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
    // Use NS defines to get relative spacing between monitor regs
    // base_addr is the address of
    // ELB_*_CSR_STA_AXI_BW_MON_RD_LATENCY_BYTE_ADDRESS =
    uint32_t rd_latency, rd_bandwidth, rd_cnt, rd_trans;
    uint32_t wr_latency, wr_bandwidth, wr_cnt, wr_trans;
    bwmon_t *bwmon_data = &asic->bwmons[index];

    pal_reg_rd32w(base_addr +
                      NS_AP_M_MINI_CSR_STA_AXI_BW_MON_RD_LATENCY_BYTE_ADDRESS -
                      NS_AP_M_MINI_CSR_STA_AXI_BW_MON_RD_LATENCY_BYTE_ADDRESS,
                  &rd_latency, 1);
    pal_reg_rd32w(base_addr +
                      NS_AP_M_MINI_CSR_STA_AXI_BW_MON_RD_BANDWIDTH_BYTE_ADDRESS -
                      NS_AP_M_MINI_CSR_STA_AXI_BW_MON_RD_LATENCY_BYTE_ADDRESS,
                  &rd_bandwidth, 1);
    pal_reg_rd32w(base_addr + NS_AP_M_MINI_CSR_CNT_AXI_BW_MON_RD_BYTE_ADDRESS -
                      NS_AP_M_MINI_CSR_STA_AXI_BW_MON_RD_LATENCY_BYTE_ADDRESS,
                  &rd_cnt, 1);
    pal_reg_rd32w(base_addr +
                      NS_AP_M_MINI_CSR_STA_AXI_BW_MON_RD_TRANSACTIONS_BYTE_ADDRESS -
                      NS_AP_M_MINI_CSR_STA_AXI_BW_MON_RD_LATENCY_BYTE_ADDRESS,
                  &rd_trans, 1);
    pal_reg_rd32w(base_addr +
                      NS_AP_M_MINI_CSR_STA_AXI_BW_MON_WR_LATENCY_BYTE_ADDRESS -
                      NS_AP_M_MINI_CSR_STA_AXI_BW_MON_RD_LATENCY_BYTE_ADDRESS,
                  &wr_latency, 1);
    pal_reg_rd32w(base_addr +
                      NS_AP_M_MINI_CSR_STA_AXI_BW_MON_WR_BANDWIDTH_BYTE_ADDRESS -
                      NS_AP_M_MINI_CSR_STA_AXI_BW_MON_RD_LATENCY_BYTE_ADDRESS,
                  &wr_bandwidth, 1);
    pal_reg_rd32w(base_addr + NS_AP_M_MINI_CSR_CNT_AXI_BW_MON_WR_BYTE_ADDRESS -
                      NS_AP_M_MINI_CSR_STA_AXI_BW_MON_RD_LATENCY_BYTE_ADDRESS,
                  &wr_cnt, 1);
    pal_reg_rd32w(base_addr +
                      NS_AP_M_MINI_CSR_STA_AXI_BW_MON_WR_TRANSACTIONS_BYTE_ADDRESS -
                      NS_AP_M_MINI_CSR_STA_AXI_BW_MON_RD_LATENCY_BYTE_ADDRESS,
                  &wr_trans, 1);

    elbmon_asic_bwmon_rd_store(
        bwmon_data, rd_cnt,
        NS_AP_M_MINI_CSR_STA_AXI_BW_MON_RD_LATENCY_AVRG_GET(rd_latency),
        NS_AP_M_MINI_CSR_STA_AXI_BW_MON_RD_LATENCY_MAXV_GET(rd_latency),
        NS_AP_M_MINI_CSR_STA_AXI_BW_MON_RD_BANDWIDTH_AVRG_GET(rd_bandwidth),
        NS_AP_M_MINI_CSR_STA_AXI_BW_MON_RD_BANDWIDTH_MAXV_GET(rd_bandwidth),
        NS_AP_M_MINI_CSR_STA_AXI_BW_MON_RD_TRANSACTIONS_OUTSTANDING_GET(rd_trans),
        NS_AP_M_MINI_CSR_STA_AXI_BW_MON_RD_TRANSACTIONS_DESS_RDY_GET(rd_trans));
    elbmon_asic_bwmon_wr_store(
        bwmon_data, wr_cnt,
        NS_AP_M_MINI_CSR_STA_AXI_BW_MON_WR_LATENCY_AVRG_GET(wr_latency),
        NS_AP_M_MINI_CSR_STA_AXI_BW_MON_WR_LATENCY_MAXV_GET(wr_latency),
        NS_AP_M_MINI_CSR_STA_AXI_BW_MON_WR_BANDWIDTH_AVRG_GET(wr_bandwidth),
        NS_AP_M_MINI_CSR_STA_AXI_BW_MON_WR_BANDWIDTH_MAXV_GET(wr_bandwidth),
        NS_AP_M_MINI_CSR_STA_AXI_BW_MON_WR_TRANSACTIONS_OUTSTANDING_GET(wr_trans),
        NS_AP_M_MINI_CSR_STA_AXI_BW_MON_WR_TRANSACTIONS_DESS_RDY_GET(wr_trans));
}

void
bwmon_read_counters()
{
    int index = 0;

    bwmon_fn(index++, (ELB_ADDR_BASE_PXB_PXB_OFFSET +
		       ELB_GL0_CSR_PX_M_MINI_CSR_STA_AXI_BW_MON_RD_LATENCY_BYTE_ADDRESS));

    bwmon_fn(index++, (ELB_ADDR_BASE_PR_PR_OFFSET +
		       ELB_GL1_CSR_PR_M_MINI_CSR_STA_AXI_BW_MON_RD_LATENCY_BYTE_ADDRESS));

    bwmon_fn(index++, (ELB_ADDR_BASE_PT_PT_OFFSET +
		       ELB_GL4_CSR_PT_M_MINI_CSR_STA_AXI_BW_MON_RD_LATENCY_BYTE_ADDRESS));

    bwmon_fn(index++, (ELB_ADDR_BASE_SSI_PICS_OFFSET +
		       ELB_GL7_CSR_SI_M_MINI_CSR_STA_AXI_BW_MON_RD_LATENCY_BYTE_ADDRESS));

    bwmon_fn(index++, (ELB_ADDR_BASE_SSE_PICS_OFFSET +
		       ELB_GL5_CSR_SE_M_MINI_CSR_STA_AXI_BW_MON_RD_LATENCY_BYTE_ADDRESS));

    bwmon_fn(index++, (ELB_ADDR_BASE_MS_MS_OFFSET +
		       ELB_GL0_CSR_MS_M_MINI_CSR_STA_AXI_BW_MON_RD_LATENCY_BYTE_ADDRESS));

}

void
bwmon_reset_counters()
{
}

