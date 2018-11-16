/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "dtls.hpp"

#include "cap_mpu_c_hdr.h"
#include "cap_te_c_hdr.h"
#include "cap_top_csr_defines.h"

#include "platform/src/lib/pal/include/pal.h"

uint64_t pipe_base[PIPE_CNT] = {[TXDMA] = CAP_ADDR_BASE_PCT_MPU_0_OFFSET,
                                [RXDMA] = CAP_ADDR_BASE_PCR_MPU_0_OFFSET,
                                [P4IG] = CAP_ADDR_BASE_SGI_MPU_0_OFFSET,
                                [P4EG] = CAP_ADDR_BASE_SGE_MPU_0_OFFSET};

uint64_t pipe_sz[PIPE_CNT] = {
    [TXDMA] = CAP_ADDR_BASE_PCT_MPU_1_OFFSET - CAP_ADDR_BASE_PCT_MPU_0_OFFSET,
    [RXDMA] = CAP_ADDR_BASE_PCR_MPU_1_OFFSET - CAP_ADDR_BASE_PCR_MPU_0_OFFSET,
    [P4IG] = CAP_ADDR_BASE_SGI_MPU_1_OFFSET - CAP_ADDR_BASE_SGI_MPU_0_OFFSET,
    [P4EG] = CAP_ADDR_BASE_SGE_MPU_1_OFFSET - CAP_ADDR_BASE_SGE_MPU_0_OFFSET,
};

uint64_t te_base[PIPE_CNT] = {[TXDMA] = CAP_ADDR_BASE_PCT_TE_0_OFFSET,
                              [RXDMA] = CAP_ADDR_BASE_PCR_TE_0_OFFSET,
                              [P4IG] = CAP_ADDR_BASE_SGI_TE_0_OFFSET,
                              [P4EG] = CAP_ADDR_BASE_SGE_TE_0_OFFSET};

uint64_t te_sz[PIPE_CNT] = {
    [TXDMA] = CAP_ADDR_BASE_PCT_TE_1_OFFSET - CAP_ADDR_BASE_PCT_TE_0_OFFSET,
    [RXDMA] = CAP_ADDR_BASE_PCR_TE_1_OFFSET - CAP_ADDR_BASE_PCR_TE_0_OFFSET,
    [P4IG] = CAP_ADDR_BASE_SGI_TE_1_OFFSET - CAP_ADDR_BASE_SGI_TE_0_OFFSET,
    [P4EG] = CAP_ADDR_BASE_SGE_TE_1_OFFSET - CAP_ADDR_BASE_SGE_TE_0_OFFSET};

uint64_t stage_cnt[PIPE_CNT] = {[TXDMA] = 8, [RXDMA] = 8, [P4IG] = 6, [P4EG] = 6};

uint64_t
get_sdp_base(uint8_t pipeline, uint8_t stage)
{
    assert(pipeline < PIPE_CNT);
    assert(stage < stage_cnt[pipeline]);

    return pipe_base[pipeline] + (pipe_sz[pipeline] * stage);
}

void
sdp_read_counters(int verbose, uint8_t pipeline, uint8_t stage)
{
    uint64_t base = get_sdp_base(pipeline, stage);
    uint32_t sdp[3]; /* only 3, CAP_MCPU_CSR_CNT_SDP_SIZE=4 looks wrong? */
    int sop_out, phv_fifo_depth;

    if (verbose) {
        pal_reg_rd32w(base + CAP_MPU_CSR_CNT_SDP_BYTE_OFFSET, sdp, 3);

        /* extract from sdp[1] */
        sop_out = CAP_MPU_CSR_CNT_SDP_CNT_SDP_1_3_SOP_OUT_GET(sdp[1]);
        /* extract from sdp[0] */
        phv_fifo_depth = CAP_MPU_CSR_CNT_SDP_CNT_SDP_0_3_PHV_FIFO_DEPTH_GET(sdp[0]);

        printf("  sdp PHV FIFO depth=%u\n", phv_fifo_depth);
        printf("  sdp PHV processed count=%u\n", sop_out);
    }
}

void
sdp_reset_counters(int verbose, uint8_t pipeline, uint8_t stage)
{
    uint64_t base = get_sdp_base(pipeline, stage);
    uint32_t zero[3] = {0};

    pal_reg_wr32w(base + CAP_MPU_CSR_CNT_SDP_BYTE_OFFSET, zero, 3);
}

uint64_t
get_te_base(uint8_t pipeline, uint8_t stage)
{
    assert(pipeline < PIPE_CNT);
    assert(stage < stage_cnt[pipeline]);

    return te_base[pipeline] + (te_sz[pipeline] * stage);
}

