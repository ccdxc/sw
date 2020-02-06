//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
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

#include "cap_top_csr_defines.h"
#include "cap_txs_c_hdr.h"
#include "cap_wa_c_hdr.h"
namespace pt {
#include "cap_pt_c_hdr.h"
}
namespace psp {
#include "cap_psp_c_hdr.h"
}

#include "platform/pal/include/pal.h"
#include "capmon.hpp"

void
capmon_pipeline_data_store1(uint8_t type,
                            uint64_t phv_cnt, uint64_t phv_drop,
                            uint64_t phv_err, uint64_t phv_recirc,
                            uint64_t pb_cnt,
                            uint64_t resub_cnt)
{
    pipeline_t *pipeline = &asic->pipelines[type];
    pipeline->phv_cnt = phv_cnt;
    pipeline->phv_drop = phv_drop;
    pipeline->phv_err = phv_err;
    pipeline->phv_recirc = phv_recirc;
    pipeline->pb_cnt = pb_cnt;
    pipeline->resub_cnt = resub_cnt;
}

void
capmon_pipeline_data_store2(uint8_t type,
                            uint64_t pend_rd, uint64_t pend_wr,
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
capmon_pipeline_data_store3(uint8_t type,
                            uint64_t ma_cnt, uint64_t ma_drop_cnt,
                            uint64_t ma_recirc_cnt,
                            uint64_t sw_cnt,
                            uint64_t pb_pbus_cnt, uint64_t pr_pbus_cnt)
{
    pipeline_t *pipeline = &asic->pipelines[type];

    pipeline->ma_cnt = ma_cnt;
    pipeline->ma_drop_cnt = ma_drop_cnt;
    pipeline->ma_recirc_cnt = ma_recirc_cnt;
    pipeline->sw_cnt = sw_cnt;
    pipeline->pb_pbus_cnt = pb_pbus_cnt;
    pipeline->pr_pbus_cnt = pr_pbus_cnt;
}

void
capmon_pipeline_data_store4(int type,
                            uint8_t phv_xoff, uint64_t phv_xoff_cnt,
                            uint8_t host_xoff, uint64_t host_xoff_cnt,
                            uint8_t pkt_xoff, uint64_t pkt_xoff_cnt,
                            uint64_t polls)
{
    pipeline_t *pipeline = &asic->pipelines[type];

    pipeline->phv_xoff = (phv_xoff * 100.0) / polls;
    pipeline->phv_xoff_cnt = phv_xoff_cnt / polls;
    pipeline->pkt_xoff = (pkt_xoff * 100.0) / polls;
    pipeline->pkt_xoff_cnt = pkt_xoff_cnt / polls;
    pipeline->host_xoff = (host_xoff * 100.0) / polls;
    pipeline->host_xoff_cnt = host_xoff_cnt / polls;
}

void
ptd_read_counters(int verbose)
{
    uint32_t sta_id = 0, sta_xoff = 0, sta_fifo = 0;
    uint32_t cnt[4] = {0};
    uint32_t pend_rd = 0;
    uint32_t pend_wr = 0;
    uint32_t rd_ff_empty = 0;
    uint32_t rd_ff_full = 0;
    uint32_t wr_ff_empty = 0;
    uint32_t wr_ff_full = 0;
    uint32_t pkt_ff_empty = 0;
    uint32_t pkt_ff_full = 0;
    uint32_t phv_xoff = 0;
    uint32_t phv_xoff_cnt = 0;
    int i;
    int polls = 100;

    // TXDMA stats
    pal_reg_rd32w(CAP_ADDR_BASE_PT_PT_OFFSET + CAP_PT_CSR_PTD_BYTE_ADDRESS +
                      CAP_PTD_CSR_CNT_PHV_BYTE_OFFSET,
                  cnt, 4);
    uint32_t phv_drop = CAP_PTD_CSR_CNT_PHV_CNT_PHV_1_4_DROP_GET(cnt[1]);
    uint32_t phv_err = CAP_PTD_CSR_CNT_PHV_CNT_PHV_2_4_ERR_GET(cnt[2]);
    uint32_t phv_recirc = CAP_PTD_CSR_CNT_PHV_CNT_PHV_3_4_RECIRC_GET(cnt[3]);

    // Number of PHVs sent from pipeline to TXDMA engine
    pal_reg_rd32w(CAP_ADDR_BASE_PT_PT_OFFSET +
                      CAP_PT_CSR_PTD_CNT_MA_BYTE_ADDRESS,
                  cnt, 3);
    uint64_t phv_cnt =
        CAP_PTD_CSR_CNT_MA_CNT_MA_0_3_SOP_31_0_GET(cnt[0]) |
        ((uint64_t)(CAP_PTD_CSR_CNT_MA_CNT_MA_1_3_SOP_39_32_GET(cnt[1])) << 32);

    // Number of PHVs recirculated
    pal_reg_rd32w(CAP_ADDR_BASE_PT_PT_OFFSET +
                      CAP_PT_CSR_PTD_CNT_NPV_RESUB_BYTE_ADDRESS,
                  cnt, 3);
    uint64_t resub_cnt =
        CAP_PTD_CSR_CNT_NPV_RESUB_CNT_NPV_RESUB_0_3_SOP_31_0_GET(cnt[0]) |
        ((uint64_t)(CAP_PTD_CSR_CNT_NPV_RESUB_CNT_NPV_RESUB_1_3_SOP_39_32_GET(cnt[1])) << 32);

    // Number of packets transmitted
    pal_reg_rd32w(CAP_ADDR_BASE_PT_PT_OFFSET +
                      CAP_PT_CSR_PTD_CNT_PB_BYTE_ADDRESS,
                  cnt, 3);
    uint64_t pb_cnt =
        CAP_PTD_CSR_CNT_PB_CNT_PB_0_3_SOP_31_0_GET(cnt[0]) |
        ((uint64_t)(CAP_PTD_CSR_CNT_PB_CNT_PB_1_3_SOP_39_32_GET(cnt[1])) << 32);

    for (i = 0; i < polls; i++) {
        // FIFO Status
        pal_reg_rd32w(CAP_ADDR_BASE_PT_PT_OFFSET +
                          CAP_PT_CSR_PTD_STA_FIFO_BYTE_ADDRESS,
                      &sta_fifo, 1);
        rd_ff_full += CAP_PTD_CSR_STA_FIFO_LAT_FF_FULL_GET(sta_fifo);
        rd_ff_empty += CAP_PTD_CSR_STA_FIFO_LAT_FF_EMPTY_GET(sta_fifo);
        wr_ff_full += CAP_PTD_CSR_STA_FIFO_WR_MEM_FF_FULL_GET(sta_fifo);
        wr_ff_empty += CAP_PTD_CSR_STA_FIFO_WR_MEM_FF_EMPTY_GET(sta_fifo);
        pkt_ff_full += CAP_PTD_CSR_STA_FIFO_PKT_FF_FULL_GET(sta_fifo);
        pkt_ff_empty += CAP_PTD_CSR_STA_FIFO_PKT_FF_EMPTY_GET(sta_fifo);

        // Pending reads/writes
        pal_reg_rd32w(CAP_ADDR_BASE_PT_PT_OFFSET +
                          CAP_PT_CSR_PTD_STA_ID_BYTE_ADDRESS,
                      &sta_id, 1);
        pend_rd += CAP_PTD_CSR_STA_ID_RD_PEND_CNT_GET(sta_id);
        pend_wr += CAP_PTD_CSR_STA_ID_WR_PEND_CNT_GET(sta_id);

        // XOFF
        pal_reg_rd32w(CAP_ADDR_BASE_PT_PT_OFFSET +
                          CAP_PT_CSR_PTD_STA_XOFF_BYTE_ADDRESS,
                      &sta_xoff, 1);

        phv_xoff += CAP_PTD_CSR_STA_XOFF_NUMPHV_XOFF_GET(sta_xoff);
        phv_xoff_cnt += CAP_PTD_CSR_STA_XOFF_NUMPHV_COUNTER_GET(sta_xoff);
    }

    capmon_pipeline_data_store1(TXDMA, phv_cnt, phv_drop, phv_err, phv_recirc,
                                pb_cnt, resub_cnt);
    capmon_pipeline_data_store2(TXDMA, pend_rd, pend_wr,
                                rd_ff_empty, rd_ff_full,
                                wr_ff_empty, wr_ff_full,
                                pkt_ff_empty, pkt_ff_full,
                                0, polls);
    capmon_pipeline_data_store4(TXDMA,
                                phv_xoff, phv_xoff_cnt,
                                0, 0,
                                0, 0,
                                polls);
}

void
ptd_reset_counters(int verbose)
{
    uint32_t zero[4] = {0};

    pal_reg_wr32w(CAP_ADDR_BASE_PT_PT_OFFSET + CAP_PT_CSR_PTD_BYTE_ADDRESS +
                      CAP_PTD_CSR_CNT_PHV_BYTE_OFFSET,
                  zero, 4);
    pal_reg_wr32w(CAP_ADDR_BASE_PT_PT_OFFSET +
                      CAP_PT_CSR_PTD_CNT_MA_BYTE_ADDRESS,
                  zero, 3);
    pal_reg_wr32w(CAP_ADDR_BASE_PT_PT_OFFSET +
                      CAP_PT_CSR_PTD_CNT_NPV_RESUB_BYTE_ADDRESS,
                  zero, 3);
    pal_reg_wr32w(CAP_ADDR_BASE_PT_PT_OFFSET +
                      CAP_PT_CSR_PTD_CNT_PB_BYTE_ADDRESS,
                  zero, 3);
}

void
npv_read_counters(int verbose)
{
    uint64_t ma_cnt, sw_cnt, pb_pbus_cnt, pr_pbus_cnt, ma_drop_cnt,
        ma_recirc_cnt;

    pal_reg_rd32w(CAP_ADDR_BASE_PT_PT_OFFSET +
                      CAP_PSP_CSR_CNT_MA_SOP_BYTE_ADDRESS,
                  (uint32_t *)&ma_cnt, 2);
    pal_reg_rd32w(CAP_ADDR_BASE_PT_PT_OFFSET +
                      CAP_PSP_CSR_CNT_MA_DROP_BYTE_ADDRESS,
                  (uint32_t *)&ma_drop_cnt, 2);
    pal_reg_rd32w(CAP_ADDR_BASE_PT_PT_OFFSET +
                      CAP_PSP_CSR_CNT_MA_RECIRC_BYTE_ADDRESS,
                  (uint32_t *)&ma_recirc_cnt, 2);
    pal_reg_rd32w(CAP_ADDR_BASE_PT_PT_OFFSET +
                      CAP_PSP_CSR_CNT_SW_SOP_BYTE_ADDRESS,
                  (uint32_t *)&sw_cnt, 2);
    pal_reg_rd32w(CAP_ADDR_BASE_PT_PT_OFFSET +
                      CAP_PSP_CSR_CNT_PB_PBUS_SOP_BYTE_ADDRESS,
                  (uint32_t *)&pb_pbus_cnt, 2);
    pal_reg_rd32w(CAP_ADDR_BASE_PT_PT_OFFSET +
                      CAP_PSP_CSR_CNT_PR_PBUS_SOP_BYTE_ADDRESS,
                  (uint32_t *)&pr_pbus_cnt, 2);

    capmon_pipeline_data_store3(TXDMA, ma_cnt, ma_drop_cnt, ma_recirc_cnt,
                                sw_cnt, pb_pbus_cnt, pr_pbus_cnt);
}

void
npv_reset_counters(int verbose)
{
    uint32_t zero[4] = {0};

    pal_reg_wr32w(CAP_ADDR_BASE_PT_PT_OFFSET +
                      CAP_PSP_CSR_CNT_MA_SOP_BYTE_ADDRESS,
                  zero, 2);
    pal_reg_wr32w(CAP_ADDR_BASE_PT_PT_OFFSET +
                      CAP_PSP_CSR_CNT_SW_SOP_BYTE_ADDRESS,
                  zero, 2);
    pal_reg_wr32w(CAP_ADDR_BASE_PT_PT_OFFSET +
                      CAP_PSP_CSR_CNT_PB_PBUS_SOP_BYTE_ADDRESS,
                  zero, 2);
    pal_reg_wr32w(CAP_ADDR_BASE_PT_PT_OFFSET +
                      CAP_PSP_CSR_CNT_PR_PBUS_SOP_BYTE_ADDRESS,
                  zero, 2);
    pal_reg_wr32w(CAP_ADDR_BASE_PT_PT_OFFSET +
                      CAP_PSP_CSR_CNT_MA_DROP_BYTE_ADDRESS,
                  zero, 2);
    pal_reg_wr32w(CAP_ADDR_BASE_PT_PT_OFFSET +
                      CAP_PSP_CSR_CNT_MA_RECIRC_BYTE_ADDRESS,
                  zero, 2);
}

void
capmon_asic_data_store7(uint64_t host_dbs, uint64_t local_dbs,
                        uint64_t db_to_sched_reqs)
{
    asic->host_dbs = host_dbs;
    asic->local_dbs = local_dbs;
    asic->db_to_sched_reqs = db_to_sched_reqs;
}

void
doorbell_read_counters(int verbose)
{
    uint64_t host_cnt, local_cnt, sched_cnt;

    // Doorbell Host
    pal_reg_rd32w(CAP_ADDR_BASE_DB_WA_OFFSET +
                      CAP_WA_CSR_CNT_WA_HOST_DOORBELLS_BYTE_ADDRESS,
                  (uint32_t *)&host_cnt, 2);

    // Doorbell Local
    pal_reg_rd32w(CAP_ADDR_BASE_DB_WA_OFFSET +
                      CAP_WA_CSR_CNT_WA_LOCAL_DOORBELLS_BYTE_ADDRESS,
                  (uint32_t *)&local_cnt, 2);

    // Doorbell to Sched
    pal_reg_rd32w(CAP_ADDR_BASE_DB_WA_OFFSET +
                      CAP_WA_CSR_CNT_WA_SCHED_OUT_BYTE_ADDRESS,
                  (uint32_t *)&sched_cnt, 2);

    capmon_asic_data_store7(host_cnt, local_cnt, sched_cnt);
}

void
doorbell_reset_counters(int verbose)
{
    uint32_t zero[4] = {0};

    // Doorbell count reset
    pal_reg_wr32w(CAP_ADDR_BASE_TXS_TXS_OFFSET +
                      CAP_TXS_CSR_CNT_SCH_DOORBELL_SET_BYTE_ADDRESS,
                  zero, 2);
    pal_reg_wr32w(CAP_ADDR_BASE_TXS_TXS_OFFSET +
                      CAP_TXS_CSR_CNT_SCH_DOORBELL_CLR_BYTE_ADDRESS,
                  zero, 2);
    // Doorbell Host
    pal_reg_wr32w(CAP_ADDR_BASE_DB_WA_OFFSET +
                      CAP_WA_CSR_CNT_WA_HOST_DOORBELLS_BYTE_ADDRESS,
                  zero, 2);
    // Doorbell Local
    pal_reg_wr32w(CAP_ADDR_BASE_DB_WA_OFFSET +
                      CAP_WA_CSR_CNT_WA_LOCAL_DOORBELLS_BYTE_ADDRESS,
                  zero, 2);
    // Doorbell to Sched
    pal_reg_wr32w(CAP_ADDR_BASE_DB_WA_OFFSET +
                      CAP_WA_CSR_CNT_WA_SCHED_OUT_BYTE_ADDRESS,
                  zero, 2);
}

void
txs_read_counters(int verbose)
{
    uint32_t cnt[2];
    uint32_t xoff_vector, cnt_txdma;
    uint32_t xoff[16] = {0};
    int cos, i;
    int polls = 100;
    uint32_t sets, clears;

    // Doorbell sets/clears
    pal_reg_rd32w(CAP_ADDR_BASE_TXS_TXS_OFFSET +
                      CAP_TXS_CSR_CNT_SCH_DOORBELL_SET_BYTE_ADDRESS,
                  cnt, 2);
    sets = cnt[0];

    pal_reg_rd32w(CAP_ADDR_BASE_TXS_TXS_OFFSET +
                      CAP_TXS_CSR_CNT_SCH_DOORBELL_CLR_BYTE_ADDRESS,
                  cnt, 2);
    clears = cnt[0];

    asic->sets = sets;
    asic->clears = clears;

    // Packet Buffer XOFF
    for (i = 0; i < polls; i++) {
        pal_reg_rd32w(CAP_ADDR_BASE_TXS_TXS_OFFSET +
                          CAP_TXS_CSR_STA_GLB_BYTE_OFFSET,
                      &xoff_vector, 1);
        for (cos = 0; cos < 16; cos++) {
            xoff[cos] += (((xoff_vector >> cos) & 1) == 1) ? 1 : 0;
        }
    }

    for (cos = 0; cos < 16; cos++) {
        asic->xoff[cos] = (xoff[cos] * 100) / polls;
    }

    // TxDMA PHVs
    for (cos = 0; cos < 16; cos++) {
        pal_reg_rd32w(CAP_ADDR_BASE_TXS_TXS_OFFSET +
                          CAP_TXS_CSR_CNT_SCH_TXDMA_COS0_ADDRESS + (cos * 8),
                      &cnt_txdma, 1);
        asic->phvs[cos] = cnt_txdma;
        if (cnt_txdma > 0) {
        }
    }
}

void
txs_reset_counters(int verbose)
{
    uint32_t zero[4] = {0};
    int cos;

    // TXS counter reset
    for (cos = 0; cos < 16; cos++) {
        pal_reg_wr32w(CAP_ADDR_BASE_TXS_TXS_OFFSET + (cos * 8) +
                          CAP_TXS_CSR_CNT_SCH_TXDMA_COS0_ADDRESS,
                      zero, 1);
    }
}
