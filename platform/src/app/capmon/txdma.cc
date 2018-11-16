/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */
#include <stdint.h>
#include <stdio.h>

#include "dtls.hpp"

#include "cap_top_csr_defines.h"
#include "cap_txs_c_hdr.h"
#include "cap_wa_c_hdr.h"
namespace pt
{
#include "cap_pt_c_hdr.h"
}
namespace psp
{
#include "cap_psp_c_hdr.h"
}

#include "platform/src/lib/pal/include/pal.h"

void
ptd_read_counters(int verbose)
{
    uint32_t cnt_pend = 0;
    uint32_t sta_xoff = 0, sta_fifo = 0;
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

    pal_reg_rd32w(CAP_ADDR_BASE_PT_PT_OFFSET + CAP_PT_CSR_PTD_BYTE_ADDRESS +
                      CAP_PTD_CSR_CNT_PHV_BYTE_OFFSET,
                  cnt, 4);
    uint32_t phv_drop = CAP_PTD_CSR_CNT_PHV_CNT_PHV_1_4_DROP_GET(cnt[1]);
    uint32_t phv_err = CAP_PTD_CSR_CNT_PHV_CNT_PHV_2_4_ERR_GET(cnt[2]);
    uint32_t phv_recirc = CAP_PTD_CSR_CNT_PHV_CNT_PHV_3_4_RECIRC_GET(cnt[3]);

    // Number of PHVs sent from pipeline to TXDMA engine
    pal_reg_rd32w(CAP_ADDR_BASE_PT_PT_OFFSET + CAP_PT_CSR_PTD_CNT_MA_BYTE_ADDRESS, cnt, 3);
    int ma_cnt = CAP_PTD_CSR_CNT_MA_CNT_MA_0_3_SOP_31_0_GET(cnt[0]);

    pal_reg_rd32w(CAP_ADDR_BASE_PT_PT_OFFSET + CAP_PT_CSR_PTD_CNT_NPV_RESUB_BYTE_ADDRESS, cnt, 3);
    int resub_cnt = CAP_PTD_CSR_CNT_NPV_RESUB_CNT_NPV_RESUB_0_3_SOP_31_0_GET(cnt[0]);

    pal_reg_rd32w(CAP_ADDR_BASE_PT_PT_OFFSET + CAP_PT_CSR_PTD_CNT_PB_BYTE_ADDRESS, cnt, 3);
    int pb_cnt = CAP_PTD_CSR_CNT_PB_CNT_PB_0_3_SOP_31_0_GET(cnt[0]);

    // Pending Reads/Writes, # PHVs
    for (i = 0; i < polls; i++) {
        // FIFO Status
        pal_reg_rd32w(CAP_ADDR_BASE_PT_PT_OFFSET + CAP_PT_CSR_PTD_STA_FIFO_BYTE_ADDRESS, &sta_fifo,
                      1);
        rd_ff_full += CAP_PTD_CSR_STA_FIFO_LAT_FF_FULL_GET(sta_fifo);
        rd_ff_empty += CAP_PTD_CSR_STA_FIFO_LAT_FF_EMPTY_GET(sta_fifo);
        wr_ff_full += CAP_PTD_CSR_STA_FIFO_WR_MEM_FF_FULL_GET(sta_fifo);
        wr_ff_empty += CAP_PTD_CSR_STA_FIFO_WR_MEM_FF_EMPTY_GET(sta_fifo);
        pkt_ff_full += CAP_PTD_CSR_STA_FIFO_PKT_FF_FULL_GET(sta_fifo);
        pkt_ff_empty += CAP_PTD_CSR_STA_FIFO_PKT_FF_EMPTY_GET(sta_fifo);
        // Pending:
        pal_reg_rd32w(CAP_ADDR_BASE_PT_PT_OFFSET + CAP_PT_CSR_PTD_STA_ID_BYTE_ADDRESS, &cnt_pend,
                      1);
        pal_reg_rd32w(CAP_ADDR_BASE_PT_PT_OFFSET + CAP_PT_CSR_PTD_STA_XOFF_BYTE_ADDRESS, &sta_xoff,
                      1);
        pend_rd += CAP_PTD_CSR_STA_ID_RD_PEND_CNT_GET(cnt_pend);
        pend_wr += CAP_PTD_CSR_STA_ID_WR_PEND_CNT_GET(cnt_pend);
        num_phv += CAP_PTD_CSR_STA_XOFF_NUMPHV_COUNTER_GET(sta_xoff);
    }

    printf(" phv=%u pkt=%u drop=%u(%u%%) err=%u recirc=%u resub=%u in_flight=%u\n", ma_cnt, pb_cnt,
           phv_drop, (phv_drop * 100) / ma_cnt, phv_err, phv_recirc, resub_cnt, num_phv / polls);

    printf("       AXI reads=%u writes=%u\n", pend_rd / polls, pend_wr / polls);

    printf("       FIFO (empty%%/full%%) rd=%u/%u wr=%u/%u pkt=%u/%u\n",
           (rd_ff_empty * 100) / polls, (rd_ff_full * 100) / polls, (wr_ff_empty * 100) / polls,
           (wr_ff_full * 100) / polls, (pkt_ff_empty * 100) / polls, (pkt_ff_full * 100) / polls);
}

