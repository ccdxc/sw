//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file contains the implementation for fetching and storing info
/// from txdma pipline
///
//===----------------------------------------------------------------------===//

#include <stdint.h>
#include <stdio.h>

#include "dtls.hpp"
#include "elb_top_csr_defines.h"
#include "elb_txs_c_hdr.h"
#include "elb_wa_c_hdr.h"
namespace pt {
#include "elb_pt_c_hdr.h"
}
namespace psp {
#include "elb_psp_c_hdr.h"
}

#include "platform/pal/include/pal.h"
#include "elbmon.hpp"

void
elbmon_pipeline_data_store1 (uint8_t type, uint64_t ma_cnt, uint64_t pb_cnt,
                             uint64_t phv_drop, uint64_t phv_err,
                             uint64_t phv_recirc, uint64_t resub_cnt,
                             uint64_t num_phv, uint64_t polls)
{
    pipeline_t *pipeline = &asic->pipelines[type];
    pipeline->phv = ma_cnt;
    pipeline->pb_cnt = pb_cnt;
    pipeline->phv_drop = phv_drop;
    pipeline->phv_err = phv_err;
    pipeline->phv_recirc = phv_recirc;
    pipeline->resub_cnt = resub_cnt;
    pipeline->in_flight = num_phv / polls;
}

void
elbmon_pipeline_data_store2 (uint8_t type, uint64_t pend_rd, uint64_t pend_wr,
                             uint64_t rd_ff_empty, uint64_t rd_ff_full,
                             uint64_t wr_ff_empty, uint64_t wr_ff_full,
                             uint64_t pkt_ff_empty, uint64_t pkt_ff_full,
                             uint64_t pkt_ff_depth, uint64_t polls)
{
    pipeline_t *pipeline = &asic->pipelines[type];
    pipeline->axi_reads = pend_rd / polls;
    pipeline->axi_writes = pend_wr / polls;
    pipeline->rd_empty_fifos = (rd_ff_empty * 100) / polls;
    pipeline->rd_full_fifos = (rd_ff_full * 100) / polls;
    pipeline->wr_empty_fifos = (wr_ff_empty * 100) / polls;
    pipeline->wr_full_fifos = (wr_ff_full * 100) / polls;
    pipeline->pkt_empty_fifos = (pkt_ff_empty * 100) / polls;
    pipeline->pkt_full_fifos = (pkt_ff_full * 100) / polls;
    if (type == RXDMA) {
        pipeline->ff_depth = (pkt_ff_depth * 100) / polls;
    }
}

