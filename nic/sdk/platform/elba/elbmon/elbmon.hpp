//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file contains the decl. and defn. for elbmon funcs and structs
/// asic, pipeline, stage, mpu structs that contain the counters
///
//===----------------------------------------------------------------------===//

#ifndef ELBMON_H
#define ELBMON_H

#include <stdlib.h>
#include <unistd.h>
#include <cstring>
#include <pthread.h>
#include <stdio.h>
#include "nic/sdk/platform/elba/elbmon/dtls.hpp"

#define STAGE_COUNT_P4 6
#define STAGE_COUNT_DMA 8

#define BUFFER_LEN 512
#define NUM_OF_LINES 6
#define FIRST_COL 0
#define FIRST_ROW 0
#define SECOND_ROW FIRST_ROW + 1

#define LINES1 150
#define COLS1 150
#define ROW_LEN 80

typedef enum export_format_ {
    EXPORT_NONE,
    EXPORT_CSV,
    EXPORT_JSON,
    EXPORT_XML,
    EXPORT_HTML,
    EXPORT_TEXT,
} export_format_t;

typedef struct cmd_line_args_ {
    int display_interval;
    int export_interval;
    export_format_t export_format;
} cmd_line_args_t;

extern bool top_like_ui;
typedef struct functions_ {
    void (*display)(void *);
    void (*fetch)(void *);
    void (*export_data)(void *);
} functions_t;

typedef struct _var_ {
    uint8_t in;
    uint8_t stg;
    uint8_t out;
} _var_t;

#define STALL_COUNT 4
typedef struct mpu_ {
    // processing  0%, stalls: hazard  0% phvwr  0% tblwr  0% memwr  0%
    uint8_t index;
    uint8_t processing_pc;
    uint8_t stall[4];
    uint8_t phv_data_depth;
    uint8_t phv_cmd_depth;

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

    uint8_t inst_executed_pc;
    uint8_t icache_miss_pc;
    uint8_t icache_fill_stall_pc;
    uint8_t phv_executed_pc;
    uint8_t hazard_stall_pc;
    uint8_t phvwr_stall_pc;
    uint8_t memwr_stall_pc;
    uint8_t tblwr_stall_pc;
    uint8_t fence_stall_pc;
    uint64_t addr;

    functions_t functions;
} mpu_t;

#define MPU_COUNT 4
// stage
typedef struct stage_ {
    uint8_t srdy;
    uint8_t drdy;
    uint32_t te;
    uint8_t last_table_type;
    uint32_t _lat;
    uint32_t min;
    uint32_t max;
    uint32_t m[MPU_COUNT];
    mpu_t mpus[MPU_COUNT];
    uint32_t te_phv_cnt;
    uint32_t te_axi_cnt;
    uint32_t te_tcam_cnt;
    uint32_t te_mpu_cnt;
    uint32_t phv_fifo_depth;
    uint32_t phv_processed_count;
    _var_t util, xoff, idle;
    int index;

    functions_t functions;
} stage_t;

/*
 * Struct for P4 ingress/egress pipelines
 */
typedef struct pipeline_ {
    uint8_t type;
    /* Types for dma_pipeline_type. */

    uint64_t ma_cnt;
    uint64_t pb_cnt;
    uint64_t phv_drop;
    uint64_t phv_err;
    uint64_t phv_recirc;
    uint64_t resub_cnt;
    uint64_t num_phv;
    uint64_t in_flight;
    uint64_t axi_reads;
    uint64_t axi_writes;
    // uint8_t phvs_in_p4;
    uint8_t rd_empty_fifos;
    uint8_t rd_full_fifos;
    uint8_t wr_empty_fifos;
    uint8_t wr_full_fifos;
    uint8_t pkt_empty_fifos;
    uint8_t pkt_full_fifos;
    uint8_t ff_depth;
    uint8_t xoff;

    uint64_t phv; // ma_cnt
    uint64_t pb_pbus_cnt;
    uint64_t pr_pbus_cnt;
    uint64_t sw_cnt;
    uint64_t phv_drop_cnt;
    uint64_t recirc_cnt;

    /* RxDMA */
    uint64_t hostq_xoff_cnt;
    uint64_t pkt_xoff_cnt;
    uint64_t phv_xoff_cnt;
    uint8_t host_xoff;
    uint8_t phv_xoff;
    uint8_t pb_xoff;

    /* Types for pipeline_type. */
    uint64_t pkt_from_pb;
    uint64_t phv_to_s0;
    uint64_t pkt_to_dp;

    uint8_t stage_count;
    // stage_t stages[stage_count];
    stage_t *stages;

    functions_t functions;
} pipeline_t;

typedef struct bwmon_ {
    /* bwmon */
    uint32_t rd_latency_avg;
    uint32_t rd_latency_max;
    uint32_t rd_bandwidth_avg;
    uint32_t rd_bandwidth_max;
    uint32_t rd_trans;
    uint32_t rd_trans_no_drdy;
    uint64_t rd_cnt;

    uint32_t wr_latency_avg;
    uint32_t wr_latency_max;
    uint32_t wr_bandwidth_avg;
    uint32_t wr_bandwidth_max;
    uint32_t wr_trans;
    uint32_t wr_trans_no_drdy;
    uint64_t wr_cnt;
} bwmon_t;

