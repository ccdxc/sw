//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file contains the implementation for fetching and storing info
/// from each stage
///
//===----------------------------------------------------------------------===//

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "dtls.hpp"

#include "elb_mpu_c_hdr.h"
#include "elb_te_c_hdr.h"
#include "elb_top_csr_defines.h"
#include "elbmon.hpp"

#include "nic/sdk/platform/pal/include/pal.h"

uint64_t pipe_base[PIPE_CNT] = {[TXDMA] = ELB_ADDR_BASE_PCT_MPU_0_OFFSET,
                                [RXDMA] = ELB_ADDR_BASE_PCR_MPU_0_OFFSET,
                                [P4IG] = ELB_ADDR_BASE_SGI_MPU_0_OFFSET,
                                [P4EG] = ELB_ADDR_BASE_SGE_MPU_0_OFFSET};

uint64_t pipe_sz[PIPE_CNT] = {
    [TXDMA] = ELB_ADDR_BASE_PCT_MPU_1_OFFSET - ELB_ADDR_BASE_PCT_MPU_0_OFFSET,
    [RXDMA] = ELB_ADDR_BASE_PCR_MPU_1_OFFSET - ELB_ADDR_BASE_PCR_MPU_0_OFFSET,
    [P4IG] = ELB_ADDR_BASE_SGI_MPU_1_OFFSET - ELB_ADDR_BASE_SGI_MPU_0_OFFSET,
    [P4EG] = ELB_ADDR_BASE_SGE_MPU_1_OFFSET - ELB_ADDR_BASE_SGE_MPU_0_OFFSET,
};

uint64_t te_base[PIPE_CNT] = {[TXDMA] = ELB_ADDR_BASE_PCT_TE_0_OFFSET,
                              [RXDMA] = ELB_ADDR_BASE_PCR_TE_0_OFFSET,
                              [P4IG] = ELB_ADDR_BASE_SGI_TE_0_OFFSET,
                              [P4EG] = ELB_ADDR_BASE_SGE_TE_0_OFFSET};

uint64_t te_sz[PIPE_CNT] = {
    [TXDMA] = ELB_ADDR_BASE_PCT_TE_1_OFFSET - ELB_ADDR_BASE_PCT_TE_0_OFFSET,
    [RXDMA] = ELB_ADDR_BASE_PCR_TE_1_OFFSET - ELB_ADDR_BASE_PCR_TE_0_OFFSET,
    [P4IG] = ELB_ADDR_BASE_SGI_TE_1_OFFSET - ELB_ADDR_BASE_SGI_TE_0_OFFSET,
    [P4EG] = ELB_ADDR_BASE_SGE_TE_1_OFFSET - ELB_ADDR_BASE_SGE_TE_0_OFFSET};

uint64_t stage_cnt[PIPE_CNT] = {
    [TXDMA] = 8, [RXDMA] = 8, [P4IG] = 6, [P4EG] = 6};

uint64_t
get_sdp_base (uint8_t pipeline, uint8_t stage)
{
    assert(pipeline < PIPE_CNT);
    assert(stage < stage_cnt[pipeline]);

    return pipe_base[pipeline] + (pipe_sz[pipeline] * stage);
}

void
sdp_read_counters (int verbose, uint8_t pipeline, uint8_t stage)
{
    uint64_t base = get_sdp_base(pipeline, stage);
    uint32_t sdp[3]; /* only 3, ELB_MCPU_CSR_CNT_SDP_SIZE=4 looks wrong? */
    int sop_out = 0, phv_fifo_depth = 0;
    stage_t *stage_ptr = NULL;

    if (verbose) {
        pal_reg_rd32w(base + ELB_MPU_CSR_CNT_SDP_BYTE_OFFSET, sdp, 3);

        // extract from sdp[1]
        sop_out = ELB_MPU_CSR_CNT_SDP_CNT_SDP_1_3_SOP_OUT_GET(sdp[1]);
        // extract from sdp[0]
        phv_fifo_depth =
            ELB_MPU_CSR_CNT_SDP_CNT_SDP_0_3_PHV_FIFO_DEPTH_GET(sdp[0]);
    }
    stage_ptr = &(asic->pipelines[pipeline].stages[stage]);
    stage_ptr->phv_fifo_depth = phv_fifo_depth;
    stage_ptr->phv_processed_count = sop_out;
}

