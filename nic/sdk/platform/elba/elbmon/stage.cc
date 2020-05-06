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

#include "elb_stg_c_hdr.h"
//#include "elb_mpu_c_hdr.h"
//#include "elb_sdp_c_hdr.h"
#include "elb_te_c_hdr.h"
#include "elb_top_csr_defines.h"
#include "elbmon.hpp"

#include "nic/sdk/platform/pal/include/pal.h"

	// TE FSM states
#define TE_FSM_IDLE 0
#define TE_FSM_NEW_ENTRY 1
#define TE_FSM_QUEUED_AXI_REQ0 2
#define TE_FSM_ISSUED_AXI_REQ0 3
#define TE_FSM_QUEUED_AXI_REQ1 4
#define TE_FSM_ISSUED_AXI_REQ1 5
#define TE_FSM_QUEUED_TCAM_REQ 6
#define TE_FSM_ISSUED_TCAM_REQ 7

#define TE_NUM_PENDING_READS 28

uint64_t pipe_base[PIPE_CNT] = {[TXDMA] = ELB_ADDR_BASE_PCT_STG_0_OFFSET,
                                [RXDMA] = ELB_ADDR_BASE_PCR_STG_0_OFFSET,
                                [P4IG]  = ELB_ADDR_BASE_SGI_STG_0_OFFSET,
                                [P4EG]  = ELB_ADDR_BASE_SGE_STG_0_OFFSET,
                                [SXDMA] = ELB_ADDR_BASE_XG_STG_0_OFFSET};

uint64_t te_base[PIPE_CNT] = {[TXDMA] = ELB_ADDR_BASE_PCT_TE_0_OFFSET,
			      [RXDMA] = ELB_ADDR_BASE_PCR_TE_0_OFFSET,
			      [P4IG]  = ELB_ADDR_BASE_SGI_TE_0_OFFSET,
			      [P4EG]  = ELB_ADDR_BASE_SGE_TE_0_OFFSET,
			      [SXDMA] = ELB_ADDR_BASE_XG_TE_0_OFFSET};

uint64_t pipe_sz[PIPE_CNT] = {
    [TXDMA] = ELB_ADDR_BASE_PCT_STG_1_OFFSET - ELB_ADDR_BASE_PCT_STG_0_OFFSET,
    [RXDMA] = ELB_ADDR_BASE_PCR_STG_1_OFFSET - ELB_ADDR_BASE_PCR_STG_0_OFFSET,
    [P4IG]  = ELB_ADDR_BASE_SGI_STG_1_OFFSET - ELB_ADDR_BASE_SGI_STG_0_OFFSET,
    [P4EG]  = ELB_ADDR_BASE_SGE_STG_1_OFFSET - ELB_ADDR_BASE_SGE_STG_0_OFFSET,
    [SXDMA] = ELB_ADDR_BASE_XG_STG_1_OFFSET  - ELB_ADDR_BASE_XG_STG_0_OFFSET};

uint64_t te_sz[PIPE_CNT] = {
    [TXDMA] = ELB_ADDR_BASE_PCT_TE_1_OFFSET - ELB_ADDR_BASE_PCT_TE_0_OFFSET,
    [RXDMA] = ELB_ADDR_BASE_PCR_TE_1_OFFSET - ELB_ADDR_BASE_PCR_TE_0_OFFSET,
    [P4IG]  = ELB_ADDR_BASE_SGI_TE_1_OFFSET - ELB_ADDR_BASE_SGI_TE_0_OFFSET,
    [P4EG]  = ELB_ADDR_BASE_SGE_TE_1_OFFSET - ELB_ADDR_BASE_SGE_TE_0_OFFSET,
    [SXDMA] = ELB_ADDR_BASE_XG_TE_1_OFFSET  - ELB_ADDR_BASE_XG_TE_0_OFFSET};

uint64_t stage_cnt[PIPE_CNT] = {
  [TXDMA] = 8, [RXDMA] = 8, [P4IG] = 8, [P4EG] = 8, [SXDMA] = 4};

