//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file contains the implementation for displaying elba ASIC counters
///
//===----------------------------------------------------------------------===//

#include <stdio.h>
#include <stdlib.h>
#include <curses.h>
#include <string.h>
#include <cstring>
#include "elbmon.hpp"
#include <time.h>

asic_data_t *asic = NULL;

// global char buffer to hold the intermittent elbmon output
char text[BUFFER_LEN] = {};
FILE *fp = NULL;

// Appends each line into the mentioned file
static inline void
elbmon_export_to_text_file()
{
    if (fp == NULL) {
        time_t t = time(NULL);
        struct tm tm = *localtime(&t);
#define LEN 32
        char time_str[LEN] = {};
        char name[LEN * 2] = {};
        snprintf(time_str, LEN, "%d-%d-%d.%d:%d:%d", tm.tm_year + 1900,
                 tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
        snprintf(name, LEN * 2, "%s%s", "/tmp/elbmon.output.", time_str);
        printf("%s %s\n", time_str, name);
        fp = fopen(name, "w");
        if (fp == NULL) {
            printf("Error: Unable to open file:%s. Skipping export.\n", name);
            return;
        }
    }
    fprintf(fp, "%s", text);
}

// 'elbmon -x txt' will create a file in /tmp/elbmon.output.<time> with
// elbmon output
static inline void
elbmon_export_to_file(char *text)
{
    switch (export_format) {
        case EXPORT_TEXT:
            elbmon_export_to_text_file();
            break;
        case EXPORT_JSON:
            break;
        case EXPORT_XML:
            break;
        case EXPORT_HTML:
            break;
        case EXPORT_CSV:
            break;
        default:
            break;
    }
}

static inline void
elbmon_report_internal(char *text)
{
    printf("%s", text);

    // export
    if (export_to_file) {
        elbmon_export_to_file(text);
    }
}

#define ELBMON_REPORT(format, ...)                                             \
    memset(text, 0, sizeof(text));                                             \
    snprintf(text, BUFFER_LEN, format, ##__VA_ARGS__);                         \
    elbmon_report_internal(text)

void *
elbtop_display_routine(void *)
{
    asic->functions.display(NULL);
    return NULL;
}

void *
elbtop_fetch_routine(void *)
{
    return NULL;
}

void *
elbtop_export_routine(void *)
{

    return NULL;
}

#if 0
static inline void elbmon_thread_init()
{
    pthread_t display, fetch;// exp;
    pthread_create(&display, NULL, elbtop_display_routine, NULL);
    pthread_create(&fetch, NULL, elbtop_fetch_routine, NULL);
    //pthread_create(&exp, NULL, elbtop_export_routine, NULL);

    pthread_join(display, NULL);
    pthread_join(fetch, NULL);
    //pthread_join(exp, NULL);
}
#endif

static inline void
elbmon_dma_fetch_fn(void *)
{
}

static inline void
elbmon_dma_display_fn(void *)
{
}

static inline void
elbmon_dma_exp_fn(void *)
{
}

static inline void
elbmon_pipeline_fetch_fn(void *)
{
}

static inline void
elbmon_pipeline_exp_fn(void *)
{
}

static inline void
elbmon_asic_fetch_fn(void *)
{
}

// Display per mpu data
static inline void
elbmon_mpu_display_fn(void *ptr)
{
    mpu_t *mpu = NULL;
    mpu = (mpu_t *)ptr;
    if (verbose) {
        int cycles = mpu->cycles;

        ELBMON_REPORT(" mpu %d cycles=%u", mpu->index, cycles);
        ELBMON_REPORT(" inst=%u", mpu->inst_executed);
        ELBMON_REPORT(" miss=%u", mpu->icache_miss);
        ELBMON_REPORT(" istl=%u", mpu->icache_fill_stall);
        ELBMON_REPORT(" phv=%u", mpu->phv_executed);
        ELBMON_REPORT(" hzrd=%u", mpu->hazard_stall);
        ELBMON_REPORT(" phvwr_stl=%u", mpu->phvwr_stall);
        ELBMON_REPORT(" memwr_stl=%u", mpu->memwr_stall);
        ELBMON_REPORT(" tblwr_stl=%u", mpu->tblwr_stall);
        ELBMON_REPORT(" fence_stl=%u\n", mpu->fence_stall);

        cycles = cycles == 0 ? 1 : cycles;
        ELBMON_REPORT(" mpu %u percentages", mpu->index);
        ELBMON_REPORT(" inst=%u%%", (mpu->inst_executed * 100) / cycles);
        ELBMON_REPORT(" miss=%u%%", (mpu->icache_miss * 100) / cycles);
        ELBMON_REPORT(" istl=%u%%", (mpu->icache_fill_stall * 100) / cycles);
        ELBMON_REPORT(" phv=%u%%", (mpu->phv_executed * 100) / cycles);
        ELBMON_REPORT(" hzrd=%u%%", (mpu->hazard_stall * 100) / cycles);
        ELBMON_REPORT(" phvwr_stl=%u%%", (mpu->phvwr_stall * 100) / cycles);
        ELBMON_REPORT(" memwr_stl=%u%%", (mpu->memwr_stall * 100) / cycles);
        ELBMON_REPORT(" tblwr_stl=%u%%", (mpu->tblwr_stall * 100) / cycles);
        ELBMON_REPORT(" fence_stl=%u%%\n", (mpu->fence_stall * 100) / cycles);
        ELBMON_REPORT("  mpu %u table address = 0x%lx\n", mpu->index,
                      mpu->addr);
    }
}

// Display per mpu data
static inline void
elbmon_mpus_display(stage_t *stage)
{
    mpu_t *mpu = stage->mpus;
    for (int i = 0; i < MPU_COUNT; i++) {
        mpu->functions.display(mpu);
        mpu++;
    }
}

static inline void
elbmon_stage_mpu_basic_display(stage_t *stage)
{
    mpu_t *mpu = NULL;
    for (int i = 0; i < MPU_COUNT; i++) {
        mpu = &stage->mpus[i];
        if (verbose) {
            ELBMON_REPORT("  mpu %u  processing %2d%%, stalls: hazard %2d%% "
                          "phvwr %2d%% tblwr %2d%% memwr "
                          "%2d%%\n",
                          mpu->index, mpu->processing_pc, mpu->stall[3],
                          mpu->stall[2], mpu->stall[1], mpu->stall[0]);
        } else {
            ELBMON_REPORT(" m%1d=%3d%%", i, mpu->processing_pc);
        }
    }
    if (!verbose) {
        ELBMON_REPORT("\n");
    }
}

static inline void
elbmon_stage_phv_display(stage_t *stage)
{
    if (verbose) {
        ELBMON_REPORT("  te phv=%u, axi_rd=%u, tcam=%u, mpu_out=%u\n",
                      stage->te_phv_cnt, stage->te_axi_cnt, stage->te_tcam_cnt,
                      stage->te_mpu_cnt);

        ELBMON_REPORT("  sdp PHV FIFO depth=%u\n", stage->phv_fifo_depth);
        ELBMON_REPORT("  sdp PHV processed count=%u\n",
                      stage->phv_processed_count);
    }
}

// Display per stage data
static inline void
elbmon_stage_display(stage_t *stage)
{
    ELBMON_REPORT(" S%d:", stage->index);
    if (verbose) {
        ELBMON_REPORT("\n");
    }
    ELBMON_REPORT(" (util/xoff/idle) in=%3d/%3d/%3d stg=%3d/%3d/%3d "
                  "out=%3d/%3d/%3d TE=%2u",
                  stage->util.in, stage->xoff.in, stage->idle.in,
                  stage->util.stg, stage->xoff.stg, stage->idle.stg,
                  stage->util.out, stage->xoff.out, stage->idle.out, stage->te);

    if (stage->last_table_type == TABLE_PCI) {
        ELBMON_REPORT(" PCI_lat=");
    } else if (stage->last_table_type == TABLE_SRAM) {
        ELBMON_REPORT(" SRM_lat=");
    } else {
        ELBMON_REPORT(" HBM_lat=");
    }
    ELBMON_REPORT("%5u", stage->_lat);

    if (verbose) {
        ELBMON_REPORT(" min=%u, max=%u\n", stage->min, stage->max);
        ELBMON_REPORT(" phvwr depths");
        mpu_t *mpu = NULL;
        for (int i = 0; i < 4; i++) {
            mpu = &stage->mpus[i];
            ELBMON_REPORT(" m%u=%u,%u", i, (int)(mpu->phv_cmd_depth),
                          (int)(mpu->phv_data_depth));
        }
        ELBMON_REPORT("\n");
    }
}

static inline void
elbmon_stages_display_fn(stage_t *stage)
{
    elbmon_stage_display(stage);

    elbmon_stage_mpu_basic_display(stage);

    elbmon_stage_phv_display(stage);

    elbmon_mpus_display(stage);

    // elbmon_vars_display(stage);
}

// Display all the stages
static inline void
elbmon_stages_display(pipeline_t *pipeline)
{
    stage_t *stage = NULL;
    int count = pipeline->stage_count;

    for (int i = 0; i < count; i++) {
        stage = &pipeline->stages[i];
        elbmon_stages_display_fn(stage);
    }
}

static inline void
elbmon_dma_pipeline_data_display2(pipeline_t *pipeline)
{
    if (pipeline->type == TXDMA) {
        ELBMON_REPORT(" NPV: phv=%lu pb_pbus=%ld pr_pbus=%ld sw=%ld "
                      "phv_drop=%ld recirc=%ld\n",
                      pipeline->phv, pipeline->pb_pbus_cnt,
                      pipeline->pr_pbus_cnt, pipeline->sw_cnt,
                      pipeline->phv_drop_cnt, pipeline->recirc_cnt);
    } else if (pipeline->type == RXDMA) {
        ELBMON_REPORT(" PSP: phv=%lu pb_pbus=%ld pr_pbus=%ld sw=%ld "
                      "phv_drop=%ld recirc=%ld\n",
                      pipeline->phv, pipeline->pb_pbus_cnt,
                      pipeline->pr_pbus_cnt, pipeline->sw_cnt,
                      pipeline->phv_drop_cnt, pipeline->recirc_cnt);
    }
}

static inline void
elbmon_dma_pipeline_display_fn(pipeline_t *pipeline)
{
    int type = pipeline->type;

    switch (type) {
        case TXDMA:
            ELBMON_REPORT("== TXDMA ==\n");
            break;
        case RXDMA:
            ELBMON_REPORT("== RXDMA ==\n");
            break;
        default:
            break;
    }
    elbmon_dma_pipeline_data_display2(pipeline);
}

static inline void
elbmon_p4_pipeline_display_fn(pipeline_t *pipeline)
{
    int type = pipeline->type;

    switch (type) {
        case P4IG:
            ELBMON_REPORT("== P4IG ==\n");
            break;
        case P4EG:
            ELBMON_REPORT("== P4EG ==\n");
            break;
        default:
            break;
    }
    ELBMON_REPORT(" Parser: pkt_from_pb=%ld phv_to_s0=%ld pkt_to_dp=%ld\n",
                  pipeline->pkt_from_pb, pipeline->phv_to_s0,
                  pipeline->pkt_to_dp);
}

static inline void
elbmon_dma_post_stage_display(pipeline_t *pipeline)
{
    if (pipeline->type == TXDMA) {
        ELBMON_REPORT(" TxDMA:");
    } else if (pipeline->type == RXDMA) {
        ELBMON_REPORT(" RxDMA:");
    }
    ELBMON_REPORT(" phv=%ld pkt=%ld drop=%ld(%ld%%) err=%ld recirc=%ld "
                  "resub=%ld in_flight=%ld\n",
                  pipeline->phv, pipeline->pb_cnt, pipeline->phv_drop,
                  (pipeline->phv_drop * 100) / pipeline->phv, pipeline->phv_err,
                  pipeline->phv_recirc, pipeline->resub_cnt,
                  pipeline->in_flight);

    ELBMON_REPORT("       AXI reads=%ld writes=%ld\n", pipeline->axi_reads,
                  pipeline->axi_writes);

    ELBMON_REPORT("       FIFO (empty%%/full%%) rd=%d/%d wr=%d/%d pkt=%d/%d",
                  pipeline->rd_empty_fifos, pipeline->rd_full_fifos,
                  pipeline->wr_empty_fifos, pipeline->wr_full_fifos,
                  pipeline->pkt_empty_fifos, pipeline->pkt_full_fifos);
    if (pipeline->type == RXDMA) {
        ELBMON_REPORT(" ff_depth=%u\n", pipeline->ff_depth);
    } else {
        ELBMON_REPORT("\n");
    }
}

static inline void
elbmon_rxdma_post_stage_display1(pipeline_t *pipeline)
{
    ELBMON_REPORT("       XOFF hostq=%ld pkt=%ld phv=%ld phv_xoff=%d%% "
                  "pb_xoff=%d%% host_xoff=%d%%\n",
                  pipeline->hostq_xoff_cnt, pipeline->pkt_xoff_cnt,
                  pipeline->phv_xoff_cnt, pipeline->host_xoff,
                  pipeline->phv_xoff, pipeline->pb_xoff);
}

static inline void
elbmon_txdma_post_stage_display(pipeline_t *pipeline)
{
    elbmon_dma_post_stage_display(pipeline);
}

static inline void
elbmon_rxdma_post_stage_display(pipeline_t *pipeline)
{
    elbmon_dma_post_stage_display(pipeline);
}

// Display post stage data
static inline void
elbmon_pipeline_post_stage_display(pipeline_t *pipeline)
{
    int type = pipeline->type;
    switch (type) {
        case TXDMA:
            elbmon_txdma_post_stage_display(pipeline);
            break;
        case RXDMA:
            elbmon_rxdma_post_stage_display(pipeline);
            break;
        case P4IG:
        case P4EG:
            break;
            return;
    }
    if (type == RXDMA) {
        elbmon_rxdma_post_stage_display1(pipeline);
    }
}

static inline void
elbmon_pipeline_display_fn(void *ptr)
{
    pipeline_t *pipeline = (pipeline_t *)ptr;

    // Display per pipeline data
    switch (pipeline->type) {
        case TXDMA:
        case RXDMA:
            elbmon_dma_pipeline_display_fn(pipeline);
            break;
        case P4IG:
        case P4EG:
            elbmon_p4_pipeline_display_fn(pipeline);
            break;
        default:
            ELBMON_REPORT("Error: Unknown pipeline type:%d", pipeline->type);
    }

    elbmon_stages_display((pipeline_t *)pipeline);

    elbmon_pipeline_post_stage_display(pipeline);
}

// Display all pipelines
static inline void
elbmon_pipelines_display()
{
    pipeline_t *pipeline = NULL;
    for (int i = 0; i < PIPE_CNT; i++) {
        pipeline = &asic->pipelines[i];
        pipeline->functions.display(pipeline);
    }
}

static inline void
elbmon_asic_display_target_status()
{
    ELBMON_REPORT("  pending [0]=%d\n", asic->target_pending);
}

static inline void
elbmon_asic_display_target_counters()
{
    ELBMON_REPORT("  wr=%lu wr_64=%lu bytes=%lu\n"
                  "  rd=%lu rd_64=%lu bytes=%lu\n"
                  "  db_64=%lu db_32=%lu\n",
                  asic->axi_wr_tgt, asic->axi_wr_64_tgt, asic->axi_wr_bytes_tgt,
                  asic->axi_rd_tgt, asic->axi_rd_64_tgt, asic->axi_rd_bytes_tgt,
                  asic->axi_wr_db64, asic->axi_wr_db32);
}

static inline void
elbmon_asic_display_target_err_counters()
{
    ELBMON_REPORT("  ur_cpl=%ld tlp_drop=%ld rresp_err=%d bresp_err=%d "
                  "ind_cnxt_mismatch=%d\n", asic->ur_cpl,
                  asic->tlp_drop, asic->rresp_err, asic->bresp_err,
                  asic->ind_cnxt_mismatch);
}

static inline void
elbmon_asic_display_initiator_status()
{
    ELBMON_REPORT("  wr_pend=%u rd_pend=%u\n", asic->axi_wr_pend,
                  asic->axi_rd_pend);
    auto rd_total =
        asic->rd_lat0 + asic->rd_lat1 + asic->rd_lat2 + asic->rd_lat3;
    ELBMON_REPORT(
        "  read latency (clks) >%d=%.2f%% >%d=%.2f%% >%d=%.2f%% >%d=%.2f%%\n",
        asic->cfg_rdlat[3], (asic->rd_lat0 * 100.0) / rd_total,
        asic->cfg_rdlat[2], (asic->rd_lat1 * 100.0) / rd_total,
        asic->cfg_rdlat[1], (asic->rd_lat2 * 100.0) / rd_total,
        asic->cfg_rdlat[0], (asic->rd_lat3 * 100.0) / rd_total);
}

static inline void
elbmon_asic_display_initiator_counters()
{
    ELBMON_REPORT("  wr=%lu wr_64=%lu wr_256=%lu bytes=%lu\n", asic->axi_wr,
                  asic->axi_wr64, asic->axi_wr256, asic->axi_wr_bytes);
    ELBMON_REPORT("  rd=%lu rd_64=%lu wr_256=%lu bytes=%lu\n", asic->axi_rd,
                  asic->axi_rd64, asic->axi_rd256, asic->axi_rd_bytes);
    ELBMON_REPORT("  atomic=%ld\n", asic->atomic_req);
}

static inline void
elbmon_asic_display_pport_status()
{
    ELBMON_REPORT("  rx_stl=%.2f%% tx_stl=%.2f%%\n", asic->rx_stl,
                  asic->tx_stl);
}

static inline void
elbmon_asic_display_pport_counters()
{
    ELBMON_REPORT(
        "  rx_req_tlp=%ld rx_cpl_tlp=%ld tx_req_tlp=%ld tx_cpl_tlp=%ld\n",
        asic->rx_req_tlp, asic->rx_cpl_tlp, asic->tx_req_tlp, asic->tx_cpl_tlp);
}

static inline void
elbmon_asic_display_pport_err_counters()
{
    ELBMON_REPORT(
        "  rx_bad_tlp=%ld rx_bad_dllp=%ld rx_nak_rcvd=%ld\n"
        "  rx_nullified=%ld rxbfr_overflow=%ld\n"
        "  tx_nak_sent=%ld txbuf_ecc_err=%ld\n"
        "  fcpe=%ld fc_timeout=%ld replay_num_err=%ld replay_timer_err=%ld\n"
        "  core_initiated_recovery=%d ltssm_state_changed=%d\n"
        "  skp_os_err=%ld deskew_err=%ld phystatus_err=%ld\n"
        "  rx_malform_tlp=%ld rx_framing_err=%ld rx_ecrc_err=%ld\n"
        "  rx_nullify=%ld rx_watchdog_nullify=%ld rx_unsupp=%ld "
        "rxbuf_ecc_err=%ld\n"
        "  tx_drop=%ld txbfr_overflow=%ld\n",
        asic->rx_bad_tlp, asic->rx_bad_dllp, asic->rx_nak_received,
        asic->rx_nullified, asic->rxbfr_overflow, asic->tx_nak_sent,
        asic->txbuf_ecc_err, asic->fcpe, asic->fc_timeout, asic->replay_num_err,
        asic->replay_timer_err, asic->core_initiated_recovery,
        asic->ltssm_state_changed, asic->skp_os_err, asic->deskew_err,
        asic->phystatus_err, asic->rx_malform_tlp, asic->rx_framing_err,
        asic->rx_ecrc_err, asic->rx_nullify, asic->rx_watchdog_nullify,
        asic->rx_unsupp, asic->rxbuf_ecc_err, asic->tx_drop,
        asic->txbfr_overflow);
}

static inline void
elbmon_asic_display_doorbells()
{
    ELBMON_REPORT("== Doorbell ==\n");
    ELBMON_REPORT(" Host=%ld Local=%ld Sched=%ld\n", asic->host_dbs,
                  asic->local_dbs, asic->db_to_sched_reqs);
}

static inline void
elbmon_asic_display_tx_sched()
{
    uint32_t cnt_txdma;
    int cos;
    ELBMON_REPORT("== TX Scheduler ==\n");
    ELBMON_REPORT(" Set=%u", asic->sets);
    ELBMON_REPORT(" Clear=%u", asic->clears);
    ELBMON_REPORT(" PB XOFF:");
    for (cos = 0; cos < 16; cos++) {
        ELBMON_REPORT(" %x%%", asic->xoff[cos]);
    }
    ELBMON_REPORT("\n");
    ELBMON_REPORT(" PHVs:");
    for (cos = 0; cos < 16; cos++) {
        cnt_txdma = asic->phvs[cos];
        if (cnt_txdma > 0) {
            ELBMON_REPORT(" [%u]=%u", cos, cnt_txdma);
        }
    }
    ELBMON_REPORT("\n");
}

static inline void
elbmon_asic_bwmon_display(int index)
{
    bwmon_t *bwmon_data = &asic->bwmons[index];
    ELBMON_REPORT("\n");

    ELBMON_REPORT(" rd_cnt=%12lu", bwmon_data->rd_cnt);
    ELBMON_REPORT(" rd_lat=%8u/%8u", bwmon_data->rd_latency_avg,
                  bwmon_data->rd_latency_max);
    ELBMON_REPORT(" rd_bw=%4u/%4u", bwmon_data->rd_bandwidth_avg,
                  bwmon_data->rd_bandwidth_max);
    ELBMON_REPORT(" rd_pend=%4u, no_drdy=%u", bwmon_data->rd_trans,
                  bwmon_data->rd_trans_no_drdy);

    ELBMON_REPORT("\n");

    ELBMON_REPORT(" wr_cnt=%12lu", bwmon_data->wr_cnt);
    ELBMON_REPORT(" wr_lat=%8u/%8u", bwmon_data->wr_latency_avg,
                  bwmon_data->wr_latency_max);
    ELBMON_REPORT(" wr_bw=%4u/%4u", bwmon_data->wr_bandwidth_avg,
                  bwmon_data->wr_bandwidth_max);
    ELBMON_REPORT(" wr_pend=%4u, no_drdy=%u", bwmon_data->wr_trans,
                  bwmon_data->wr_trans_no_drdy);

    ELBMON_REPORT("\n");
}

static inline void
elbmon_asic_all_bwmon_display()
{
    int index = 0;

    ELBMON_REPORT("==AXI==\n");
    ELBMON_REPORT("PXB:");
    elbmon_asic_bwmon_display(index++);

    ELBMON_REPORT("RXD:");
    elbmon_asic_bwmon_display(index++);

    ELBMON_REPORT("TXD:");
    elbmon_asic_bwmon_display(index++);

    ELBMON_REPORT("SI:");
    elbmon_asic_bwmon_display(index++);

    ELBMON_REPORT("SE:");
    elbmon_asic_bwmon_display(index++);

    ELBMON_REPORT("MS: ");
    elbmon_asic_bwmon_display(index++);
}

static inline void
elbmon_asic_crypto_display()
{
    ELBMON_REPORT("==Crypto==\n");
    ELBMON_REPORT("  Doorbells:\n");
    ELBMON_REPORT("    XTS %ld XTS ENC %ld GCM0 %ld GCM1 %ld PK %ld\n",
                  asic->xts_cnt, asic->xtsenc_cnt, asic->gcm0_cnt,
                  asic->gcm1_cnt, asic->pk_cnt);
}

static inline void
elbmon_asic_display_fn(void *)
{
    // Display bwmon data
    if (bwmon) {
        elbmon_asic_all_bwmon_display();
    }
    // Display crypto
    if (crypto) {
        elbmon_asic_crypto_display();
    }
    ELBMON_REPORT("== PCIe ==\n");
    ELBMON_REPORT(" Target\n");
    elbmon_asic_display_target_status();
    if (verbose) {
        elbmon_asic_display_target_counters();
        elbmon_asic_display_target_err_counters();
    }

    ELBMON_REPORT(" Initiator\n");
    elbmon_asic_display_initiator_status();
    if (verbose) {
        elbmon_asic_display_initiator_counters();
    }

    ELBMON_REPORT(" Port 0\n");
    elbmon_asic_display_pport_status();
    if (verbose) {
        elbmon_asic_display_pport_counters();
        elbmon_asic_display_pport_err_counters();
    }

    elbmon_asic_display_doorbells();
    elbmon_asic_display_tx_sched();

    elbmon_pipelines_display();
}

static inline void
elbmon_asic_exp_fn(void *)
{
}

static inline void
elbmon_dma_init(pipeline_t *dma)
{
    dma->functions.fetch = &elbmon_dma_fetch_fn;
    dma->functions.display = &elbmon_dma_display_fn;
    dma->functions.export_data = &elbmon_dma_exp_fn;
}

static inline void
elbmon_pipeline_init(pipeline_t *pipeline)
{
    pipeline->functions.fetch = &elbmon_pipeline_fetch_fn;
    pipeline->functions.display = &elbmon_pipeline_display_fn;
    pipeline->functions.export_data = &elbmon_pipeline_exp_fn;
}

static inline void
elbmon_mpu_func_init(mpu_t *mpu)
{
    mpu->functions.fetch = NULL;
    mpu->functions.display = &elbmon_mpu_display_fn;
    mpu->functions.export_data = NULL;
}

// Initialize each mpu structure
static inline void
elbmon_mpu_init(stage_t *stage)
{
    mpu_t *mpu = NULL;
    for (int i = 0; i < MPU_COUNT; i++) {
        mpu = &stage->mpus[i];
        mpu->index = i;
        elbmon_mpu_func_init(mpu);
    }
}

// Initialize each stage structure
static inline void
elbmon_stage_init()
{
    pipeline_t *pipeline = asic->pipelines;
    stage_t *stage = NULL;
    for (int i = 0; i < PIPE_CNT; i++) {
        for (int j = 0; j < pipeline->stage_count; j++) {
            stage = &pipeline->stages[j];
            stage->index = j;
            elbmon_mpu_init(stage);
        }
        pipeline++;
    }
}

// Initialize memory for all stage structures
static inline void
elbmon_asic_stage_mem_init()
{
    pipeline_t *pipeline = NULL;

    pipeline = &asic->pipelines[TXDMA];
    pipeline->stage_count = STAGE_COUNT_DMA;
    pipeline->type = TXDMA;
    pipeline->stages = (stage_t *)calloc(1, sizeof(stage_t) * STAGE_COUNT_DMA);

    pipeline = &asic->pipelines[RXDMA];
    pipeline->stage_count = STAGE_COUNT_DMA;
    pipeline->type = RXDMA;
    pipeline->stages = (stage_t *)calloc(1, sizeof(stage_t) * STAGE_COUNT_DMA);

    pipeline = &asic->pipelines[P4IG];
    pipeline->stage_count = STAGE_COUNT_P4;
    pipeline->type = P4IG;
    pipeline->stages = (stage_t *)calloc(1, sizeof(stage_t) * STAGE_COUNT_P4);

    pipeline = &asic->pipelines[P4EG];
    pipeline->stage_count = STAGE_COUNT_P4;
    pipeline->type = P4EG;
    pipeline->stages = (stage_t *)calloc(1, sizeof(stage_t) * STAGE_COUNT_P4);

    elbmon_stage_init();
}

// Initialize memory for all asic and nested structures
static inline void
elbmon_asic_mem_init()
{
    asic = (asic_data_t *)calloc(1, sizeof(asic_data_t));

    elbmon_asic_stage_mem_init();
}

// Do asic init
static inline void *
elbmon_asic_init(void *)
{
    elbmon_asic_mem_init();

    asic->functions.fetch = &elbmon_asic_fetch_fn;
    asic->functions.display = &elbmon_asic_display_fn;
    asic->functions.export_data = &elbmon_asic_exp_fn;

    elbmon_pipeline_init(&asic->pipelines[TXDMA]);
    elbmon_pipeline_init(&asic->pipelines[RXDMA]);
    elbmon_pipeline_init(&asic->pipelines[P4IG]);
    elbmon_pipeline_init(&asic->pipelines[P4EG]);
    return NULL;
}
void *
elbmon_struct_init(void *)
{
    elbmon_asic_init(NULL);

    // elbmon_thread_init();
    return NULL;
}