typedef struct asic_data_ {
    /*
       == PCIe ==
       Target
       pending [0]=0
       wr=2984 wr_64=2984 bytes=4196
       rd=5857 rd_64=5857 bytes=11072
       db_64=0 db_32=0
       ur_cpl=19305 tlp_drop=0 rsp_err=3294239916032
       */
    uint32_t target_pending;
    uint64_t axi_wr_tgt, axi_wr_64_tgt, axi_wr_bytes_tgt;
    uint64_t axi_rd_tgt, axi_rd_64_tgt, axi_rd_bytes_tgt;
    uint64_t axi_wr_db64, axi_wr_db32;

    uint64_t ur_cpl, tlp_drop;
    uint8_t rresp_err, bresp_err, ind_cnxt_mismatch;

    /*
    Initiator
  wr_pend=0 rd_pend=0
  read latency (clks) >0=nan% >250=nan% >500=nan% >1000=nan%
  wr=0 wr_64=0 wr_256=0 bytes=0
  rd=0 rd_64=0 wr_256=0 bytes=0
  atomic=0
    */
    uint32_t axi_wr_pend, axi_rd_pend;
    uint64_t axi_wr, axi_wr64, axi_wr256, axi_wr_bytes;
    uint64_t axi_rd, axi_rd64, axi_rd256, axi_rd_bytes;
    uint64_t atomic_req;
    uint64_t rd_lat0, rd_lat1, rd_lat2, rd_lat3;
    uint16_t cfg_rdlat[4];
    /*
    Port 0
  rx_stl=0.00% tx_stl=0.00%
  rx_req_tlp=26290 rx_cpl_tlp=0 tx_req_tlp=0 tx_cpl_tlp=25206
  rx_bad_tlp=0 rx_bad_dllp=0 rx_nak_rcvd=0
  rx_nullified=0 rxbfr_overflow=0
  tx_nak_sent=0 txbuf_ecc_err=0
  fcpe=0 fc_timeout=0 replay_num_err=0 replay_timer_err=0
  core_initiated_recovery=1095216660480 ltssm_state_changed=255
  skp_os_err=0 deskew_err=0 phystatus_err=0
  rx_malform_tlp=0 rx_framing_err=0 rx_ecrc_err=0
  rx_nullify=0 rx_watchdog_nullify=128849018880 rx_unsupp=30 rxbuf_ecc_err=0
  tx_drop=0 txbfr_overflow=0
    */
    double rx_stl, tx_stl;
    uint64_t rx_req_tlp, rx_cpl_tlp, tx_req_tlp, tx_cpl_tlp;
    uint64_t rx_bad_tlp, rx_bad_dllp, rx_nak_received;
    uint64_t rx_nullified, rxbfr_overflow;
    uint64_t tx_nak_sent, txbuf_ecc_err;

    uint64_t fcpe, fc_timeout, replay_num_err, replay_timer_err;
    uint8_t core_initiated_recovery;
    uint8_t ltssm_state_changed;
    uint64_t skp_os_err, deskew_err, phystatus_err;

    uint64_t rx_malform_tlp, rx_framing_err, rx_ecrc_err, rxbuf_ecc_err,
        rx_nullify, rx_watchdog_nullify, rx_unsupp;
    uint64_t tx_drop, txbfr_overflow;
    /*
       ==Doorbells==
       Host_DBs=40749342789 Local_DBs=26944709609 DBtoSchedReq=52013971012
    */
    uint64_t host_dbs, local_dbs, db_to_sched_reqs;
    /*
       ==TX Scheduler==
       Doorbell sets=3211038880 clears=1558291895 PB XOFF: 0% 0% 0% 0% 0% 0% 0%
       0% 0% 0% 0% 0% 0% 0% 0% 0% PHVs to Stage0:
    */
    uint32_t sets, clears, phvs_to_stage0;
    uint8_t xoff[16] = {};
    uint32_t phvs[16] = {};
    /*
Txdma: NPV phv=5719238143 pb_pbus=24217657610 pr_pbus=0 sw=0 phv_drop=0 recirc=0
Rxdma: PSP phv=1520308648 pb_pbus=0 pr_pbus=1520308672 sw=829091064 phv_drop=0
recirc=0 Ingress:  SI Parser: pkt_from_pb=87778803 phv_to_s0=2472735881
pkt_to_dp=2472735877 Egress:  SE Parser: pkt_from_pb=87815956
phv_to_s0=2472772866 pkt_to_dp=2472772848
*/

    // we will define 6 structures to hold bwmon data
#define BWMON_COUNT 6
    bwmon_t bwmons[BWMON_COUNT];

    /* crypto */
    uint64_t xts_cnt, xtsenc_cnt, gcm0_cnt, gcm1_cnt, pk_cnt;

    pipeline_t pipelines[PIPE_CNT];
    functions_t functions;
} asic_data_t;

extern asic_data_t *asic;

extern int bwmon;
extern int crypto;
extern int verbose;
extern bool export_data;
extern export_format_t export_format;
void *elbtop(void *);
void *fetch_data(void *);
void *elbmon_struct_init(void *);
void elbmon_pipeline_data_store1(uint8_t, uint64_t, uint64_t, uint64_t,
                                 uint64_t, uint64_t, uint64_t, uint64_t,
                                 uint64_t);
void elbmon_pipeline_data_store2(uint8_t, uint64_t, uint64_t, uint64_t,
                                 uint64_t, uint64_t, uint64_t, uint64_t,
                                 uint64_t, uint64_t, uint64_t);
void elbmon_pipeline_data_store3(uint8_t, uint64_t, uint64_t, uint64_t,
                                 uint64_t, uint64_t, uint64_t);
extern bool export_to_file;
extern FILE *fp;
void *elbtop_display_routine(void *);

#endif // ELBMON_H