void
ptd_read_counters (int verbose)
{
    uint32_t cnt_pend[2];
    uint32_t sta_xoff = 0;
    uint32_t  sta_fifo[2];
    int i;
    int pend_rd = 0;
    int pend_wr = 0;
    int num_phv = 0;
    int rd_ff_empty = 0;
    int rd_ff_full = 0;
    int wr_ff_empty = 0;
    int wr_ff_full = 0;
    int pkt_ff_empty = 0;
    int pkt_ff_full = 0;
    int polls = 0;

    uint32_t cnt[4];

    pal_reg_rd32w(ELB_ADDR_BASE_PT_PT_OFFSET + ELB_PT_CSR_PTD_BYTE_ADDRESS +
                      ELB_PTD_CSR_CNT_PHV_BYTE_OFFSET,
                  cnt, 4);
    uint32_t phv_drop = ELB_PTD_CSR_CNT_PHV_CNT_PHV_1_4_DROP_GET(cnt[1]);
    uint32_t phv_err = ELB_PTD_CSR_CNT_PHV_CNT_PHV_2_4_ERR_GET(cnt[2]);
    uint32_t phv_recirc = ELB_PTD_CSR_CNT_PHV_CNT_PHV_3_4_RECIRC_GET(cnt[3]);

    // Number of PHVs sent from pipeline to TXDMA engine
    pal_reg_rd32w(ELB_ADDR_BASE_PT_PT_OFFSET +
                      ELB_PT_CSR_PTD_CNT_MA_BYTE_ADDRESS,
                  cnt, 3);
    int ma_cnt = ELB_PTD_CSR_CNT_MA_CNT_MA_0_3_SOP_31_0_GET(cnt[0]);

    pal_reg_rd32w(ELB_ADDR_BASE_PT_PT_OFFSET +
                      ELB_PT_CSR_PTD_CNT_NPV_RESUB_BYTE_ADDRESS,
                  cnt, 3);
    int resub_cnt =
        ELB_PTD_CSR_CNT_NPV_RESUB_CNT_NPV_RESUB_0_3_SOP_31_0_GET(cnt[0]);

    pal_reg_rd32w(ELB_ADDR_BASE_PT_PT_OFFSET +
                      ELB_PT_CSR_PTD_CNT_PB_BYTE_ADDRESS,
                  cnt, 3);
    int pb_cnt = ELB_PTD_CSR_CNT_PB_CNT_PB_0_3_SOP_31_0_GET(cnt[0]);

    // Pending Reads/Writes, # PHVs
    for (i = 0; i < polls; i++) {
        // FIFO Status
        pal_reg_rd32w(ELB_ADDR_BASE_PT_PT_OFFSET +
                          ELB_PT_CSR_PTD_STA_FIFO_BYTE_ADDRESS,
                      sta_fifo, 2);
        rd_ff_full   += ELB_PTD_CSR_STA_FIFO_STA_FIFO_0_2_LAT_FF_FULL_GET(sta_fifo[0]);
        rd_ff_empty  += ELB_PTD_CSR_STA_FIFO_STA_FIFO_0_2_LAT_FF_EMPTY_GET(sta_fifo[0]);
        wr_ff_full   += ELB_PTD_CSR_STA_FIFO_STA_FIFO_0_2_WR_MEM_FF_FULL_GET(sta_fifo[0]);
        wr_ff_empty  += ELB_PTD_CSR_STA_FIFO_STA_FIFO_0_2_WR_MEM_FF_EMPTY_GET(sta_fifo[0]);
        pkt_ff_full  += ELB_PTD_CSR_STA_FIFO_STA_FIFO_0_2_PKT_FF_FULL_GET(sta_fifo[0]);
        pkt_ff_empty += ELB_PTD_CSR_STA_FIFO_STA_FIFO_0_2_PKT_FF_EMPTY_GET(sta_fifo[0]);
        // Pending:
        pal_reg_rd32w(ELB_ADDR_BASE_PT_PT_OFFSET +
                          ELB_PT_CSR_PTD_STA_ID_BYTE_ADDRESS,
                      cnt_pend, 2);
        pal_reg_rd32w(ELB_ADDR_BASE_PT_PT_OFFSET +
                          ELB_PT_CSR_PTD_STA_XOFF_BYTE_ADDRESS,
                      &sta_xoff, 1);
        pend_rd += ELB_PTD_CSR_STA_ID_STA_ID_0_2_RD_PEND_CNT_GET(cnt_pend[0]);
        pend_wr += ELB_PTD_CSR_STA_ID_STA_ID_0_2_WR_PEND_CNT_GET(cnt_pend[0]);
        num_phv += ELB_PTD_CSR_STA_XOFF_NUMPHV_COUNTER_GET(sta_xoff);
    }

    elbmon_pipeline_data_store1(TXDMA, ma_cnt, pb_cnt, phv_drop, phv_err,
                                phv_recirc, resub_cnt, num_phv, polls);
    elbmon_pipeline_data_store2(
        TXDMA, pend_rd / polls, pend_wr / polls, rd_ff_empty, rd_ff_full,
        wr_ff_empty, wr_ff_full, pkt_ff_empty, pkt_ff_full, 0, polls);
}

void
ptd_reset_counters (int verbose)
{
    uint32_t zero[4] = {0};

    pal_reg_wr32w(ELB_ADDR_BASE_PT_PT_OFFSET + ELB_PT_CSR_PTD_BYTE_ADDRESS +
                      ELB_PTD_CSR_CNT_PHV_BYTE_OFFSET,
                  zero, 4);
    pal_reg_wr32w(ELB_ADDR_BASE_PT_PT_OFFSET +
                      ELB_PT_CSR_PTD_CNT_MA_BYTE_ADDRESS,
                  zero, 3);
    pal_reg_wr32w(ELB_ADDR_BASE_PT_PT_OFFSET +
                      ELB_PT_CSR_PTD_CNT_NPV_RESUB_BYTE_ADDRESS,
                  zero, 3);
    pal_reg_wr32w(ELB_ADDR_BASE_PT_PT_OFFSET +
                      ELB_PT_CSR_PTD_CNT_PB_BYTE_ADDRESS,
                  zero, 3);
}