void
te_read_counters(int verbose, uint8_t pipeline, uint8_t stage)
{
    uint64_t base = get_te_base(pipeline, stage);
    uint32_t te_phv_cnt;
    uint32_t te_axi_cnt;
    uint32_t te_tcam_cnt;
    uint32_t te_mpu_cnt;

    if (verbose) {
        pal_reg_rd32w(base + CAP_TE_CSR_CNT_PHV_IN_SOP_BYTE_ADDRESS, &te_phv_cnt, 1);
        pal_reg_rd32w(base + CAP_TE_CSR_CNT_AXI_RDREQ_BYTE_ADDRESS, &te_axi_cnt, 1);
        pal_reg_rd32w(base + CAP_TE_CSR_CNT_TCAM_REQ_BYTE_ADDRESS, &te_tcam_cnt, 1);
        pal_reg_rd32w(base + CAP_TE_CSR_CNT_MPU_OUT_BYTE_ADDRESS, &te_mpu_cnt, 1);
        printf("  te phv=%u, axi_rd=%u, tcam=%u, mpu_out=%u\n", te_phv_cnt, te_axi_cnt,
               te_tcam_cnt, te_mpu_cnt);
    }
}

void
te_reset_counters(int verbose, uint8_t pipeline, uint8_t stage)
{
    uint64_t base = get_te_base(pipeline, stage);
    uint32_t zero = 0;

    if (verbose) {
        pal_reg_wr32w(base + CAP_TE_CSR_CNT_PHV_IN_SOP_BYTE_ADDRESS, &zero, 1);
        pal_reg_wr32w(base + CAP_TE_CSR_CNT_AXI_RDREQ_BYTE_ADDRESS, &zero, 1);
        pal_reg_wr32w(base + CAP_TE_CSR_CNT_TCAM_REQ_BYTE_ADDRESS, &zero, 1);
        pal_reg_wr32w(base + CAP_TE_CSR_CNT_MPU_OUT_BYTE_ADDRESS, &zero, 1);
    }
}

void
mpu_read_counters(int verbose, uint8_t pipeline, uint8_t stage, uint8_t mpu)
{
    uint64_t base = get_sdp_base(pipeline, stage);
    uint32_t mpu_offset = mpu * 4;
    uint32_t inst_executed;
    uint32_t icache_miss;
    uint32_t icache_fill_stall;
    uint32_t cycles;
    uint32_t phv_executed;
    uint32_t hazard_stall;
    uint32_t phvwr_stall;
    uint32_t memwr_stall;
    uint32_t tblwr_stall;
    uint32_t fence_stall;

    if (verbose) {
        pal_reg_rd32w(mpu_offset + base + CAP_MPU_CSR_CNT_INST_EXECUTED_BYTE_OFFSET,
                      &inst_executed, 1);
        pal_reg_rd32w(mpu_offset + base + CAP_MPU_CSR_CNT_ICACHE_MISS_BYTE_OFFSET, &icache_miss,
                      1);
        pal_reg_rd32w(mpu_offset + base + CAP_MPU_CSR_CNT_ICACHE_FILL_STALL_BYTE_OFFSET,
                      &icache_fill_stall, 1);
        pal_reg_rd32w(mpu_offset + base + CAP_MPU_CSR_CNT_CYCLES_BYTE_OFFSET, &cycles, 1);
        pal_reg_rd32w(mpu_offset + base + CAP_MPU_CSR_CNT_PHV_EXECUTED_BYTE_OFFSET, &phv_executed,
                      1);
        pal_reg_rd32w(mpu_offset + base + CAP_MPU_CSR_CNT_HAZARD_STALL_BYTE_OFFSET, &hazard_stall,
                      1);
        pal_reg_rd32w(mpu_offset + base + CAP_MPU_CSR_CNT_PHVWR_STALL_BYTE_OFFSET, &phvwr_stall,
                      1);
        pal_reg_rd32w(mpu_offset + base + CAP_MPU_CSR_CNT_MEMWR_STALL_BYTE_OFFSET, &memwr_stall,
                      1);
        pal_reg_rd32w(mpu_offset + base + CAP_MPU_CSR_CNT_TBLWR_STALL_BYTE_OFFSET, &tblwr_stall,
                      1);
        pal_reg_rd32w(mpu_offset + base + CAP_MPU_CSR_CNT_FENCE_STALL_BYTE_OFFSET, &fence_stall,
                      1);

        printf(" mpu %d cycles=%u", mpu, cycles);
        printf(" inst=%u", inst_executed);
        printf(" miss=%u", icache_miss);
        printf(" istl=%u", icache_fill_stall);
        printf(" phv=%u", phv_executed);
        printf(" hzrd=%u", hazard_stall);
        printf(" phvwr_stl=%u", phvwr_stall);
        printf(" memwr_stl=%u", memwr_stall);
        printf(" tblwr_stl=%u", tblwr_stall);
        printf(" fence_stl=%u\n", fence_stall);

        cycles = cycles == 0 ? 1 : cycles;
        printf(" mpu %u percentages", mpu);
        printf(" inst=%u%%", (inst_executed * 100) / cycles);
        printf(" miss=%u%%", (icache_miss * 100) / cycles);
        printf(" istl=%u%%", (icache_fill_stall * 100) / cycles);
        printf(" phv=%u%%", (phv_executed * 100) / cycles);
        printf(" hzrd=%u%%", (hazard_stall * 100) / cycles);
        printf(" phvwr_stl=%u%%", (phvwr_stall * 100) / cycles);
        printf(" memwr_stl=%u%%", (memwr_stall * 100) / cycles);
        printf(" tblwr_stl=%u%%", (tblwr_stall * 100) / cycles);
        printf(" fence_stl=%u%%\n", (fence_stall * 100) / cycles);
    }
}

