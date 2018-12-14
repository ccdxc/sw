//
// {C} Copyright 2018 Pensando Systems Inc. All rights reserved
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file contains the implementation for displaying capri ASIC counters
///
//===----------------------------------------------------------------------===//

#include <stdio.h>
#include <stdlib.h>
#include <curses.h>
#include <string.h>
#include <cstring>
#include "capmon.hpp"
#include <time.h>

asic_data_t *asic = NULL;

// global char buffer to hold the intermittent capmon output
char text[BUFFER_LEN] = {};
FILE *fp = NULL;

// Appends each line into the mentioned file
static inline void
capmon_export_to_text_file()
{
    if (fp == NULL) {
        time_t t = time(NULL);
        struct tm tm = *localtime(&t);
#define LEN 32
        char time_str[LEN] = {};
        char name[LEN * 2] = {};
        snprintf(time_str, LEN, "%d-%d-%d.%d:%d:%d", tm.tm_year + 1900,
                 tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
        snprintf(name, LEN * 2, "%s%s", "/tmp/capmon.output.", time_str);
        printf("%s %s\n", time_str, name);
        fp = fopen(name, "w");
        if (fp == NULL) {
            printf("Error: Unable to open file:%s. Skipping export.\n", name);
            return;
        }
    }
    fprintf(fp, "%s", text);
}

// 'capmon -x txt' will create a file in /tmp/capmon.output.<time> with
// capmon output
static inline void
capmon_export_to_file(char *text)
{
    switch (export_format) {
        case EXPORT_TEXT:
            capmon_export_to_text_file();
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
capmon_report_internal(char *text)
{
    printf("%s", text);

    // export
    if (export_to_file) {
        capmon_export_to_file(text);
    }
}

#define CAPMON_REPORT(format, ...)                                             \
    memset(text, 0, sizeof(text));                                             \
    snprintf(text, BUFFER_LEN, format, ##__VA_ARGS__);                         \
    capmon_report_internal(text)

void *
captop_display_routine(void *)
{
    asic->functions.display(NULL);
    return NULL;
}

void *
captop_fetch_routine(void *)
{
    return NULL;
}

void *
captop_export_routine(void *)
{

    return NULL;
}

#if 0
static inline void capmon_thread_init()
{
    pthread_t display, fetch;// exp;
    pthread_create(&display, NULL, captop_display_routine, NULL);
    pthread_create(&fetch, NULL, captop_fetch_routine, NULL);
    //pthread_create(&exp, NULL, captop_export_routine, NULL);

    pthread_join(display, NULL);
    pthread_join(fetch, NULL);
    //pthread_join(exp, NULL);
}
#endif

static inline void
capmon_dma_fetch_fn(void *)
{
}

static inline void
capmon_dma_display_fn(void *)
{
}

static inline void
capmon_dma_exp_fn(void *)
{
}

static inline void
capmon_pipeline_fetch_fn(void *)
{
}

static inline void
capmon_pipeline_exp_fn(void *)
{
}

static inline void
capmon_asic_fetch_fn(void *)
{
}

// Display per mpu data
static inline void
capmon_mpu_display_fn(void *ptr)
{
    mpu_t *mpu = NULL;
    mpu = (mpu_t *)ptr;
    if (verbose) {
        int cycles = mpu->cycles;

        CAPMON_REPORT(" mpu %d cycles=%u", mpu->index, cycles);
        CAPMON_REPORT(" inst=%u", mpu->inst_executed);
        CAPMON_REPORT(" miss=%u", mpu->icache_miss);
        CAPMON_REPORT(" istl=%u", mpu->icache_fill_stall);
        CAPMON_REPORT(" phv=%u", mpu->phv_executed);
        CAPMON_REPORT(" hzrd=%u", mpu->hazard_stall);
        CAPMON_REPORT(" phvwr_stl=%u", mpu->phvwr_stall);
        CAPMON_REPORT(" memwr_stl=%u", mpu->memwr_stall);
        CAPMON_REPORT(" tblwr_stl=%u", mpu->tblwr_stall);
        CAPMON_REPORT(" fence_stl=%u\n", mpu->fence_stall);

        cycles = cycles == 0 ? 1 : cycles;
        CAPMON_REPORT(" mpu %u percentages", mpu->index);
        CAPMON_REPORT(" inst=%u%%", (mpu->inst_executed * 100) / cycles);
        CAPMON_REPORT(" miss=%u%%", (mpu->icache_miss * 100) / cycles);
        CAPMON_REPORT(" istl=%u%%", (mpu->icache_fill_stall * 100) / cycles);
        CAPMON_REPORT(" phv=%u%%", (mpu->phv_executed * 100) / cycles);
        CAPMON_REPORT(" hzrd=%u%%", (mpu->hazard_stall * 100) / cycles);
        CAPMON_REPORT(" phvwr_stl=%u%%", (mpu->phvwr_stall * 100) / cycles);
        CAPMON_REPORT(" memwr_stl=%u%%", (mpu->memwr_stall * 100) / cycles);
        CAPMON_REPORT(" tblwr_stl=%u%%", (mpu->tblwr_stall * 100) / cycles);
        CAPMON_REPORT(" fence_stl=%u%%\n", (mpu->fence_stall * 100) / cycles);
        CAPMON_REPORT("  mpu %u table address = 0x%lx\n", mpu->index,
                      mpu->addr);
    }
}

// Display per mpu data
static inline void
capmon_mpus_display(stage_t *stage)
{
    mpu_t *mpu = stage->mpus;
    for (int i = 0; i < MPU_COUNT; i++) {
        mpu->functions.display(mpu);
        mpu++;
    }
}

static inline void
capmon_stage_mpu_basic_display(stage_t *stage)
{
    mpu_t *mpu = NULL;
    for (int i = 0; i < MPU_COUNT; i++) {
        mpu = &stage->mpus[i];
        if (verbose) {
            CAPMON_REPORT("  mpu %u  processing %2d%%, stalls: hazard %2d%% "
                          "phvwr %2d%% tblwr %2d%% memwr "
                          "%2d%%\n",
                          mpu->index, mpu->processing_pc, mpu->stall[3],
                          mpu->stall[2], mpu->stall[1], mpu->stall[0]);
        } else {
            CAPMON_REPORT(" m%1d=%3d%%", i, mpu->processing_pc);
        }
    }
    if (!verbose) {
        CAPMON_REPORT("\n");
    }
}

static inline void
capmon_stage_phv_display(stage_t *stage)
{
    if (verbose) {
        CAPMON_REPORT("  te phv=%u, axi_rd=%u, tcam=%u, mpu_out=%u\n",
                      stage->te_phv_cnt, stage->te_axi_cnt, stage->te_tcam_cnt,
                      stage->te_mpu_cnt);

        CAPMON_REPORT("  sdp PHV FIFO depth=%u\n", stage->phv_fifo_depth);
        CAPMON_REPORT("  sdp PHV processed count=%u\n",
                      stage->phv_processed_count);
    }
}

// Display per stage data
static inline void
capmon_stage_display(stage_t *stage)
{
    CAPMON_REPORT(" S%d:", stage->index);
    if (verbose) {
        CAPMON_REPORT("\n");
    }
    CAPMON_REPORT(" (util/xoff/idle) in=%3d/%3d/%3d stg=%3d/%3d/%3d "
                  "out=%3d/%3d/%3d TE=%2u",
                  stage->util.in, stage->xoff.in, stage->idle.in,
                  stage->util.stg, stage->xoff.stg, stage->idle.stg,
                  stage->util.out, stage->xoff.out, stage->idle.out, stage->te);

    if (stage->last_table_type == TABLE_PCI) {
        CAPMON_REPORT(" PCI_lat=");
    } else if (stage->last_table_type == TABLE_SRAM) {
        CAPMON_REPORT(" SRM_lat=");
    } else {
        CAPMON_REPORT(" HBM_lat=");
    }
    CAPMON_REPORT("%5u", stage->_lat);

    if (verbose) {
        CAPMON_REPORT(" min=%u, max=%u\n", stage->min, stage->max);
        CAPMON_REPORT(" phvwr depths");
        mpu_t *mpu = NULL;
        for (int i = 0; i < 4; i++) {
            mpu = &stage->mpus[i];
            CAPMON_REPORT(" m%u=%u,%u", i, (int)(mpu->phv_cmd_depth),
                          (int)(mpu->phv_data_depth));
        }
        CAPMON_REPORT("\n");
    }
}

static inline void
capmon_stages_display_fn(stage_t *stage)
{
    capmon_stage_display(stage);

    capmon_stage_mpu_basic_display(stage);

    capmon_stage_phv_display(stage);

    capmon_mpus_display(stage);

    // capmon_vars_display(stage);
}

// Display all the stages
static inline void
capmon_stages_display(pipeline_t *pipeline)
{
    stage_t *stage = NULL;
    int count = pipeline->stage_count;

    for (int i = 0; i < count; i++) {
        stage = &pipeline->stages[i];
        capmon_stages_display_fn(stage);
    }
}

static inline void
capmon_dma_pipeline_data_display2(pipeline_t *pipeline)
{
    if (pipeline->type == TXDMA) {
        CAPMON_REPORT(" NPV: phv=%lu pb_pbus=%ld pr_pbus=%ld sw=%ld "
                      "phv_drop=%ld recirc=%ld\n",
                      pipeline->phv, pipeline->pb_pbus_cnt,
                      pipeline->pr_pbus_cnt, pipeline->sw_cnt,
                      pipeline->phv_drop_cnt, pipeline->recirc_cnt);
    } else if (pipeline->type == RXDMA) {
        CAPMON_REPORT(" PSP: phv=%lu pb_pbus=%ld pr_pbus=%ld sw=%ld "
                      "phv_drop=%ld recirc=%ld\n",
                      pipeline->phv, pipeline->pb_pbus_cnt,
                      pipeline->pr_pbus_cnt, pipeline->sw_cnt,
                      pipeline->phv_drop_cnt, pipeline->recirc_cnt);
    }
}

static inline void
capmon_dma_pipeline_display_fn(pipeline_t *pipeline)
{
    int type = pipeline->type;

    switch (type) {
        case TXDMA:
            CAPMON_REPORT("== TXDMA ==\n");
            break;
        case RXDMA:
            CAPMON_REPORT("== RXDMA ==\n");
            break;
        default:
            break;
    }
    capmon_dma_pipeline_data_display2(pipeline);
}

static inline void
capmon_p4_pipeline_display_fn(pipeline_t *pipeline)
{
    int type = pipeline->type;

    switch (type) {
        case P4IG:
            CAPMON_REPORT("== P4IG ==\n");
            break;
        case P4EG:
            CAPMON_REPORT("== P4EG ==\n");
            break;
        default:
            break;
    }
    CAPMON_REPORT(" Parser: pkt_from_pb=%ld phv_to_s0=%ld pkt_to_dp=%ld\n",
                  pipeline->pkt_from_pb, pipeline->phv_to_s0,
                  pipeline->pkt_to_dp);
}

static inline void
capmon_dma_post_stage_display(pipeline_t *pipeline)
{
    if (pipeline->type == TXDMA) {
        CAPMON_REPORT(" TxDMA:");
    } else if (pipeline->type == RXDMA) {
        CAPMON_REPORT(" RxDMA:");
    }
    CAPMON_REPORT(" phv=%ld pkt=%ld drop=%ld(%ld%%) err=%ld recirc=%ld "
                  "resub=%ld in_flight=%ld\n",
                  pipeline->phv, pipeline->pb_cnt, pipeline->phv_drop,
                  (pipeline->phv_drop * 100) / pipeline->phv, pipeline->phv_err,
                  pipeline->phv_recirc, pipeline->resub_cnt,
                  pipeline->in_flight);

    CAPMON_REPORT("       AXI reads=%ld writes=%ld\n", pipeline->axi_reads,
                  pipeline->axi_writes);

    CAPMON_REPORT("       FIFO (empty%%/full%%) rd=%d/%d wr=%d/%d pkt=%d/%d",
                  pipeline->rd_empty_fifos, pipeline->rd_full_fifos,
                  pipeline->wr_empty_fifos, pipeline->wr_full_fifos,
                  pipeline->pkt_empty_fifos, pipeline->pkt_full_fifos);
    if (pipeline->type == RXDMA) {
        CAPMON_REPORT(" ff_depth=%u\n", pipeline->ff_depth);
    } else {
        CAPMON_REPORT("\n");
    }
}

static inline void
capmon_rxdma_post_stage_display1(pipeline_t *pipeline)
{
    CAPMON_REPORT("       XOFF hostq=%ld pkt=%ld phv=%ld phv_xoff=%d%% "
                  "pb_xoff=%d%% host_xoff=%d%%\n",
                  pipeline->hostq_xoff_cnt, pipeline->pkt_xoff_cnt,
                  pipeline->phv_xoff_cnt, pipeline->host_xoff,
                  pipeline->phv_xoff, pipeline->pb_xoff);
}

static inline void
capmon_txdma_post_stage_display(pipeline_t *pipeline)
{
    capmon_dma_post_stage_display(pipeline);
}

static inline void
capmon_rxdma_post_stage_display(pipeline_t *pipeline)
{
    capmon_dma_post_stage_display(pipeline);
}

// Display post stage data
static inline void
capmon_pipeline_post_stage_display(pipeline_t *pipeline)
{
    int type = pipeline->type;
    switch (type) {
        case TXDMA:
            capmon_txdma_post_stage_display(pipeline);
            break;
        case RXDMA:
            capmon_rxdma_post_stage_display(pipeline);
            break;
        case P4IG:
        case P4EG:
            break;
            return;
    }
    if (type == RXDMA) {
        capmon_rxdma_post_stage_display1(pipeline);
    }
}

static inline void
capmon_pipeline_display_fn(void *ptr)
{
    pipeline_t *pipeline = (pipeline_t *)ptr;

    // Display per pipeline data
    switch (pipeline->type) {
        case TXDMA:
        case RXDMA:
            capmon_dma_pipeline_display_fn(pipeline);
            break;
        case P4IG:
        case P4EG:
            capmon_p4_pipeline_display_fn(pipeline);
            break;
        default:
            CAPMON_REPORT("Error: Unknown pipeline type:%d", pipeline->type);
    }

    capmon_stages_display((pipeline_t *)pipeline);

    capmon_pipeline_post_stage_display(pipeline);
}

// Display all pipelines
static inline void
capmon_pipelines_display()
{
    pipeline_t *pipeline = NULL;
    for (int i = 0; i < PIPE_CNT; i++) {
        pipeline = &asic->pipelines[i];
        pipeline->functions.display(pipeline);
    }
}

static inline void
capmon_asic_display_target_status()
{
    CAPMON_REPORT("  pending [0]=%d\n", asic->target_pending);
}

static inline void
capmon_asic_display_target_counters()
{
    CAPMON_REPORT("  wr=%lu wr_64=%lu bytes=%lu\n"
                  "  rd=%lu rd_64=%lu bytes=%lu\n"
                  "  db_64=%lu db_32=%lu\n",
                  asic->axi_wr_tgt, asic->axi_wr_64_tgt, asic->axi_wr_bytes_tgt,
                  asic->axi_rd_tgt, asic->axi_rd_64_tgt, asic->axi_rd_bytes_tgt,
                  asic->axi_wr_db64, asic->axi_wr_db32);
}

static inline void
capmon_asic_display_target_err_counters()
{
    CAPMON_REPORT("  ur_cpl=%ld tlp_drop=%ld rsp_err=%ld\n", asic->ur_cpl,
                  asic->tlp_drop, asic->rsp_err);
}

static inline void
capmon_asic_display_initiator_status()
{
    CAPMON_REPORT("  wr_pend=%u rd_pend=%u\n", asic->axi_wr_pend,
                  asic->axi_rd_pend);
    auto rd_total =
        asic->rd_lat0 + asic->rd_lat1 + asic->rd_lat2 + asic->rd_lat3;
    CAPMON_REPORT(
        "  read latency (clks) >%d=%.2f%% >%d=%.2f%% >%d=%.2f%% >%d=%.2f%%\n",
        asic->cfg_rdlat[3], (asic->rd_lat0 * 100.0) / rd_total,
        asic->cfg_rdlat[2], (asic->rd_lat1 * 100.0) / rd_total,
        asic->cfg_rdlat[1], (asic->rd_lat2 * 100.0) / rd_total,
        asic->cfg_rdlat[0], (asic->rd_lat3 * 100.0) / rd_total);
}

static inline void
capmon_asic_display_initiator_counters()
{
    CAPMON_REPORT("  wr=%lu wr_64=%lu wr_256=%lu bytes=%lu\n", asic->axi_wr,
                  asic->axi_wr64, asic->axi_wr256, asic->axi_wr_bytes);
    CAPMON_REPORT("  rd=%lu rd_64=%lu wr_256=%lu bytes=%lu\n", asic->axi_rd,
                  asic->axi_rd64, asic->axi_rd256, asic->axi_rd_bytes);
    CAPMON_REPORT("  atomic=%ld\n", asic->atomic_req);
}

static inline void
capmon_asic_display_pport_status()
{
    CAPMON_REPORT("  rx_stl=%.2f%% tx_stl=%.2f%%\n", asic->rx_stl,
                  asic->tx_stl);
}

static inline void
capmon_asic_display_pport_counters()
{
    CAPMON_REPORT(
        "  rx_req_tlp=%ld rx_cpl_tlp=%ld tx_req_tlp=%ld tx_cpl_tlp=%ld\n",
        asic->rx_req_tlp, asic->rx_cpl_tlp, asic->tx_req_tlp, asic->tx_cpl_tlp);
}

static inline void
capmon_asic_display_pport_err_counters()
{
    CAPMON_REPORT(
        "  rx_bad_tlp=%ld rx_bad_dllp=%ld rx_nak_rcvd=%ld\n"
        "  rx_nullified=%ld rxbfr_overflow=%ld\n"
        "  tx_nak_sent=%ld txbuf_ecc_err=%ld\n"
        "  fcpe=%ld fc_timeout=%ld replay_num_err=%ld replay_timer_err=%ld\n"
        "  core_initiated_recovery=%ld ltssm_state_changed=%ld\n"
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
capmon_asic_display_doorbells()
{
    CAPMON_REPORT("== Doorbell ==\n");
    CAPMON_REPORT(" Host=%ld Local=%ld Sched=%ld\n", asic->host_dbs,
                  asic->local_dbs, asic->db_to_sched_reqs);
}

static inline void
capmon_asic_display_tx_sched()
{
    uint32_t cnt_txdma;
    int cos;
    CAPMON_REPORT("== TX Scheduler ==\n");
    CAPMON_REPORT(" Set=%u", asic->sets);
    CAPMON_REPORT(" Clear=%u", asic->clears);
    CAPMON_REPORT(" PB XOFF:");
    for (cos = 0; cos < 16; cos++) {
        CAPMON_REPORT(" %x%%", asic->xoff[cos]);
    }
    CAPMON_REPORT("\n");
    CAPMON_REPORT(" PHVs:");
    for (cos = 0; cos < 16; cos++) {
        cnt_txdma = asic->phvs[cos];
        if (cnt_txdma > 0) {
            CAPMON_REPORT(" [%u]=%u", cos, cnt_txdma);
        }
    }
    CAPMON_REPORT("\n");
}

static inline void
capmon_asic_bwmon_display(int index)
{
    bwmon_t *bwmon_data = &asic->bwmons[index];
    CAPMON_REPORT("\n");

    CAPMON_REPORT(" rd_cnt=%12lu", bwmon_data->rd_cnt);
    CAPMON_REPORT(" rd_lat=%8u/%8u", bwmon_data->rd_latency_avg,
                  bwmon_data->rd_latency_max);
    CAPMON_REPORT(" rd_bw=%4u/%4u", bwmon_data->rd_bandwidth_avg,
                  bwmon_data->rd_bandwidth_max);
    CAPMON_REPORT(" rd_pend=%4u, no_drdy=%u", bwmon_data->rd_trans,
                  bwmon_data->rd_trans_no_drdy);

    CAPMON_REPORT("\n");

    CAPMON_REPORT(" wr_cnt=%12lu", bwmon_data->wr_cnt);
    CAPMON_REPORT(" wr_lat=%8u/%8u", bwmon_data->wr_latency_avg,
                  bwmon_data->wr_latency_max);
    CAPMON_REPORT(" wr_bw=%4u/%4u", bwmon_data->wr_bandwidth_avg,
                  bwmon_data->wr_bandwidth_max);
    CAPMON_REPORT(" wr_pend=%4u, no_drdy=%u", bwmon_data->wr_trans,
                  bwmon_data->wr_trans_no_drdy);

    CAPMON_REPORT("\n");
}

static inline void
capmon_asic_all_bwmon_display()
{
    int index = 0;

    CAPMON_REPORT("==AXI==\n");
    CAPMON_REPORT("PXB:");
    capmon_asic_bwmon_display(index++);

    CAPMON_REPORT("RXD:");
    capmon_asic_bwmon_display(index++);

    CAPMON_REPORT("TXD:");
    capmon_asic_bwmon_display(index++);

    CAPMON_REPORT("SI:");
    capmon_asic_bwmon_display(index++);

    CAPMON_REPORT("SE:");
    capmon_asic_bwmon_display(index++);

    CAPMON_REPORT("MS: ");
    capmon_asic_bwmon_display(index++);
}

static inline void
capmon_asic_crypto_display()
{
    CAPMON_REPORT("==Crypto==\n");
    CAPMON_REPORT("  Doorbells:\n");
    CAPMON_REPORT("    XTS %ld XTS ENC %ld GCM0 %ld GCM1 %ld PK %ld\n",
                  asic->xts_cnt, asic->xtsenc_cnt, asic->gcm0_cnt,
                  asic->gcm1_cnt, asic->pk_cnt);
}

static inline void
capmon_asic_display_fn(void *)
{
    // Display bwmon data
    if (bwmon) {
        capmon_asic_all_bwmon_display();
    }
    // Display crypto
    if (crypto) {
        capmon_asic_crypto_display();
    }
    CAPMON_REPORT("== PCIe ==\n");
    CAPMON_REPORT(" Target\n");
    capmon_asic_display_target_status();
    if (verbose) {
        capmon_asic_display_target_counters();
        capmon_asic_display_target_err_counters();
    }

    CAPMON_REPORT(" Initiator\n");
    capmon_asic_display_initiator_status();
    if (verbose) {
        capmon_asic_display_initiator_counters();
    }

    CAPMON_REPORT(" Port 0\n");
    capmon_asic_display_pport_status();
    if (verbose) {
        capmon_asic_display_pport_counters();
        capmon_asic_display_pport_err_counters();
    }

    capmon_asic_display_doorbells();
    capmon_asic_display_tx_sched();

    capmon_pipelines_display();
}

static inline void
capmon_asic_exp_fn(void *)
{
}

static inline void
capmon_dma_init(pipeline_t *dma)
{
    dma->functions.fetch = &capmon_dma_fetch_fn;
    dma->functions.display = &capmon_dma_display_fn;
    dma->functions.export_data = &capmon_dma_exp_fn;
}

static inline void
capmon_pipeline_init(pipeline_t *pipeline)
{
    pipeline->functions.fetch = &capmon_pipeline_fetch_fn;
    pipeline->functions.display = &capmon_pipeline_display_fn;
    pipeline->functions.export_data = &capmon_pipeline_exp_fn;
}

static inline void
capmon_mpu_func_init(mpu_t *mpu)
{
    mpu->functions.fetch = NULL;
    mpu->functions.display = &capmon_mpu_display_fn;
    mpu->functions.export_data = NULL;
}

// Initialize each mpu structure
static inline void
capmon_mpu_init(stage_t *stage)
{
    mpu_t *mpu = NULL;
    for (int i = 0; i < MPU_COUNT; i++) {
        mpu = &stage->mpus[i];
        mpu->index = i;
        capmon_mpu_func_init(mpu);
    }
}

// Initialize each stage structure
static inline void
capmon_stage_init()
{
    pipeline_t *pipeline = asic->pipelines;
    stage_t *stage = NULL;
    for (int i = 0; i < PIPE_CNT; i++) {
        for (int j = 0; j < pipeline->stage_count; j++) {
            stage = &pipeline->stages[j];
            stage->index = j;
            capmon_mpu_init(stage);
        }
        pipeline++;
    }
}

// Initialize memory for all stage structures
static inline void
capmon_asic_stage_mem_init()
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

    capmon_stage_init();
}

// Initialize memory for all asic and nested structures
static inline void
capmon_asic_mem_init()
{
    asic = (asic_data_t *)calloc(1, sizeof(asic_data_t));

    capmon_asic_stage_mem_init();
}

// Do asic init
static inline void *
capmon_asic_init(void *)
{
    capmon_asic_mem_init();

    asic->functions.fetch = &capmon_asic_fetch_fn;
    asic->functions.display = &capmon_asic_display_fn;
    asic->functions.export_data = &capmon_asic_exp_fn;

    capmon_pipeline_init(&asic->pipelines[TXDMA]);
    capmon_pipeline_init(&asic->pipelines[RXDMA]);
    capmon_pipeline_init(&asic->pipelines[P4IG]);
    capmon_pipeline_init(&asic->pipelines[P4EG]);
    return NULL;
}
void *
capmon_struct_init(void *)
{
    capmon_asic_init(NULL);

    // capmon_thread_init();
    return NULL;
}