void
elbmon_pipeline_data_store3 (uint8_t type, uint64_t ma_cnt, uint64_t pb_pbus_cnt,
                             uint64_t pr_pbus_cnt, uint64_t sw_cnt,
                             uint64_t ma_drop_cnt, uint64_t ma_recirc_cnt)
{
    pipeline_t *pipeline = NULL;
    pipeline = &(asic->pipelines[type]);

    pipeline->phv = ma_cnt;
    pipeline->pb_pbus_cnt = pb_pbus_cnt;
    pipeline->pr_pbus_cnt = pr_pbus_cnt;
    pipeline->sw_cnt = sw_cnt;
    pipeline->phv_drop_cnt = ma_drop_cnt;
    pipeline->recirc_cnt = ma_recirc_cnt;
}

void
npv_read_counters (int verbose)
{
    uint64_t ma_cnt, sw_cnt, pb_pbus_cnt, pr_pbus_cnt, ma_drop_cnt,
        ma_recirc_cnt;

    pal_reg_rd32w(ELB_ADDR_BASE_PT_PT_OFFSET +
                      ELB_PSP_CSR_CNT_MA_SOP_BYTE_ADDRESS,
                  (uint32_t *)&ma_cnt, 2);
    pal_reg_rd32w(ELB_ADDR_BASE_PT_PT_OFFSET +
                      ELB_PSP_CSR_CNT_SW_SOP_BYTE_ADDRESS,
                  (uint32_t *)&sw_cnt, 2);
    pal_reg_rd32w(ELB_ADDR_BASE_PT_PT_OFFSET +
                      ELB_PSP_CSR_CNT_PB_PBUS_SOP_BYTE_ADDRESS,
                  (uint32_t *)&pb_pbus_cnt, 2);
    pal_reg_rd32w(ELB_ADDR_BASE_PT_PT_OFFSET +
                      ELB_PSP_CSR_CNT_PR_PBUS_SOP_BYTE_ADDRESS,
                  (uint32_t *)&pr_pbus_cnt, 2);
    pal_reg_rd32w(ELB_ADDR_BASE_PT_PT_OFFSET +
                      ELB_PSP_CSR_CNT_MA_DROP_BYTE_ADDRESS,
                  (uint32_t *)&ma_drop_cnt, 2);
    pal_reg_rd32w(ELB_ADDR_BASE_PT_PT_OFFSET +
                      ELB_PSP_CSR_CNT_MA_RECIRC_BYTE_ADDRESS,
                  (uint32_t *)&ma_recirc_cnt, 2);

    elbmon_pipeline_data_store3(TXDMA, ma_cnt, pb_pbus_cnt, pr_pbus_cnt, sw_cnt,
                                ma_drop_cnt, ma_recirc_cnt);
}

void
npv_reset_counters (int verbose)
{
    uint32_t zero[4] = {0};

    pal_reg_wr32w(ELB_ADDR_BASE_PT_PT_OFFSET +
                      ELB_PSP_CSR_CNT_MA_SOP_BYTE_ADDRESS,
                  zero, 2);
    pal_reg_wr32w(ELB_ADDR_BASE_PT_PT_OFFSET +
                      ELB_PSP_CSR_CNT_SW_SOP_BYTE_ADDRESS,
                  zero, 2);
    pal_reg_wr32w(ELB_ADDR_BASE_PT_PT_OFFSET +
                      ELB_PSP_CSR_CNT_PB_PBUS_SOP_BYTE_ADDRESS,
                  zero, 2);
    pal_reg_wr32w(ELB_ADDR_BASE_PT_PT_OFFSET +
                      ELB_PSP_CSR_CNT_PR_PBUS_SOP_BYTE_ADDRESS,
                  zero, 2);
    pal_reg_wr32w(ELB_ADDR_BASE_PT_PT_OFFSET +
                      ELB_PSP_CSR_CNT_MA_DROP_BYTE_ADDRESS,
                  zero, 2);
    pal_reg_wr32w(ELB_ADDR_BASE_PT_PT_OFFSET +
                      ELB_PSP_CSR_CNT_MA_RECIRC_BYTE_ADDRESS,
                  zero, 2);
}