void
sdp_reset_counters (int verbose, uint8_t pipeline, uint8_t stage)
{
    uint64_t base = get_sdp_base(pipeline, stage);
    uint32_t zero[3] = {0};

    pal_reg_wr32w(base + ELB_MPU_CSR_CNT_SDP_BYTE_OFFSET, zero, 3);
}

uint64_t
get_te_base (uint8_t pipeline, uint8_t stage)
{
    assert(pipeline < PIPE_CNT);
    assert(stage < stage_cnt[pipeline]);

    return te_base[pipeline] + (te_sz[pipeline] * stage);
}

void
te_read_counters (int verbose, uint8_t pipeline, uint8_t stage)
{
    uint64_t base = get_te_base(pipeline, stage);
    uint32_t te_phv_cnt;
    uint32_t te_axi_cnt;
    uint32_t te_tcam_cnt;
    uint32_t te_mpu_cnt;

    if (verbose) {
        pal_reg_rd32w(base + ELB_TE_CSR_CNT_PHV_IN_SOP_BYTE_ADDRESS,
                      &te_phv_cnt, 1);
        pal_reg_rd32w(base + ELB_TE_CSR_CNT_AXI_RDREQ_BYTE_ADDRESS, &te_axi_cnt,
                      1);
        pal_reg_rd32w(base + ELB_TE_CSR_CNT_TCAM_REQ_BYTE_ADDRESS, &te_tcam_cnt,
                      1);
        pal_reg_rd32w(base + ELB_TE_CSR_CNT_MPU_OUT_BYTE_ADDRESS, &te_mpu_cnt,
                      1);
    }
    stage_t *stage_ptr = NULL;
    stage_ptr = &(asic->pipelines[pipeline].stages[stage]);
    stage_ptr->te_phv_cnt = te_phv_cnt;
    stage_ptr->te_axi_cnt = te_axi_cnt;
    stage_ptr->te_tcam_cnt = te_tcam_cnt;
    stage_ptr->te_mpu_cnt = te_mpu_cnt;
}

void
te_reset_counters (int verbose, uint8_t pipeline, uint8_t stage)
{
    uint64_t base = get_te_base(pipeline, stage);
    uint32_t zero = 0;

    if (verbose) {
        pal_reg_wr32w(base + ELB_TE_CSR_CNT_PHV_IN_SOP_BYTE_ADDRESS, &zero, 1);
        pal_reg_wr32w(base + ELB_TE_CSR_CNT_AXI_RDREQ_BYTE_ADDRESS, &zero, 1);
        pal_reg_wr32w(base + ELB_TE_CSR_CNT_TCAM_REQ_BYTE_ADDRESS, &zero, 1);
        pal_reg_wr32w(base + ELB_TE_CSR_CNT_MPU_OUT_BYTE_ADDRESS, &zero, 1);
    }
}