void
mpu_reset_counters(int verbose, uint8_t pipeline, uint8_t stage, uint8_t mpu)
{
    uint64_t base = get_sdp_base(pipeline, stage);
    uint32_t mpu_offset = mpu * 4;
    uint32_t zero = 0;

    pal_reg_wr32w(mpu_offset + base + CAP_MPU_CSR_CNT_INST_EXECUTED_BYTE_OFFSET, &zero, 1);
    pal_reg_wr32w(mpu_offset + base + CAP_MPU_CSR_CNT_ICACHE_MISS_BYTE_OFFSET, &zero, 1);
    pal_reg_wr32w(mpu_offset + base + CAP_MPU_CSR_CNT_ICACHE_FILL_STALL_BYTE_OFFSET, &zero, 1);
    pal_reg_wr32w(mpu_offset + base + CAP_MPU_CSR_CNT_CYCLES_BYTE_OFFSET, &zero, 1);
    pal_reg_wr32w(mpu_offset + base + CAP_MPU_CSR_CNT_PHV_EXECUTED_BYTE_OFFSET, &zero, 1);
    pal_reg_wr32w(mpu_offset + base + CAP_MPU_CSR_CNT_HAZARD_STALL_BYTE_OFFSET, &zero, 1);
    pal_reg_wr32w(mpu_offset + base + CAP_MPU_CSR_CNT_PHVWR_STALL_BYTE_OFFSET, &zero, 1);
    pal_reg_wr32w(mpu_offset + base + CAP_MPU_CSR_CNT_MEMWR_STALL_BYTE_OFFSET, &zero, 1);
    pal_reg_wr32w(mpu_offset + base + CAP_MPU_CSR_CNT_TBLWR_STALL_BYTE_OFFSET, &zero, 1);
    pal_reg_wr32w(mpu_offset + base + CAP_MPU_CSR_CNT_FENCE_STALL_BYTE_OFFSET, &zero, 1);
}

void
mpu_read_table_addr(int verbose, uint8_t pipeline, uint8_t stage, uint8_t mpu)
{
    uint64_t base = get_sdp_base(pipeline, stage);
    uint32_t mpu_offset = mpu * 4;
    uint32_t table_addr[2];
    uint64_t addr;

    if (verbose) {
        pal_reg_rd32w(base + CAP_MPU_CSR_STA_TBL_ADDR_BYTE_OFFSET +
                          (CAP_MPU_CSR_STA_TBL_ADDR_ARRAY_ELEMENT_SIZE * mpu_offset),
                      table_addr, 2);
        addr =
            ((uint64_t)CAP_MPU_CSR_STA_TBL_ADDR_STA_TBL_ADDR_0_2_IN_MPU_31_0_GET(table_addr[0]) +
             ((uint64_t)CAP_MPU_CSR_STA_TBL_ADDR_STA_TBL_ADDR_1_2_IN_MPU_63_32_GET(table_addr[1])
              << 32));
        printf("  mpu %u table address = 0x%lx\n", mpu, addr);
    }
}