void
ptd_reset_counters(int verbose)
{
    uint32_t zero[4] = {0};

    pal_reg_wr32w(CAP_ADDR_BASE_PT_PT_OFFSET + CAP_PT_CSR_PTD_BYTE_ADDRESS +
                      CAP_PTD_CSR_CNT_PHV_BYTE_OFFSET,
                  zero, 4);
    pal_reg_wr32w(CAP_ADDR_BASE_PT_PT_OFFSET + CAP_PT_CSR_PTD_CNT_MA_BYTE_ADDRESS, zero, 3);
    pal_reg_wr32w(CAP_ADDR_BASE_PT_PT_OFFSET + CAP_PT_CSR_PTD_CNT_NPV_RESUB_BYTE_ADDRESS, zero, 3);
    pal_reg_wr32w(CAP_ADDR_BASE_PT_PT_OFFSET + CAP_PT_CSR_PTD_CNT_PB_BYTE_ADDRESS, zero, 3);
}

void
npv_read_counters(int verbose)
{
    uint64_t ma_cnt, sw_cnt, pb_pbus_cnt, pr_pbus_cnt, ma_drop_cnt, ma_recirc_cnt;

    pal_reg_rd32w(CAP_ADDR_BASE_PT_PT_OFFSET + CAP_PSP_CSR_CNT_MA_SOP_BYTE_ADDRESS,
                  (uint32_t *)&ma_cnt, 2);
    pal_reg_rd32w(CAP_ADDR_BASE_PT_PT_OFFSET + CAP_PSP_CSR_CNT_SW_SOP_BYTE_ADDRESS,
                  (uint32_t *)&sw_cnt, 2);
    pal_reg_rd32w(CAP_ADDR_BASE_PT_PT_OFFSET + CAP_PSP_CSR_CNT_PB_PBUS_SOP_BYTE_ADDRESS,
                  (uint32_t *)&pb_pbus_cnt, 2);
    pal_reg_rd32w(CAP_ADDR_BASE_PT_PT_OFFSET + CAP_PSP_CSR_CNT_PR_PBUS_SOP_BYTE_ADDRESS,
                  (uint32_t *)&pr_pbus_cnt, 2);
    pal_reg_rd32w(CAP_ADDR_BASE_PT_PT_OFFSET + CAP_PSP_CSR_CNT_MA_DROP_BYTE_ADDRESS,
                  (uint32_t *)&ma_drop_cnt, 2);
    pal_reg_rd32w(CAP_ADDR_BASE_PT_PT_OFFSET + CAP_PSP_CSR_CNT_MA_RECIRC_BYTE_ADDRESS,
                  (uint32_t *)&ma_recirc_cnt, 2);

    printf(" NPV: phv=%ld pb_pbus=%ld pr_pbus=%ld sw=%ld phv_drop=%ld recirc=%ld\n", ma_cnt,
           pb_pbus_cnt, pr_pbus_cnt, sw_cnt, ma_drop_cnt, ma_recirc_cnt);
}

void
npv_reset_counters(int verbose)
{
    uint32_t zero[4] = {0};

    pal_reg_wr32w(CAP_ADDR_BASE_PT_PT_OFFSET + CAP_PSP_CSR_CNT_MA_SOP_BYTE_ADDRESS, zero, 2);
    pal_reg_wr32w(CAP_ADDR_BASE_PT_PT_OFFSET + CAP_PSP_CSR_CNT_SW_SOP_BYTE_ADDRESS, zero, 2);
    pal_reg_wr32w(CAP_ADDR_BASE_PT_PT_OFFSET + CAP_PSP_CSR_CNT_PB_PBUS_SOP_BYTE_ADDRESS, zero, 2);
    pal_reg_wr32w(CAP_ADDR_BASE_PT_PT_OFFSET + CAP_PSP_CSR_CNT_PR_PBUS_SOP_BYTE_ADDRESS, zero, 2);
    pal_reg_wr32w(CAP_ADDR_BASE_PT_PT_OFFSET + CAP_PSP_CSR_CNT_MA_DROP_BYTE_ADDRESS, zero, 2);
    pal_reg_wr32w(CAP_ADDR_BASE_PT_PT_OFFSET + CAP_PSP_CSR_CNT_MA_RECIRC_BYTE_ADDRESS, zero, 2);
}