void
mpu_read_counters (int verbose, uint8_t pipeline, uint8_t stage, uint8_t mpu)
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
    mpu_t *mpu_ptr = &(asic->pipelines[pipeline].stages[stage].mpus[mpu]);

    if (verbose) {
        pal_reg_rd32w(mpu_offset + base +
                          ELB_MPU_CSR_CNT_INST_EXECUTED_BYTE_OFFSET,
                      &inst_executed, 1);
        pal_reg_rd32w(mpu_offset + base +
                          ELB_MPU_CSR_CNT_ICACHE_MISS_BYTE_OFFSET,
                      &icache_miss, 1);
        pal_reg_rd32w(mpu_offset + base +
                          ELB_MPU_CSR_CNT_ICACHE_FILL_STALL_BYTE_OFFSET,
                      &icache_fill_stall, 1);
        pal_reg_rd32w(mpu_offset + base + ELB_MPU_CSR_CNT_CYCLES_BYTE_OFFSET,
                      &cycles, 1);
        pal_reg_rd32w(mpu_offset + base +
                          ELB_MPU_CSR_CNT_PHV_EXECUTED_BYTE_OFFSET,
                      &phv_executed, 1);
        pal_reg_rd32w(mpu_offset + base +
                          ELB_MPU_CSR_CNT_HAZARD_STALL_BYTE_OFFSET,
                      &hazard_stall, 1);
        pal_reg_rd32w(mpu_offset + base +
                          ELB_MPU_CSR_CNT_PHVWR_STALL_BYTE_OFFSET,
                      &phvwr_stall, 1);
        pal_reg_rd32w(mpu_offset + base +
                          ELB_MPU_CSR_CNT_MEMWR_STALL_BYTE_OFFSET,
                      &memwr_stall, 1);
        pal_reg_rd32w(mpu_offset + base +
                          ELB_MPU_CSR_CNT_TBLWR_STALL_BYTE_OFFSET,
                      &tblwr_stall, 1);
        pal_reg_rd32w(mpu_offset + base +
                          ELB_MPU_CSR_CNT_FENCE_STALL_BYTE_OFFSET,
                      &fence_stall, 1);
        mpu_ptr->cycles = cycles;

        mpu_ptr->inst_executed = inst_executed;
        mpu_ptr->icache_miss = icache_miss;
        mpu_ptr->icache_fill_stall = icache_fill_stall;
        mpu_ptr->phv_executed = phv_executed;
        mpu_ptr->hazard_stall = hazard_stall;
        mpu_ptr->phvwr_stall = phvwr_stall;
        mpu_ptr->memwr_stall = memwr_stall;
        mpu_ptr->tblwr_stall = tblwr_stall;
        mpu_ptr->fence_stall = fence_stall;

        mpu_ptr->inst_executed_pc = inst_executed * 100 / cycles;
        mpu_ptr->icache_miss_pc = icache_miss * 100 / cycles;
        mpu_ptr->icache_fill_stall_pc = icache_fill_stall * 100 / cycles;
        mpu_ptr->phv_executed_pc = phv_executed * 100 / cycles;
        mpu_ptr->hazard_stall_pc = hazard_stall * 100 / cycles;
        mpu_ptr->phvwr_stall_pc = phvwr_stall * 100 / cycles;
        mpu_ptr->memwr_stall_pc = memwr_stall * 100 / cycles;
        mpu_ptr->tblwr_stall_pc = tblwr_stall * 100 / cycles;
        mpu_ptr->fence_stall_pc = fence_stall * 100 / cycles;
    }
}

void
mpu_reset_counters (int verbose, uint8_t pipeline, uint8_t stage, uint8_t mpu)
{
    uint64_t base = get_sdp_base(pipeline, stage);
    uint32_t mpu_offset = mpu * 4;
    uint32_t zero = 0;

    pal_reg_wr32w(mpu_offset + base + ELB_MPU_CSR_CNT_INST_EXECUTED_BYTE_OFFSET,
                  &zero, 1);
    pal_reg_wr32w(mpu_offset + base + ELB_MPU_CSR_CNT_ICACHE_MISS_BYTE_OFFSET,
                  &zero, 1);
    pal_reg_wr32w(mpu_offset + base +
                      ELB_MPU_CSR_CNT_ICACHE_FILL_STALL_BYTE_OFFSET,
                  &zero, 1);
    pal_reg_wr32w(mpu_offset + base + ELB_MPU_CSR_CNT_CYCLES_BYTE_OFFSET, &zero,
                  1);
    pal_reg_wr32w(mpu_offset + base + ELB_MPU_CSR_CNT_PHV_EXECUTED_BYTE_OFFSET,
                  &zero, 1);
    pal_reg_wr32w(mpu_offset + base + ELB_MPU_CSR_CNT_HAZARD_STALL_BYTE_OFFSET,
                  &zero, 1);
    pal_reg_wr32w(mpu_offset + base + ELB_MPU_CSR_CNT_PHVWR_STALL_BYTE_OFFSET,
                  &zero, 1);
    pal_reg_wr32w(mpu_offset + base + ELB_MPU_CSR_CNT_MEMWR_STALL_BYTE_OFFSET,
                  &zero, 1);
    pal_reg_wr32w(mpu_offset + base + ELB_MPU_CSR_CNT_TBLWR_STALL_BYTE_OFFSET,
                  &zero, 1);
    pal_reg_wr32w(mpu_offset + base + ELB_MPU_CSR_CNT_FENCE_STALL_BYTE_OFFSET,
                  &zero, 1);
}