void
stg_poll(int verbose, uint8_t pipeline, uint8_t stage)
{
    uint64_t base = get_sdp_base(pipeline, stage);
    uint32_t sta_stg;
    uint32_t sta_ctl[4][2];
    uint32_t latency[4];
    uint32_t latency_val[4];
    uint32_t table_type[4];
    uint32_t phv_data_depth[4], phv_cmd_depth[4];
    uint32_t sta_pend[4];
    uint32_t tblwr_valid[4];
    uint32_t latency_min = 10000;
    uint32_t latency_max = 0;
    uint32_t latency_total = 0;
    uint32_t last_table_type = 0;
    uint32_t mpu_processing[4], stall[4][4], mpu, te_valid = 0, j;
    uint32_t srdy_in = 0, drdy_in = 0, idle_in = 0, util_in = 0, xoff_in = 0;
    uint32_t srdy = 0, drdy = 0, idle = 0, util = 0, xoff = 0;
    uint32_t srdy_out = 0, drdy_out = 0, idle_out = 0, util_out = 0, xoff_out = 0;
    int i, polls = 100;

    for (mpu = 0; mpu < 4; mpu++) {
        mpu_processing[mpu] = 0;
        stall[mpu][0] = 0;
        stall[mpu][1] = 0;
        stall[mpu][2] = 0;
        stall[mpu][3] = 0;
        latency[mpu] = 0;
        latency_val[mpu] = 0;
        table_type[mpu] = 0;
        phv_data_depth[mpu] = 0;
        phv_cmd_depth[mpu] = 0;
    }

    for (i = 0; i < polls; i++) {
        // Read per-Stage registers
        pal_reg_rd32w(base + CAP_MPU_CSR_STA_STG_BYTE_OFFSET, &sta_stg, 1);

        // Read per-MPU registers
        for (mpu = 0; mpu < 4; mpu++) {
            pal_reg_rd32w(base + CAP_MPU_CSR_STA_CTL_MPU_STA_CTL_MPU_0_2_BYTE_OFFSET +
                              mpu * CAP_MPU_CSR_STA_CTL_MPU_BYTE_SIZE,
                          sta_ctl[mpu], 2);
            pal_reg_rd32w(base + (CAP_MPU_CSR_STA_TABLE_ARRAY_ELEMENT_SIZE * 4 * mpu) +
                              CAP_MPU_CSR_STA_TABLE_BYTE_OFFSET,
                          &latency[mpu], 1);
            pal_reg_rd32w(base + (CAP_MPU_CSR_STA_TABLE_ARRAY_ELEMENT_SIZE * 4 * mpu) +
                              CAP_MPU_CSR_STA_PEND_BYTE_OFFSET,
                          &sta_pend[mpu], 1);
        }

        // Process per-MPU registers
        for (mpu = 0; mpu < 4; mpu++) {
            //        CAP_MPU_CSR_STA_CTL_MPU_STA_CTL_MPU_1_2_EX_PC_30_29_GET(sta_ctl[mpu][1])
            //        CAP_MPU_CSR_STA_CTL_MPU_STA_CTL_MPU_0_2_EX_PC_28_0_GET(sta_ctl[mpu][0])
            phv_data_depth[mpu] +=
                CAP_MPU_CSR_STA_CTL_MPU_STA_CTL_MPU_1_2_PHV_DATA_FIFO_DEPTH_GET(sta_ctl[mpu][1]);
            phv_cmd_depth[mpu] +=
                CAP_MPU_CSR_STA_CTL_MPU_STA_CTL_MPU_1_2_PHV_CMD_FIFO_DEPTH_GET(sta_ctl[mpu][1]);
            stall[mpu][0] +=
                CAP_MPU_CSR_STA_CTL_MPU_STA_CTL_MPU_1_2_STALL_VECTOR_GET(sta_ctl[mpu][1]) & 0x1;
            stall[mpu][1] +=
                (CAP_MPU_CSR_STA_CTL_MPU_STA_CTL_MPU_1_2_STALL_VECTOR_GET(sta_ctl[mpu][1]) >> 1) &
                0x1;
            stall[mpu][2] +=
                (CAP_MPU_CSR_STA_CTL_MPU_STA_CTL_MPU_1_2_STALL_VECTOR_GET(sta_ctl[mpu][1]) >> 2) &
                0x1;
            stall[mpu][3] +=
                (CAP_MPU_CSR_STA_CTL_MPU_STA_CTL_MPU_1_2_STALL_VECTOR_GET(sta_ctl[mpu][1]) >> 3) &
                0x1;
            mpu_processing[mpu] += (CAP_MPU_CSR_STA_STG_MPU_PROCESSING_GET(sta_stg) >> mpu) & 0x1;
            latency_val[mpu] =
                CAP_MPU_CSR_STA_TABLE_MPU_PROCESSING_TABLE_LATENCY_GET(latency[mpu]);
            table_type[mpu] =
                CAP_MPU_CSR_STA_TABLE_MPU_PROCESSING_TABLE_PCIE_GET(latency[mpu])
                    ? TABLE_PCI
                    : CAP_MPU_CSR_STA_TABLE_MPU_PROCESSING_TABLE_SRAM_GET(latency[mpu])
                          ? TABLE_SRAM
                          : TABLE_HBM;
            tblwr_valid[mpu] +=
                CAP_MPU_CSR_STA_PEND_STA_PEND_1_2_PENDING_TABLE_WRITE_VALID0_GET(sta_pend[mpu]) +
                CAP_MPU_CSR_STA_PEND_STA_PEND_1_2_PENDING_TABLE_WRITE_VALID1_GET(sta_pend[mpu]);
        }

        // Only use MPU[3] for latency as it is used first in distribution
        latency_total += latency_val[3];
        last_table_type = table_type[3];
        if (latency_val[3] > latency_max)
            latency_max = latency_val[3];
        if (latency_val[3] < latency_min)
            latency_min = latency_val[3];

        srdy = CAP_MPU_CSR_STA_STG_SRDY_GET(sta_stg);
        drdy = CAP_MPU_CSR_STA_STG_DRDY_GET(sta_stg);
        srdy_in = CAP_MPU_CSR_STA_STG_SDP_SRDY_IN_GET(sta_stg);
        drdy_in = CAP_MPU_CSR_STA_STG_SDP_DRDY_IN_GET(sta_stg);
        srdy_out = CAP_MPU_CSR_STA_STG_SRDY_OUT_GET(sta_stg);
        drdy_out = CAP_MPU_CSR_STA_STG_DRDY_OUT_GET(sta_stg);

        if ((!srdy && !drdy) || (!srdy && drdy)) {
            idle++;
        } else if (srdy && !drdy) {
            xoff++;
        } else if (srdy && drdy) {
            util++;
        }

        if ((!srdy_in && !drdy_in) || (!srdy_in && drdy_in)) {
            idle_in++;
        } else if (srdy_in && !drdy_in) {
            xoff_in++;
        } else if (srdy_in && drdy_in) {
            util_in++;
        }

        if ((!srdy_out && !drdy_out) || (!srdy_out && drdy_out)) {
            idle_out++;
        } else if (srdy_out && !drdy_out) {
            xoff_out++;
        } else if (srdy_out && drdy_out) {
            util_out++;
        }

        // count all te_valid pending
        for (j = 0; j < 16; j++) {
            te_valid += (((CAP_MPU_CSR_STA_STG_TE_VALID_GET(sta_stg) >> j) & 1) == 1) ? 1 : 0;
        }
    }

    printf(" (util/xoff/idle) in=%3d/%3d/%3d stg=%3d/%3d/%3d out=%3d/%3d/%3d TE=%2u",
           (util_in * 100) / polls, (xoff_in * 100) / polls, (idle_in * 100) / polls,
           (util * 100) / polls, (xoff * 100) / polls, (idle * 100) / polls,
           (util_out * 100) / polls, (xoff_out * 100) / polls, (idle_out * 100) / polls,
           te_valid / polls);

    if (last_table_type == TABLE_PCI)
        printf(" PCI_lat=");
    else if (last_table_type == TABLE_SRAM)
        printf(" SRM_lat=");
    else
        printf(" HBM_lat=");
    printf("%5u", latency_total / polls);

    if (verbose) {
        printf(" min=%u, max=%u\n", (int)latency_min, (int)latency_max);
        printf(" phvwr depths");
        for (mpu = 0; mpu < 4; mpu++) {
            printf(" m%u=%u,%u", mpu, (int)(phv_cmd_depth[mpu] / polls),
                   (int)(phv_data_depth[mpu] / polls));
        }
        printf("\n");
    }

    for (mpu = 0; mpu < 4; mpu++) {
        if (verbose) {
            printf(
                "  mpu %u  processing %2d%%, stalls: hazard %2d%% phvwr %2d%% tblwr %2d%% memwr "
                "%2d%%\n",
                mpu, mpu_processing[mpu] * 100 / polls, stall[mpu][3] * 100 / polls,
                stall[mpu][2] * 100 / polls, stall[mpu][1] * 100 / polls,
                stall[mpu][0] * 100 / polls);
        } else {
            printf(" m%1d=%3d%%", mpu, mpu_processing[mpu] * 100 / polls);
        }
    }

    if (!verbose) {
        printf("\n");
    }
}