void
doorbell_read_counters(int verbose)
{
    uint64_t host_cnt, local_cnt, sched_cnt;

    // Doorbell Host
    pal_reg_rd32w(CAP_ADDR_BASE_DB_WA_OFFSET + CAP_WA_CSR_CNT_WA_HOST_DOORBELLS_BYTE_ADDRESS,
                  (uint32_t *)&host_cnt, 2);

    // Doorbell Local
    pal_reg_rd32w(CAP_ADDR_BASE_DB_WA_OFFSET + CAP_WA_CSR_CNT_WA_LOCAL_DOORBELLS_BYTE_ADDRESS,
                  (uint32_t *)&local_cnt, 2);

    // Doorbell to Sched
    pal_reg_rd32w(CAP_ADDR_BASE_DB_WA_OFFSET + CAP_WA_CSR_CNT_WA_SCHED_OUT_BYTE_ADDRESS,
                  (uint32_t *)&sched_cnt, 2);

    printf(" Host=%ld Local=%ld Sched=%ld\n", host_cnt, local_cnt, sched_cnt);
}

void
doorbell_reset_counters(int verbose)
{
    uint32_t zero[4] = {0};

    // Doorbell count reset
    pal_reg_wr32w(CAP_ADDR_BASE_TXS_TXS_OFFSET + CAP_TXS_CSR_CNT_SCH_DOORBELL_SET_BYTE_ADDRESS,
                  zero, 2);
    pal_reg_wr32w(CAP_ADDR_BASE_TXS_TXS_OFFSET + CAP_TXS_CSR_CNT_SCH_DOORBELL_CLR_BYTE_ADDRESS,
                  zero, 2);
    // Doorbell Host
    pal_reg_wr32w(CAP_ADDR_BASE_DB_WA_OFFSET + CAP_WA_CSR_CNT_WA_HOST_DOORBELLS_BYTE_ADDRESS, zero,
                  2);
    // Doorbell Local
    pal_reg_wr32w(CAP_ADDR_BASE_DB_WA_OFFSET + CAP_WA_CSR_CNT_WA_LOCAL_DOORBELLS_BYTE_ADDRESS,
                  zero, 2);
    // Doorbell to Sched
    pal_reg_wr32w(CAP_ADDR_BASE_DB_WA_OFFSET + CAP_WA_CSR_CNT_WA_SCHED_OUT_BYTE_ADDRESS, zero, 2);
}

void
txs_read_counters(int verbose)
{
    uint32_t cnt[2];
    uint32_t xoff_vector, cnt_txdma;
    uint32_t xoff[16] = {0};
    int cos, i;
    int polls = 100;

    // Doorbell sets/clears
    pal_reg_rd32w(CAP_ADDR_BASE_TXS_TXS_OFFSET + CAP_TXS_CSR_CNT_SCH_DOORBELL_SET_BYTE_ADDRESS,
                  cnt, 2);
    printf(" Set=%u", cnt[0]);

    pal_reg_rd32w(CAP_ADDR_BASE_TXS_TXS_OFFSET + CAP_TXS_CSR_CNT_SCH_DOORBELL_CLR_BYTE_ADDRESS,
                  cnt, 2);
    printf(" Clear=%u", cnt[0]);

    // Packet Buffer XOFF
    for (i = 0; i < polls; i++) {
        pal_reg_rd32w(CAP_ADDR_BASE_TXS_TXS_OFFSET + CAP_TXS_CSR_STA_GLB_BYTE_OFFSET, &xoff_vector,
                      1);
        for (cos = 0; cos < 16; cos++) {
            xoff[cos] += (((xoff_vector >> cos) & 1) == 1) ? 1 : 0;
        }
    }

    printf(" PB XOFF:");
    for (cos = 0; cos < 16; cos++) {
        printf(" %x%%", (xoff[cos] * 100) / polls);
    }

    // TxDMA PHVs
    printf("\n PHVs:");
    for (cos = 0; cos < 16; cos++) {
        pal_reg_rd32w(CAP_ADDR_BASE_TXS_TXS_OFFSET + CAP_TXS_CSR_CNT_SCH_TXDMA_COS0_ADDRESS +
                          (cos * 8),
                      &cnt_txdma, 1);
        if (cnt_txdma > 0) {
            printf(" [%u]=%u", cos, cnt_txdma);
        }
    }
    printf("\n");
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