void
elbmon_asic_data_store7 (uint64_t host_dbs, uint64_t local_dbs,
                         uint64_t db_to_sched_reqs)
{
    asic->host_dbs = host_dbs;
    asic->local_dbs = local_dbs;
    asic->db_to_sched_reqs = db_to_sched_reqs;
}

void
doorbell_read_counters (int verbose)
{
    uint64_t host_cnt, local_cnt, sched_cnt;

    // Doorbell Host
    pal_reg_rd32w(ELB_ADDR_BASE_DB_WA_OFFSET +
                      ELB_WA_CSR_CNT_WA_HOST_DOORBELLS_BYTE_ADDRESS,
                  (uint32_t *)&host_cnt, 2);

    // Doorbell Local
    pal_reg_rd32w(ELB_ADDR_BASE_DB_WA_OFFSET +
                      ELB_WA_CSR_CNT_WA_LOCAL_DOORBELLS_BYTE_ADDRESS,
                  (uint32_t *)&local_cnt, 2);

    // Doorbell to Sched
    pal_reg_rd32w(ELB_ADDR_BASE_DB_WA_OFFSET +
                      ELB_WA_CSR_CNT_WA_SCHED_OUT_BYTE_ADDRESS,
                  (uint32_t *)&sched_cnt, 2);

    elbmon_asic_data_store7(host_cnt, local_cnt, sched_cnt);
}

void
doorbell_reset_counters (int verbose)
{
    uint32_t zero[4] = {0};

    // Doorbell count reset
    pal_reg_wr32w(ELB_ADDR_BASE_TXS_TXS_OFFSET +
                      ELB_TXS_CSR_CNT_SCH_DOORBELL_SET_BYTE_ADDRESS,
                  zero, 2);
    pal_reg_wr32w(ELB_ADDR_BASE_TXS_TXS_OFFSET +
                      ELB_TXS_CSR_CNT_SCH_DOORBELL_CLR_BYTE_ADDRESS,
                  zero, 2);
    // Doorbell Host
    pal_reg_wr32w(ELB_ADDR_BASE_DB_WA_OFFSET +
                      ELB_WA_CSR_CNT_WA_HOST_DOORBELLS_BYTE_ADDRESS,
                  zero, 2);
    // Doorbell Local
    pal_reg_wr32w(ELB_ADDR_BASE_DB_WA_OFFSET +
                      ELB_WA_CSR_CNT_WA_LOCAL_DOORBELLS_BYTE_ADDRESS,
                  zero, 2);
    // Doorbell to Sched
    pal_reg_wr32w(ELB_ADDR_BASE_DB_WA_OFFSET +
                      ELB_WA_CSR_CNT_WA_SCHED_OUT_BYTE_ADDRESS,
                  zero, 2);
}