uint64_t
get_sdp_base (uint8_t pipeline, uint8_t stage)
{
    assert(pipeline < PIPE_CNT);
    assert(stage < stage_cnt[pipeline]);

    return pipe_base[pipeline] + (pipe_sz[pipeline] * stage) + ELB_STG_CSR_SDP_BYTE_ADDRESS;
}

uint64_t
get_mpu_base (uint8_t pipeline, uint8_t stage, uint8_t mpu)
{
    assert(pipeline < PIPE_CNT);
    assert(stage < stage_cnt[pipeline]);
    assert(mpu < 4);

    return pipe_base[pipeline] + (pipe_sz[pipeline] * stage) + ELB_STG_CSR_MPU_BYTE_ADDRESS +
      (ELB_STG_CSR_MPU_ARRAY_ELEMENT_SIZE * 4 * mpu); // in bytes
}

uint64_t
get_te_base (uint8_t pipeline, uint8_t stage)
{
    assert(pipeline < PIPE_CNT);
    assert(stage < stage_cnt[pipeline]);

    return te_base[pipeline] + (te_sz[pipeline] * stage);
}

uint64_t
get_stg_base (uint8_t pipeline, uint8_t stage)
{
    assert(pipeline < PIPE_CNT);
    assert(stage < stage_cnt[pipeline]);

    return pipe_base[pipeline] + (pipe_sz[pipeline] * stage);
}

void
sdp_read_counters (int verbose, uint8_t pipeline, uint8_t stage)
{
    uint64_t sdp_base = get_sdp_base(pipeline, stage);
    uint32_t sdp[6]; /* 6 for Elba, 3 for Capri, ELB_MCPU_CSR_CNT_SDP_SIZE=4 looks wrong? */
    uint32_t sdp_fifo, read_ptr, write_ptr;
    int phv_fifo_depth = 0;
    // int sop_in = 0;
    // int eop_in = 0;
    // eop_out = 0;
    int srdy_nodrdy_in = 0;
    int sop_out = 0, srdy_nodrdy_out = 0;
    stage_t *stage_ptr = NULL;

    if (verbose) {
        pal_reg_rd32w(sdp_base + ELB_SDP_CSR_CNT_SDP_BYTE_OFFSET, sdp, 6);
	// sop_in          = ELB_SDP_CSR_CNT_SDP_CNT_SDP_0_6_SOP_IN_GET(sdp[0]);
        // eop_in          = ELB_SDP_CSR_CNT_SDP_CNT_SDP_1_6_EOP_IN_GET(sdp[1]);
	srdy_nodrdy_in  = ELB_SDP_CSR_CNT_SDP_CNT_SDP_2_6_SRDY_NO_DRDY_IN_GET(sdp[2]);
        sop_out         = ELB_SDP_CSR_CNT_SDP_CNT_SDP_3_6_SOP_OUT_GET(sdp[3]);
        // eop_out         = ELB_SDP_CSR_CNT_SDP_CNT_SDP_4_6_EOP_OUT_GET(sdp[4]);
	srdy_nodrdy_out = ELB_SDP_CSR_CNT_SDP_CNT_SDP_5_6_SRDY_NO_DRDY_OUT_GET(sdp[5]);

	// number of entries in this SDP:
        pal_reg_rd32w(sdp_base + ELB_SDP_CSR_STA_SDP_FIFO_BYTE_OFFSET, &sdp_fifo, 1);
        read_ptr = ELB_SDP_CSR_STA_SDP_FIFO_READ_POINTER_GET(sdp_fifo);
        write_ptr = ELB_SDP_CSR_STA_SDP_FIFO_WRITE_POINTER_GET(sdp_fifo);
	phv_fifo_depth = (write_ptr >= read_ptr) ? (write_ptr - read_ptr) : 
	                                           (32 - read_ptr + write_ptr);

    }
    stage_ptr = &(asic->pipelines[pipeline].stages[stage]);
    stage_ptr->phv_fifo_depth = phv_fifo_depth;
    stage_ptr->phv_processed_count = sop_out;
    stage_ptr->sdp_nodrdy_in  = srdy_nodrdy_in;
    stage_ptr->sdp_nodrdy_out = srdy_nodrdy_out;
}