void
mpu_read_table_addr (int verbose, uint8_t pipeline, uint8_t stage, uint8_t mpu)
{
    uint64_t base = get_sdp_base(pipeline, stage);
    uint32_t mpu_offset = mpu * 4;
    uint32_t table_addr[2];
    uint64_t addr;
    mpu_t *mpu_ptr = NULL;

    if (verbose) {
        pal_reg_rd32w(
            base + ELB_MPU_CSR_STA_TBL_ADDR_BYTE_OFFSET +
                (ELB_MPU_CSR_STA_TBL_ADDR_ARRAY_ELEMENT_SIZE * mpu_offset),
            table_addr, 2);
        addr = ((uint64_t)
                    ELB_MPU_CSR_STA_TBL_ADDR_STA_TBL_ADDR_0_2_IN_MPU_31_0_GET(
                        table_addr[0]) +
                ((uint64_t)
                     ELB_MPU_CSR_STA_TBL_ADDR_STA_TBL_ADDR_1_2_IN_MPU_63_32_GET(
                         table_addr[1])
                 << 32));
        mpu_ptr = &asic->pipelines[pipeline].stages[stage].mpus[mpu];
        mpu_ptr->addr = addr;
    }
}

void
stg_poll (int verbose, uint8_t pipeline, uint8_t stage)
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
    uint32_t srdy_out = 0, drdy_out = 0, idle_out = 0, util_out = 0,
             xoff_out = 0;
    int i, polls = 100;
    mpu_t *mpu_ptr = NULL;

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
        pal_reg_rd32w(base + ELB_MPU_CSR_STA_STG_BYTE_OFFSET, &sta_stg, 1);

        // Read per-MPU registers
        for (mpu = 0; mpu < 4; mpu++) {
            pal_reg_rd32w(
                base + ELB_MPU_CSR_STA_CTL_MPU_STA_CTL_MPU_0_2_BYTE_OFFSET +
                    mpu * ELB_MPU_CSR_STA_CTL_MPU_BYTE_SIZE,
                sta_ctl[mpu], 2);
            pal_reg_rd32w(
                base + (ELB_MPU_CSR_STA_TABLE_ARRAY_ELEMENT_SIZE * 4 * mpu) +
                    ELB_MPU_CSR_STA_TABLE_BYTE_OFFSET,
                &latency[mpu], 1);
            pal_reg_rd32w(
                base + (ELB_MPU_CSR_STA_TABLE_ARRAY_ELEMENT_SIZE * 4 * mpu) +
                    ELB_MPU_CSR_STA_PEND_BYTE_OFFSET,
                &sta_pend[mpu], 1);
        }

        // Process per-MPU registers
        for (mpu = 0; mpu < 4; mpu++) {
            //        ELB_MPU_CSR_STA_CTL_MPU_STA_CTL_MPU_1_2_EX_PC_30_29_GET(sta_ctl[mpu][1])
            //        ELB_MPU_CSR_STA_CTL_MPU_STA_CTL_MPU_0_2_EX_PC_28_0_GET(sta_ctl[mpu][0])
            phv_data_depth[mpu] +=
                ELB_MPU_CSR_STA_CTL_MPU_STA_CTL_MPU_1_2_PHV_DATA_FIFO_DEPTH_GET(
                    sta_ctl[mpu][1]);
            phv_cmd_depth[mpu] +=
                ELB_MPU_CSR_STA_CTL_MPU_STA_CTL_MPU_1_2_PHV_CMD_FIFO_DEPTH_GET(
                    sta_ctl[mpu][1]);
            stall[mpu][0] +=
                ELB_MPU_CSR_STA_CTL_MPU_STA_CTL_MPU_1_2_STALL_VECTOR_GET(
                    sta_ctl[mpu][1]) &
                0x1;
            stall[mpu][1] +=
                (ELB_MPU_CSR_STA_CTL_MPU_STA_CTL_MPU_1_2_STALL_VECTOR_GET(
                     sta_ctl[mpu][1]) >>
                 1) &
                0x1;
            stall[mpu][2] +=
                (ELB_MPU_CSR_STA_CTL_MPU_STA_CTL_MPU_1_2_STALL_VECTOR_GET(
                     sta_ctl[mpu][1]) >>
                 2) &
                0x1;
            stall[mpu][3] +=
                (ELB_MPU_CSR_STA_CTL_MPU_STA_CTL_MPU_1_2_STALL_VECTOR_GET(
                     sta_ctl[mpu][1]) >>
                 3) &
                0x1;
            mpu_processing[mpu] +=
                (ELB_MPU_CSR_STA_STG_MPU_PROCESSING_GET(sta_stg) >> mpu) & 0x1;
            latency_val[mpu] =
                ELB_MPU_CSR_STA_TABLE_MPU_PROCESSING_TABLE_LATENCY_GET(
                    latency[mpu]);
            table_type[mpu] =
                ELB_MPU_CSR_STA_TABLE_MPU_PROCESSING_TABLE_PCIE_GET(
                    latency[mpu])
                    ? TABLE_PCI
                    : ELB_MPU_CSR_STA_TABLE_MPU_PROCESSING_TABLE_SRAM_GET(
                          latency[mpu])
                          ? TABLE_SRAM
                          : TABLE_HBM;
            tblwr_valid[mpu] +=
                ELB_MPU_CSR_STA_PEND_STA_PEND_1_2_PENDING_TABLE_WRITE_VALID0_GET(
                    sta_pend[mpu]) +
                ELB_MPU_CSR_STA_PEND_STA_PEND_1_2_PENDING_TABLE_WRITE_VALID1_GET(
                    sta_pend[mpu]);
        }

        // Only use MPU[3] for latency as it is used first in distribution
        latency_total += latency_val[3];
        last_table_type = table_type[3];
        if (latency_val[3] > latency_max)
            latency_max = latency_val[3];
        if (latency_val[3] < latency_min)
            latency_min = latency_val[3];

        srdy = ELB_MPU_CSR_STA_STG_SRDY_GET(sta_stg);
        drdy = ELB_MPU_CSR_STA_STG_DRDY_GET(sta_stg);
        srdy_in = ELB_MPU_CSR_STA_STG_SDP_SRDY_IN_GET(sta_stg);
        drdy_in = ELB_MPU_CSR_STA_STG_SDP_DRDY_IN_GET(sta_stg);
        srdy_out = ELB_MPU_CSR_STA_STG_SRDY_OUT_GET(sta_stg);
        drdy_out = ELB_MPU_CSR_STA_STG_DRDY_OUT_GET(sta_stg);

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
            te_valid +=
                (((ELB_MPU_CSR_STA_STG_TE_VALID_GET(sta_stg) >> j) & 1) == 1)
                    ? 1
                    : 0;
        }
    }

    stage_t *stage_ptr = NULL;
    stage_ptr = &(asic->pipelines[pipeline].stages[stage]);
    stage_ptr->util.in = (util_in * 100) / polls;
    stage_ptr->util.stg = (util * 100) / polls;
    stage_ptr->util.out = (util_out * 100) / polls;

    stage_ptr->idle.in = (idle_in * 100) / polls;
    stage_ptr->idle.stg = (idle * 100) / polls;
    stage_ptr->idle.out = (idle_out * 100) / polls;

    stage_ptr->xoff.in = (xoff_in * 100) / polls;
    stage_ptr->xoff.stg = (xoff * 100) / polls;
    stage_ptr->xoff.out = (xoff_out * 100) / polls;
    stage_ptr->last_table_type = last_table_type;
    stage_ptr->_lat = latency_total / polls;
    stage_ptr->te = te_valid / polls;
    stage_ptr->min = latency_min;
    stage_ptr->max = latency_max;
    for (mpu = 0; mpu < 4; mpu++) {
        stage_ptr->m[mpu] = phv_data_depth[mpu] / polls;
    }

    for (mpu = 0; mpu < 4; mpu++) {
        mpu_ptr = &asic->pipelines[pipeline].stages[stage].mpus[mpu];
        mpu_ptr->processing_pc = mpu_processing[mpu] * 100 / polls;
        mpu_ptr->stall[0] = stall[mpu][0];
        mpu_ptr->stall[1] = stall[mpu][1];
        mpu_ptr->stall[2] = stall[mpu][2];
        mpu_ptr->stall[3] = stall[mpu][3];
    }
}