void
txs_read_counters (int verbose)
{
    uint32_t cnt[2];
    uint32_t xoff_vector;
    uint32_t cnt_txdma[2];
    uint32_t cnt_sxdma[2];
    uint32_t xoff[16] = {0};
    int cos, i;
    int polls = 100;
    uint32_t sets, clears;

    // Doorbell sets/clears
    pal_reg_rd32w(ELB_ADDR_BASE_TXS_TXS_OFFSET +
                      ELB_TXS_CSR_CNT_SCH_DOORBELL_SET_BYTE_ADDRESS,
                  cnt, 2);
    sets = cnt[0];

    pal_reg_rd32w(ELB_ADDR_BASE_TXS_TXS_OFFSET +
                      ELB_TXS_CSR_CNT_SCH_DOORBELL_CLR_BYTE_ADDRESS,
                  cnt, 2);
    clears = cnt[0];

    asic->sets = sets;
    asic->clears = clears;

    // Packet Buffer XOFF
    for (i = 0; i < polls; i++) {
        pal_reg_rd32w(ELB_ADDR_BASE_TXS_TXS_OFFSET +
                          ELB_TXS_CSR_STA_GLB_BYTE_OFFSET,
                      &xoff_vector, 1);
        for (cos = 0; cos < 16; cos++) {
            xoff[cos] += (((xoff_vector >> cos) & 1) == 1) ? 1 : 0;
        }
    }

    for (cos = 0; cos < 16; cos++) {
        asic->xoff[cos] = (xoff[cos] * 100) / polls;
    }

    // TxDMA PHVs
    pal_reg_rd32w(ELB_TXS_CSR_CNT_SCH_TXDMA_SENT_OFFSET,
		  cnt_txdma, 2);
    asic->txdma_phvs = ELB_TXS_CSR_CNT_SCH_TXDMA_SENT_CNT_SCH_TXDMA_SENT_0_2_VAL_31_0_GET(cnt_txdma[0]) +
                 ELB_TXS_CSR_CNT_SCH_TXDMA_SENT_CNT_SCH_TXDMA_SENT_1_2_VAL_63_32_GET(cnt_txdma[1]);

    // SxDMA PHVs
    pal_reg_rd32w(ELB_TXS_CSR_CNT_SCH_SXDMA_SENT_OFFSET,
		  cnt_sxdma, 2);
    asic->sxdma_phvs = ELB_TXS_CSR_CNT_SCH_SXDMA_SENT_CNT_SCH_SXDMA_SENT_0_2_VAL_31_0_GET(cnt_sxdma[0]) +
                 ELB_TXS_CSR_CNT_SCH_SXDMA_SENT_CNT_SCH_SXDMA_SENT_1_2_VAL_63_32_GET(cnt_sxdma[1]);


}

void
txs_read_debug_counters (int verbose)
{
    uint32_t cnt, enable, lif;
    uint32_t cfg[3];
    int i;
    int stride = ELB_TXS_CSR_CNT_DOORBELL_DEBUG1_OFFSET - ELB_TXS_CSR_CNT_DOORBELL_DEBUG0_OFFSET;

    for(i=0; i<4; i++) {
      pal_reg_rd32w(ELB_TXS_CSR_CNT_DOORBELL_DEBUG0_OFFSET + (i * stride), &cnt, 1);
      asic->cnt_doorbell[i] = cnt;
      pal_reg_rd32w(ELB_TXS_CSR_CNT_TXDMA_DEBUG0_OFFSET + (i * stride), &cnt, 1);
      asic->cnt_txdma[i] = cnt;
      pal_reg_rd32w(ELB_TXS_CSR_CNT_SXDMA_DEBUG0_OFFSET + (i * stride), &cnt, 1);
      asic->cnt_sxdma[i] = cnt;
    }

    stride = ELB_TXS_CSR_CFG_DOORBELL_DEBUG1_OFFSET - ELB_TXS_CSR_CFG_DOORBELL_DEBUG0_OFFSET;
    for(i=0; i<4; i++) {
      pal_reg_rd32w(ELB_TXS_CSR_CFG_DOORBELL_DEBUG0_OFFSET + (i * stride), cfg, 3);
      enable = ELB_TXS_CSR_CFG_DOORBELL_DEBUG0_CFG_DOORBELL_DEBUG0_0_3_ENABLE_GET(cfg[0]);
      lif = ELB_TXS_CSR_CFG_DOORBELL_DEBUG0_CFG_DOORBELL_DEBUG0_0_3_LIF_START_GET(cfg[0]);
      asic->cnt_lif[i] = lif;
      asic->cnt_enable[i] = enable;
    }
}

void
txs_reset_counters (int verbose)
{
    uint32_t zero[2] = {0};

    // TXS counter reset
    pal_reg_wr32w(ELB_TXS_CSR_CNT_SCH_TXDMA_SENT_OFFSET,
		  zero, 2);
    pal_reg_wr32w(ELB_TXS_CSR_CNT_SCH_SXDMA_SENT_OFFSET,
		  zero, 2);

}

