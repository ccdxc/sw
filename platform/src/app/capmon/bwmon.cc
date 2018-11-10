/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */
#include <stdint.h>
#include <stdio.h>

#include "cap_ms_c_hdr.h"
#include "cap_pics_c_hdr.h"
#include "cap_pp_c_hdr.h"
#include "cap_pxb_c_hdr.h"
#include "cap_top_csr_defines.h"

#include "pal.h"

void
bwmon(uint32_t base_addr)
{
    // Use PICS defines to get relative spacing between monitor regs
    // base_addr is the address of CAP_*_CSR_STA_AXI_BW_MON_RD_LATENCY_BYTE_ADDRESS =
    uint32_t rd_latency, rd_bandwidth, rd_cnt, rd_trans;
    uint32_t wr_latency, wr_bandwidth, wr_cnt, wr_trans;

    pal_reg_rd32w(base_addr + CAP_PICS_CSR_STA_AXI_BW_MON_RD_LATENCY_BYTE_ADDRESS -
                      CAP_PICS_CSR_STA_AXI_BW_MON_RD_LATENCY_BYTE_ADDRESS,
                  &rd_latency, 1);
    pal_reg_rd32w(base_addr + CAP_PICS_CSR_STA_AXI_BW_MON_RD_BANDWIDTH_BYTE_ADDRESS -
                      CAP_PICS_CSR_STA_AXI_BW_MON_RD_LATENCY_BYTE_ADDRESS,
                  &rd_bandwidth, 1);
    pal_reg_rd32w(base_addr + CAP_PICS_CSR_CNT_AXI_BW_MON_RD_BYTE_ADDRESS -
                      CAP_PICS_CSR_STA_AXI_BW_MON_RD_LATENCY_BYTE_ADDRESS,
                  &rd_cnt, 1);
    pal_reg_rd32w(base_addr + CAP_PICS_CSR_STA_AXI_BW_MON_RD_TRANSACTIONS_BYTE_ADDRESS -
                      CAP_PICS_CSR_STA_AXI_BW_MON_RD_LATENCY_BYTE_ADDRESS,
                  &rd_trans, 1);
    pal_reg_rd32w(base_addr + CAP_PICS_CSR_STA_AXI_BW_MON_WR_LATENCY_BYTE_ADDRESS -
                      CAP_PICS_CSR_STA_AXI_BW_MON_RD_LATENCY_BYTE_ADDRESS,
                  &wr_latency, 1);
    pal_reg_rd32w(base_addr + CAP_PICS_CSR_STA_AXI_BW_MON_WR_BANDWIDTH_BYTE_ADDRESS -
                      CAP_PICS_CSR_STA_AXI_BW_MON_RD_LATENCY_BYTE_ADDRESS,
                  &wr_bandwidth, 1);
    pal_reg_rd32w(base_addr + CAP_PICS_CSR_CNT_AXI_BW_MON_WR_BYTE_ADDRESS -
                      CAP_PICS_CSR_STA_AXI_BW_MON_RD_LATENCY_BYTE_ADDRESS,
                  &wr_cnt, 1);
    pal_reg_rd32w(base_addr + CAP_PICS_CSR_STA_AXI_BW_MON_WR_TRANSACTIONS_BYTE_ADDRESS -
                      CAP_PICS_CSR_STA_AXI_BW_MON_RD_LATENCY_BYTE_ADDRESS,
                  &wr_trans, 1);

    printf("\n");

    printf(" rd_cnt=%12u", rd_cnt);
    printf(" rd_lat=%8u/%8u", CAP_PICS_CSR_STA_AXI_BW_MON_RD_LATENCY_AVRG_GET(rd_latency),
           CAP_PICS_CSR_STA_AXI_BW_MON_RD_LATENCY_MAXV_GET(rd_latency));
    printf(" rd_bw=%4u/%4u", CAP_PICS_CSR_STA_AXI_BW_MON_RD_BANDWIDTH_AVRG_GET(rd_bandwidth),
           CAP_PICS_CSR_STA_AXI_BW_MON_RD_BANDWIDTH_MAXV_GET(rd_bandwidth));
    printf(" rd_pend=%4u, no_drdy=%u",
           CAP_PICS_CSR_STA_AXI_BW_MON_RD_TRANSACTIONS_OUTSTANDING_GET(rd_trans),
           CAP_PICS_CSR_STA_AXI_BW_MON_RD_TRANSACTIONS_DESS_RDY_GET(rd_trans));

    printf("\n");

    printf(" wr_cnt=%12u", wr_cnt);
    printf(" wr_lat=%8u/%8u", CAP_PICS_CSR_STA_AXI_BW_MON_WR_LATENCY_AVRG_GET(wr_latency),
           CAP_PICS_CSR_STA_AXI_BW_MON_WR_LATENCY_MAXV_GET(wr_latency));
    printf(" wr_bw=%4u/%4u", CAP_PICS_CSR_STA_AXI_BW_MON_WR_BANDWIDTH_AVRG_GET(wr_bandwidth),
           CAP_PICS_CSR_STA_AXI_BW_MON_WR_BANDWIDTH_MAXV_GET(wr_bandwidth));
    printf(" wr_pend=%4u, no_drdy=%u",
           CAP_PICS_CSR_STA_AXI_BW_MON_WR_TRANSACTIONS_OUTSTANDING_GET(wr_trans),
           CAP_PICS_CSR_STA_AXI_BW_MON_WR_TRANSACTIONS_DESS_RDY_GET(wr_trans));

    printf("\n");
}

void
bwmon_read_counters()
{
    printf("PXB:");
    bwmon((CAP_ADDR_BASE_PXB_PXB_OFFSET + CAP_PXB_CSR_STA_AXI_BW_MON_RD_LATENCY_BYTE_ADDRESS));

    printf("RXD:");
    bwmon((CAP_ADDR_BASE_RPC_PICS_OFFSET + CAP_PICS_CSR_STA_AXI_BW_MON_RD_LATENCY_BYTE_ADDRESS));

    printf("TXD:");
    bwmon((CAP_ADDR_BASE_TPC_PICS_OFFSET + CAP_PICS_CSR_STA_AXI_BW_MON_RD_LATENCY_BYTE_ADDRESS));

    printf("SI:");
    bwmon((CAP_ADDR_BASE_SSI_PICS_OFFSET + CAP_PICS_CSR_STA_AXI_BW_MON_RD_LATENCY_BYTE_ADDRESS));

    printf("SE:");
    bwmon((CAP_ADDR_BASE_SSE_PICS_OFFSET + CAP_PICS_CSR_STA_AXI_BW_MON_RD_LATENCY_BYTE_ADDRESS));

    printf("MS: ");
    bwmon((CAP_ADDR_BASE_MS_MS_OFFSET + CAP_MS_CSR_STA_AXI_BW_MON_RD_LATENCY_BYTE_ADDRESS));
}

void
bwmon_reset_counters()
{
}