void
sdp_reset_counters (int verbose, uint8_t pipeline, uint8_t stage)
{
    uint64_t sdp_base = get_sdp_base(pipeline, stage);
    uint32_t zero[6] = {0};

    pal_reg_wr32w(sdp_base + ELB_SDP_CSR_CNT_SDP_BYTE_OFFSET, zero, 6);
}

void
te_read_counters (int verbose, uint8_t pipeline, uint8_t stage)
{
    uint64_t te_base = get_te_base(pipeline, stage);
    uint32_t te_phv_cnt;
    uint32_t te_axi_cnt;
    uint32_t te_tcam_cnt;
    uint32_t te_mpu_cnt;

    if (verbose) {
        pal_reg_rd32w(te_base + ELB_TE_CSR_CNT_PHV_IN_SOP_BYTE_ADDRESS,
                      &te_phv_cnt, 1);
        pal_reg_rd32w(te_base + ELB_TE_CSR_CNT_AXI_RDREQ_BYTE_ADDRESS, &te_axi_cnt,
                      1);
        pal_reg_rd32w(te_base + ELB_TE_CSR_CNT_TCAM_REQ_BYTE_ADDRESS, &te_tcam_cnt,
                      1);
        pal_reg_rd32w(te_base + ELB_TE_CSR_CNT_MPU_OUT_BYTE_ADDRESS, &te_mpu_cnt,
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
    uint64_t te_base = get_te_base(pipeline, stage);
    uint32_t zero = 0;

    if (verbose) {
        pal_reg_wr32w(te_base + ELB_TE_CSR_CNT_PHV_IN_SOP_BYTE_ADDRESS, &zero, 1);
        pal_reg_wr32w(te_base + ELB_TE_CSR_CNT_AXI_RDREQ_BYTE_ADDRESS, &zero, 1);
        pal_reg_wr32w(te_base + ELB_TE_CSR_CNT_TCAM_REQ_BYTE_ADDRESS, &zero, 1);
        pal_reg_wr32w(te_base + ELB_TE_CSR_CNT_MPU_OUT_BYTE_ADDRESS, &zero, 1);
    }
}

void
mpu_read_counters (int verbose, uint8_t pipeline, uint8_t stage, uint8_t mpu)
{
  uint64_t mpu_base = get_mpu_base(pipeline, stage, mpu);
    uint32_t inst_executed;
    uint32_t icache_miss;
    uint32_t dcache_miss;
    uint32_t cycles;
    uint32_t phv_executed;
    uint32_t phvwr_stall;
    uint32_t st_stall;
    mpu_t *mpu_ptr = &(asic->pipelines[pipeline].stages[stage].mpus[mpu]);

    if (verbose) {
        pal_reg_rd32w( mpu_base +
                          ELB_MPU_CSR_CNT_INST_EXECUTED_BYTE_OFFSET,
                      &inst_executed, 1);
        pal_reg_rd32w( mpu_base +
                          ELB_MPU_CSR_CNT_ICACHE_MISS_BYTE_OFFSET,
                      &icache_miss, 1);
        pal_reg_rd32w( mpu_base +
                          ELB_MPU_CSR_CNT_DCACHE_MISS_BYTE_OFFSET,
                      &dcache_miss, 1);
        pal_reg_rd32w( mpu_base + ELB_MPU_CSR_CNT_CYCLES_BYTE_OFFSET,
                      &cycles, 1);
        pal_reg_rd32w( mpu_base +
                          ELB_MPU_CSR_CNT_PHV_EXECUTED_BYTE_OFFSET,
                      &phv_executed, 1);
        pal_reg_rd32w( mpu_base +
                          ELB_MPU_CSR_CNT_PHVWR_STALL_BYTE_OFFSET,
                      &phvwr_stall, 1);
        pal_reg_rd32w( mpu_base +
                          ELB_MPU_CSR_CNT_ST_STALL_BYTE_OFFSET,
                      &st_stall, 1);
        mpu_ptr->cycles = cycles;

        mpu_ptr->inst_executed = inst_executed;
        mpu_ptr->icache_miss = icache_miss;
        mpu_ptr->dcache_miss = dcache_miss;
        mpu_ptr->phv_executed = phv_executed;
        mpu_ptr->phvwr_stall = phvwr_stall;
        mpu_ptr->st_stall = st_stall;

	if(cycles==0) cycles=1;
	if(inst_executed==0) inst_executed=1;
	if(phv_executed==0) phv_executed=1;

        mpu_ptr->inst_executed_pc = inst_executed * 100 / cycles;
        mpu_ptr->icache_miss_pc = icache_miss * 100 / inst_executed;
        mpu_ptr->dcache_miss_pc = dcache_miss * 100 / inst_executed;
        mpu_ptr->inst_per_phv = inst_executed / phv_executed;
        mpu_ptr->phvwr_stall_pc = phvwr_stall * 100 / cycles;
        mpu_ptr->st_stall_pc = st_stall * 100 / cycles;
    }
}

void
mpu_reset_counters (int verbose, uint8_t pipeline, uint8_t stage, uint8_t mpu)
{
  uint64_t mpu_base = get_mpu_base(pipeline, stage, mpu);
    uint32_t zero = 0;

    pal_reg_wr32w( mpu_base + ELB_MPU_CSR_CNT_INST_EXECUTED_BYTE_OFFSET,
                  &zero, 1);
    pal_reg_wr32w( mpu_base + ELB_MPU_CSR_CNT_ICACHE_MISS_BYTE_OFFSET,
                  &zero, 1);
    pal_reg_wr32w( mpu_base + ELB_MPU_CSR_CNT_DCACHE_MISS_BYTE_OFFSET,
                  &zero, 1);
    pal_reg_wr32w( mpu_base + ELB_MPU_CSR_CNT_CYCLES_BYTE_OFFSET, &zero,
                  1);
    pal_reg_wr32w( mpu_base + ELB_MPU_CSR_CNT_PHV_EXECUTED_BYTE_OFFSET,
                  &zero, 1);
    pal_reg_wr32w( mpu_base + ELB_MPU_CSR_CNT_PHVWR_STALL_BYTE_OFFSET,
                  &zero, 1);
    pal_reg_wr32w( mpu_base + ELB_MPU_CSR_CNT_ST_STALL_BYTE_OFFSET,
                  &zero, 1);
}

void
mpu_read_table_addr (int verbose, uint8_t pipeline, uint8_t stage, uint8_t mpu)
{
  uint64_t mpu_base = get_mpu_base(pipeline, stage, mpu);
  uint32_t table_addr[2];
  uint64_t addr;
  mpu_t *mpu_ptr = NULL;

    if (verbose) {
        pal_reg_rd32w(mpu_base + ELB_MPU_CSR_STA_TBL_ADDR_BYTE_OFFSET,
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
    uint64_t stg_base = get_stg_base(pipeline, stage);
    uint64_t te_base  = get_te_base(pipeline, stage);
    uint32_t sta_stg;
    uint32_t sta_te_fsm[3];
    uint32_t fsm_value;
    uint32_t latency[4];
    uint32_t latency_val[4];
    uint32_t table_type[4];
    uint32_t latency_min = 10000;
    uint32_t latency_max = 0;
    uint32_t latency_total = 0;
    uint32_t last_table_type = 0;
    uint32_t mpu_processing[4], mpu, j;
    uint32_t te_idle = 0, te_new = 0, te_issued = 0, te_queued = 0;
    uint32_t srdy_in = 0, drdy_in = 0, idle_in = 0, util_in = 0, xoff_in = 0;
    uint32_t srdy_out = 0, drdy_out = 0, idle_out = 0, util_out = 0,
             xoff_out = 0;
    int i, polls = 100;
    mpu_t *mpu_ptr = NULL; 
    uint64_t mpu_base;

    for (mpu = 0; mpu < 4; mpu++) {
        mpu_processing[mpu] = 0;
        latency[mpu] = 0;
        latency_val[mpu] = 0;
        table_type[mpu] = 0;
    }

    for (i = 0; i < polls; i++) {
        // Read per-Stage registers
        pal_reg_rd32w(stg_base + ELB_STG_CSR_STA_STG_BYTE_ADDRESS, &sta_stg, 1);

        // Read per-Stage TE FSM
        pal_reg_rd32w(te_base +  ELB_TE_CSR_STA_WAIT_FSM_STATES_BYTE_OFFSET, sta_te_fsm, 3);

        // Read per-MPU registers
        for (mpu = 0; mpu < 4; mpu++) {
	  mpu_base = get_mpu_base(pipeline, stage, mpu);
	  pal_reg_rd32w(mpu_base + ELB_MPU_CSR_STA_TABLE_BYTE_OFFSET,
			&latency[mpu], 1);
        }

        // Process per-MPU registers
        for (mpu = 0; mpu < 4; mpu++) {
            //        ELB_MPU_CSR_STA_CTL_MPU_STA_CTL_MPU_1_2_EX_PC_30_29_GET(sta_ctl[mpu][1])
            //        ELB_MPU_CSR_STA_CTL_MPU_STA_CTL_MPU_0_2_EX_PC_28_0_GET(sta_ctl[mpu][0])
            mpu_processing[mpu] +=
                (ELB_STG_CSR_STA_STG_MPU_PROCESSING_GET(sta_stg) >> mpu) & 0x1;

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
        }

        // Only use MPU[3] for latency as it is used first in distribution
        latency_total += latency_val[3];
        last_table_type = table_type[3];
        if (latency_val[3] > latency_max)
            latency_max = latency_val[3];
        if (latency_val[3] < latency_min)
            latency_min = latency_val[3];

        srdy_in = ELB_STG_CSR_STA_STG_SRDY_GET(sta_stg);
        drdy_in = ELB_STG_CSR_STA_STG_DRDY_GET(sta_stg);
        srdy_out = ELB_STG_CSR_STA_STG_SRDY_OUT_GET(sta_stg);
        drdy_out = ELB_STG_CSR_STA_STG_DRDY_OUT_GET(sta_stg);

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

        // count all te states pending
        for (j = 0; j < TE_NUM_PENDING_READS; j++) {
	  // 3-bit state values packed into 3-word arrary:
	  fsm_value = (sta_te_fsm[ (j * 3) / 32] >> ((j * 3) % 32)) & 0x7;
	  if(((j * 3) % 32)>=30) // 3-bit state wraps to next word
	    {
	      fsm_value += (sta_te_fsm[((j+1) * 3) / 32] << (32 - ((j * 3) % 32))) & 0x7;
	    }
	  if(fsm_value == TE_FSM_IDLE) te_idle++;
	  if(fsm_value == TE_FSM_NEW_ENTRY) te_new++;
	  if(fsm_value == TE_FSM_ISSUED_AXI_REQ0) te_issued++;
	  if(fsm_value == TE_FSM_QUEUED_AXI_REQ0) te_queued++;
        }
    }

    stage_t *stage_ptr = NULL;
    stage_ptr = &(asic->pipelines[pipeline].stages[stage]);
    stage_ptr->util.in = (util_in * 100) / polls;
    stage_ptr->util.out = (util_out * 100) / polls;

    stage_ptr->idle.in = (idle_in * 100) / polls;
    stage_ptr->idle.out = (idle_out * 100) / polls;

    stage_ptr->xoff.in = (xoff_in * 100) / polls;
    stage_ptr->xoff.out = (xoff_out * 100) / polls;
    stage_ptr->last_table_type = last_table_type;
    stage_ptr->_lat = latency_total / polls;
    stage_ptr->te_idle = te_idle / polls;
    stage_ptr->te_new  = te_new / polls;
    stage_ptr->te_issued = te_issued / polls;
    stage_ptr->te_queued = te_queued / polls;
    stage_ptr->min = latency_min;
    stage_ptr->max = latency_max;

    for (mpu = 0; mpu < 4; mpu++) {
        mpu_ptr = &asic->pipelines[pipeline].stages[stage].mpus[mpu];
        mpu_ptr->processing_pc = mpu_processing[mpu] * 100 / polls;
    }
}