void
txs_reset_debug_counters (int verbose)
{
    uint32_t zero = 0;
    int i;
    int stride = ELB_TXS_CSR_CNT_DOORBELL_DEBUG1_OFFSET - ELB_TXS_CSR_CNT_DOORBELL_DEBUG0_OFFSET;

    for(i=0; i<4; i++) {
      pal_reg_wr32w(ELB_TXS_CSR_CNT_DOORBELL_DEBUG0_OFFSET + (i * stride), &zero, 1);
      pal_reg_wr32w(ELB_TXS_CSR_CNT_TXDMA_DEBUG0_OFFSET + (i * stride), &zero, 1);
      pal_reg_wr32w(ELB_TXS_CSR_CNT_SXDMA_DEBUG0_OFFSET + (i * stride), &zero, 1);
    }
}

void
txs_program_debug_counters (int verbose)
{
    uint32_t cfg[3] = {0};
    int i;
    int stride;
    int lif;
    int lif_start = 0;
    int lif_end = 2047;
    u_int32_t cnt[8];
    u_int32_t valid;

    i = 0;
    // Detect enabled LIFs, choose first 4:
    for (lif = lif_start; lif <= lif_end; lif++) {
        pal_reg_rd32w(ELB_ADDR_BASE_DB_WA_OFFSET +
                          ELB_WA_CSR_DHS_LIF_QSTATE_MAP_BYTE_ADDRESS +
                          (16 * lif),
                      cnt, 8);
        valid = ELB_WA_CSR_DHS_LIF_QSTATE_MAP_ENTRY_ENTRY_0_8_VLD_GET(cnt[0]);
        if (!valid) {
            continue;
        }
	asic->cnt_lif[i] = lif; // chose next debug LIF to observe
	asic->cnt_enable[i] = 1;
	if(i >= 3) break; // got the first 4 enabled LIFs
	else i++;
    }
    // Program the debug count registers
    stride = ELB_TXS_CSR_CFG_DOORBELL_DEBUG1_OFFSET - ELB_TXS_CSR_CFG_DOORBELL_DEBUG0_OFFSET;
    for(i=0; i<4; i++) {
      // DB
      cfg[0] = ELB_TXS_CSR_CFG_DOORBELL_DEBUG0_CFG_DOORBELL_DEBUG0_0_3_ENABLE_SET(asic->cnt_enable[i]) |
	ELB_TXS_CSR_CFG_DOORBELL_DEBUG0_CFG_DOORBELL_DEBUG0_0_3_LIF_START_SET(asic->cnt_lif[i]) |
	ELB_TXS_CSR_CFG_DOORBELL_DEBUG0_CFG_DOORBELL_DEBUG0_0_3_LIF_END_SET(asic->cnt_lif[i]) |
	ELB_TXS_CSR_CFG_DOORBELL_DEBUG0_CFG_DOORBELL_DEBUG0_0_3_LIF_EN_SET(asic->cnt_enable[i]);
      pal_reg_wr32w(ELB_TXS_CSR_CFG_DOORBELL_DEBUG0_OFFSET + (i * stride), cfg, 3);
      // TXDMA
      cfg[0] = ELB_TXS_CSR_CFG_TXDMA_DEBUG0_CFG_TXDMA_DEBUG0_0_3_ENABLE_SET(asic->cnt_enable[i]) |
	ELB_TXS_CSR_CFG_TXDMA_DEBUG0_CFG_TXDMA_DEBUG0_0_3_LIF_START_SET(asic->cnt_lif[i]) |
	ELB_TXS_CSR_CFG_TXDMA_DEBUG0_CFG_TXDMA_DEBUG0_0_3_LIF_END_SET(asic->cnt_lif[i]) |
	ELB_TXS_CSR_CFG_TXDMA_DEBUG0_CFG_TXDMA_DEBUG0_0_3_LIF_EN_SET(asic->cnt_enable[i]);
      pal_reg_wr32w(ELB_TXS_CSR_CFG_TXDMA_DEBUG0_OFFSET + (i * stride), cfg, 3);
    }
}
